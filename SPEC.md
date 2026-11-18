# Спецификация требований: диспетчер задач с приоритетами

Восстановлена по коду ветки `dev`, коммит `fddc7b5`, 2026-11-18. Описывает только реализованное поведение; источники указаны как `путь:строка`. Обзор проекта: [docs/README.md](docs/README.md).

## 1. Назначение и границы

Библиотека C++23 для выполнения задач `std::function<void()>` пулом потоков с учётом приоритета (`High`, `Normal`) и демонстрационная программа `dispatcher`. Учебный проект из шаблона задания 6-го спринта курса «Мидл разработчик С++» (`README.md:9`).

В границах:

- потокобезопасные очереди `BoundedQueue` и `UnboundedQueue` за интерфейсом `IQueue`;
- очередь приоритетов `PriorityQueue` с блокирующим `pop` и остановкой `shutdown`;
- пул потоков `ThreadPool` и фасад `TaskDispatcher`;
- журнал `Logger` в `stdout`;
- программа `main.cpp` и тесты GoogleTest.

Вне границ (в коде отсутствует): возврат результата задачи, ожидание завершения отдельной задачи, отмена, отложенные и периодические задачи, сохранение очереди на диск, сеть, приоритеты кроме двух значений `TaskPriority`.

## 2. Контекст

- Действующие лица: код-клиент, вызывающий `TaskDispatcher::schedule` из одного или нескольких потоков; рабочие потоки пула. Люди-пользователи в файлах не названы.
- Внешние системы: нет. Ввод-вывод — только запись строк в `stdout` (`include/logger.hpp:23`).
- Среда выполнения: процесс ОС с поддержкой `std::jthread`. Подготовленная среда разработки — dev-контейнер Ubuntu с GCC 15 (`.devcontainer/devcontainer.json:2`, `.devcontainer/Dockerfile:1-9`). Другие платформы в файлах не упомянуты.

## 3. Функциональные требования

### 3.1. Очереди

- **FR-1.** Система должна добавлять задачу в `UnboundedQueue` без ограничения числа элементов. Source: `src/queue/unbounded_queue.cpp:5-9`. Verification: `UnboundedQueueTest.HandlesManyItems`.
- **FR-2.** Система должна в `try_pop` обеих очередей без ожидания возвращать первую по порядку поступления задачу или `std::nullopt`, если очередь пуста. Source: `src/queue/unbounded_queue.cpp:11-19`, `src/queue/bounded_queue.cpp:14-23`. Verification: `UnboundedQueueTest.PushAndPop`, `BoundedQueueTest.PushAndPop` (наличие и пустота; порядок FIFO тестом не проверяется).
- **FR-3.** Система должна блокировать `BoundedQueue::push`, пока число элементов не меньше ёмкости. Source: `src/queue/bounded_queue.cpp:9`. Verification: `BoundedQueueTest.RespectsCapacity` создаёт ожидание, но факт блокировки не утверждает: без неё тест тоже прошёл бы.
- **FR-4.** Система должна после извлечения из `BoundedQueue` будить один ожидающий `push`. Source: `src/queue/bounded_queue.cpp:19-21`. Verification: `BoundedQueueTest.RespectsCapacity`.
- **FR-5.** Система должна допускать одновременные `push` и `try_pop` из разных потоков, защищая очередь мьютексом. Source: `src/queue/bounded_queue.cpp:8,15`, `src/queue/unbounded_queue.cpp:6,12`. Verification: `BoundedQueueTest.MultiThreaded`, `UnboundedQueueTest.MultiThreaded`.

### 3.2. Очередь приоритетов

- **FR-6.** Система должна создавать по одной подочереди на каждый приоритет из настроек: `BoundedQueue` с ёмкостью `capacity` (1024, если не задана) при `bounded == true`, иначе `UnboundedQueue`. Source: `src/queue/priority_queue.cpp:9-18`. Verification: `PriorityQueueTest.*` (только неограниченные подочереди); ограниченная — косвенно `TaskDispatcherTest.*`; значение 1024 — no test.
- **FR-7.** Система должна помещать задачу в подочередь её приоритета, а при отсутствии такой подочереди отбрасывать задачу без сообщения об ошибке. Source: `src/queue/priority_queue.cpp:20-28`. Verification: `PriorityQueueTest.HighPriorityFirst` (маршрутизация); отбрасывание — no test.
- **FR-8.** Система должна в `pop` возвращать задачу из первой непустой подочереди в порядке возрастания значения `TaskPriority` (`High` раньше `Normal`). Source: `src/queue/priority_queue.cpp:39-43`, `include/types.hpp:7`. Verification: `PriorityQueueTest.HighPriorityFirst`.
- **FR-9.** Система должна блокировать `pop`, пока все подочереди пусты и остановка не запрошена. Source: `src/queue/priority_queue.cpp:45-49`. Verification: `PriorityQueueTest.PopBlocksOnEmpty`.
- **FR-10.** Система должна по `shutdown` будить все ожидающие `pop`, после чего `pop` выдаёт оставшиеся задачи, а затем возвращает `std::nullopt`. Source: `src/queue/priority_queue.cpp:30-33,38-47`. Verification: `PriorityQueueTest.Shutdown`, `PriorityQueueTest.ShutdownDrainsTasks`.

### 3.3. Пул потоков

- **FR-11.** Система должна при создании `ThreadPool` запускать `thread_count` рабочих потоков `std::jthread`. Source: `src/thread_pool/thread_pool.cpp:11-15`. Verification: косвенно `TaskDispatcherTest.ExecutesTasks`.
- **FR-12.** Система должна в каждом рабочем потоке извлекать и выполнять задачи, пока `pop` не вернёт `std::nullopt`. Source: `src/thread_pool/thread_pool.cpp:19-22`. Verification: `TaskDispatcherTest.ExecutesTasks`.
- **FR-13.** Система должна перехватывать исключение задачи, писать `Task threw an exception: <what()>` или `Task threw an unknown exception.` через `Logger` и продолжать работу потока. Source: `src/thread_pool/thread_pool.cpp:21-27`. Verification: no test.
- **FR-14.** Система должна в деструкторе `ThreadPool` вызывать `shutdown` очереди; потоки присоединяются деструктором `std::jthread` после выполнения оставшихся задач. Source: `src/thread_pool/thread_pool.cpp:17`, `include/thread_pool/thread_pool.hpp:10`. Verification: `TaskDispatcherTest.ExecutesTasks` (счётчик проверяется после выхода из области видимости).

### 3.4. Диспетчер

- **FR-15.** Система должна в `TaskDispatcher` создавать общую `PriorityQueue` и `ThreadPool`, по умолчанию с ограниченной подочередью `High` ёмкостью 1000 и неограниченной `Normal`. Source: `include/task_dispatcher.hpp:16-19`, `src/task_dispatcher.cpp:6-9`. Verification: `TaskDispatcherTest.ExecutesTasks`.
- **FR-16.** Система должна в `schedule(priority, task)` передавать задачу в `PriorityQueue::push`. Source: `src/task_dispatcher.cpp:13-15`. Verification: `TaskDispatcherTest.ExecutesTasks`, `TaskDispatcherTest.HighPriorityTasksFirst` (результат последнего зависит от планирования потоков, см. раздел 9).
- **FR-17.** Система должна уничтожать пул раньше очереди: `pool_` объявлен после `queue_`. Source: `include/task_dispatcher.hpp:12-13`. Verification: no test.

### 3.5. Журнал и демонстрация

- **FR-18.** Система должна предоставлять единственный экземпляр `Logger`, который пишет каждое сообщение отдельной строкой в `stdout` с отключённой буферизацией. Source: `include/logger.hpp:10-15,22-26`. Verification: no test.
- **FR-19.** Система должна в `main` создавать `TaskDispatcher` с `std::thread::hardware_concurrency()` потоками и из 5 потоков ставить по 10 пар задач `Normal`/`High`, печатающих `Normal priority message №N` / `High priority message №N`, `N = 10*i + j`. Source: `main.cpp:13-25`. Verification: no test.

### 3.6. Объявлено, но не реализовано

- `not_empty_cv_` в обеих очередях получает `notify_one`, но ни один код его не ждёт: `include/queue/bounded_queue.hpp:13`, `include/queue/unbounded_queue.hpp:12`, `src/queue/bounded_queue.cpp:11`, `src/queue/unbounded_queue.cpp:8`.
- Каталог git-хуков `.githooks`, на который `CMakeLists.txt:10-14` переключает `core.hooksPath`, в репозитории отсутствует.
- Задача VS Code `GCC: Build Debug app` (`.vscode/launch.json:14`) не определена: `tasks.json` нет.
- Закомментированная связь с целью `metric` (`src/thread_pool/CMakeLists.txt:5-8`); такой цели в проекте нет.

## 4. Интерфейсы

Программный интерфейс (пространство имён `dispatcher`):

| Сущность | Сигнатура | Источник |
|---|---|---|
| `TaskDispatcher` | `explicit TaskDispatcher(size_t thread_count, const std::flat_map<TaskPriority, queue::QueueOptions>& tasks_options = {...})` | `include/task_dispatcher.hpp:16-19` |
| | `void schedule(TaskPriority priority, std::function<void()> task)` | `include/task_dispatcher.hpp:21` |
| `queue::PriorityQueue` | `explicit PriorityQueue(const std::flat_map<TaskPriority, QueueOptions>&)`, `push(TaskPriority, Task)`, `std::optional<Task> pop()`, `shutdown()` | `include/queue/priority_queue.hpp:21-24` |
| `queue::IQueue` | `push(Task)`, `std::optional<Task> try_pop()` | `include/queue/queue.hpp:14-19` |
| `queue::BoundedQueue` | `explicit BoundedQueue(unsigned capacity)` | `include/queue/bounded_queue.hpp:17` |
| `queue::UnboundedQueue` | конструктор по умолчанию | `include/queue/unbounded_queue.hpp:15` |
| `thread_pool::ThreadPool` | `ThreadPool(std::shared_ptr<queue::PriorityQueue>, size_t thread_count)` | `include/thread_pool/thread_pool.hpp:14` |
| `Logger` (глобальное) | `static Logger& Get()`, `void Log(const std::string&)` | `include/logger.hpp:10-15` |

Цели сборки:

| Цель | Тип | Источник |
|---|---|---|
| `dispatcher` | исполняемый файл из `main.cpp`, аргументов нет | `CMakeLists.txt:22-32`, `main.cpp:12` |
| `task_dispatcher`, `queue`, `thread_pool` | библиотеки | `src/CMakeLists.txt:4`, `src/queue/CMakeLists.txt:1`, `src/thread_pool/CMakeLists.txt:1` |
| `dispatcher_test`, `queue_test` | тесты, зарегистрированы в CTest | `tests/CMakeLists.txt:1-14`, `tests/queue/CMakeLists.txt:1-16` |

Вывод программы: строки текста в `stdout`, по одной на сообщение (`include/logger.hpp:15`).

## 5. Данные

- `TaskPriority` — `enum class { High, Normal }` (`include/types.hpp:7`).
- `Task` — `std::function<void()>` (`include/queue/queue.hpp:7`).
- `QueueOptions` — `{ bool bounded; std::optional<int> capacity; }` (`include/queue/queue.hpp:9-12`).
- Хранение — только в памяти процесса: `std::queue<Task>` в подочередях, `std::flat_map` подочередей по приоритету (`include/queue/bounded_queue.hpp:10`, `include/queue/priority_queue.hpp:15`). Постоянного хранилища нет.

## 6. Нефункциональные требования

- **NFR-1.** Код компилируется по стандарту C++23 без расширений компилятора. Source: `CMakeLists.txt:4-6`, `conanfile.py:19`.
- **NFR-2.** Сборка требует CMake ≥ 3.25 и экспортирует `compile_commands.json`. Source: `CMakeLists.txt:1,7`, `conanfile.py:24`.
- **NFR-3.** Флаг остановки хранится в `std::atomic<bool>` с порядком памяти release/acquire. Source: `src/queue/priority_queue.cpp:31,45`.
- **NFR-4.** Вывод журнала не буферизуется. Source: `include/logger.hpp:25`.
- **NFR-5.** Форматирование — `.clang-format` (основа LLVM, ширина 120, отступ 4, без табуляции); VS Code форматирует при сохранении. Source: `.clang-format:2-12`, `.vscode/settings.json:3-4`.
- **NFR-6.** Среда разработки: образ Ubuntu, закреплённый по дайджесту, GCC 15 по умолчанию. Source: `.devcontainer/Dockerfile:1,5,8-9`.

Целевых показателей производительности, задержки или пропускной способности в файлах нет.

## 7. Ограничения и зависимости

- Стандартная библиотека с `<flat_map>` и `<print>` (`include/queue/priority_queue.hpp:7`, `main.cpp:4`).
- GoogleTest: обязателен на этапе конфигурации (`CMakeLists.txt:16`); источник — `gtest/1.16.0` через Conan (`conanfile.py:13`) или пакет `libgtest-dev` без указания версии (`.devcontainer/Dockerfile:5`). Файла блокировки Conan нет.
- Профиль Conan: gcc 15, `compiler.cppstd=23`, `libstdc++11`, Release, Linux x86_64 (`.devcontainer/default`).
- Лицензия MIT (`LICENSE`).

## 8. Приёмка

1. Собрать по `README.md:29-37` (`mkdir build ; cd build`, `cmake ..`, `make -j4`) в dev-контейнере.
2. Выполнить `ctest --verbose` в `build` (`README.md:41-44`): оба теста CTest, `dispatcher_test` и `queue_test`, должны пройти; внутри них 12 тестов GoogleTest, перечисленных в разделе 3.
3. Запустить `build/dispatcher`: ожидается, по чтению `main.cpp:16-25`, 100 строк — по одной `Normal priority message №N` и `High priority message №N` для каждого `N` от 0 до 49, в произвольном порядке. Это ожидание не проверялось запуском.
4. Требования с пометкой «no test» (FR-6 в части 1024, FR-7 в части отбрасывания, FR-13, FR-17–FR-19) проверяются чтением кода по указанным строкам.

## 9. Открытые вопросы

- Текст задания курса в репозитории отсутствует; какие требования задания код должен выполнять, неизвестно.
- Какая ветка основная: `origin/HEAD` указывает на `release`, где код и README отличаются от `dev`; описывать ли её отдельно.
- Какой источник GoogleTest предполагается — Conan или системный пакет; команды `conan install` в файлах нет.
- Были ли тесты запущены и проходили ли они: CI в ветке нет, лог прогона не сохранён. По чтению кода `TaskDispatcherTest.HighPriorityTasksFirst` зависит от планирования (задачи `Normal` ставятся раньше `High` при работающих потоках, `tests/task_dispatcher.cpp:56-70`) — вывод, на `dev` не проверен; в ветке `main` тест той же схемы, по сообщению коммита `a28620b` (2026-08-10), упал в CI на первом элементе.
- Вывод из кода, не воспроизведён: взаимная блокировка при заполненной ограниченной подочереди (`src/queue/priority_queue.cpp:22-24` держит мьютекс во время ожидания в `src/queue/bounded_queue.cpp:9`) и потеря пробуждения при `shutdown` без захвата мьютекса (`src/queue/priority_queue.cpp:30-33,45-49`). Частичный ответ из других веток: потеря пробуждения в `origin/release` исправлена коммитом `a7a4618` (2026-11-07, `shutdown` под `mutex_`); `PriorityQueue::push` там устроен так же, как в `dev`, признаков исправления взаимной блокировки нет.
- Почему ёмкости по умолчанию 1000 (`include/task_dispatcher.hpp:18`) и 1024 (`src/queue/priority_queue.cpp:13`), и должна ли неизвестная или отрицательная ёмкость, а также приоритет без подочереди, давать ошибку.
- Поддерживаемые платформы и компиляторы кроме GCC 15 в контейнере: в `dev` других нет; в `main` и `release` добавлена сборка под Windows (MSVC) (`7e664c5`, `scripts/build_windows.ps1`). Относится ли это к `dev`, неизвестно.
- Что должно было лежать в `.githooks`.
