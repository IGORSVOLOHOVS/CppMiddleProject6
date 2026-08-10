# cpp-middle-project-sprint-6 <!-- omit in toc -->

- [Начало работы](#начало-работы)
- [Сборка проекта и запуск тестов](#сборка-проекта-и-запуск-тестов)
  - [Команды для сборки проекта](#команды-для-сборки-проекта)
  - [Команда для запуска тестов](#команда-для-запуска-тестов)
- [Сборка под Windows](#сборка-под-windows)
  - [Что нужно установить](#что-нужно-установить)
  - [Как собрать](#как-собрать)
  - [Пресеты CMake](#пресеты-cmake)


Шаблон репозитория для практического задания «Система диспетчеризации задач с приоритетами» 6-го спринта «Мидл разработчик С++».

## Начало работы

1. Нажмите зелёную кнопку `Use this template`, затем `Create a new repository`.
2. Назовите свой репозиторий.
3. Склонируйте созданный репозиторий командой `git clone your-repository-name`.
4. Создайте новую ветку командой `git switch -c development`.
5. Откройте проект в `Visual Studio Code`.
6. Нажмите `F1` и откройте проект в dev-контейнере командой `Dev Containers: Reopen in Container`.

## Сборка проекта и запуск тестов

Данный репозиторий использует **cmake** — генератор систем сборки для C и C++. Позволяет создавать проекты, которые могут компилироваться на различных платформах и с различными компиляторами. Подробнее о cmake:
  - https://dzen.ru/a/ZzZGUm-4o0u-IQlb
  - https://neerc.ifmo.ru/wiki/index.php?title=CMake_Tutorial
  - https://cmake.org/cmake/help/book/mastering-cmake/cmake/Help/guide/tutorial/index.html

### Команды для сборки проекта

```bash
mkdir build ; cd build

# Вызывается один раз перед сборкой проекта
cmake ..

# Вызывается каждый раз, когда необходимо собрать проект
make -j4
```

### Команда для запуска тестов

```bash
cd build
ctest --verbose
```

## Сборка под Windows

Dev-контейнер остаётся основным способом сборки, но проект собирается и нативно —
компилятором MSVC, без Docker и без WSL. Точка входа одна: `scripts\build_windows.ps1`.

### Что нужно установить

- **Visual Studio 2022** (Community достаточно) с рабочей нагрузкой
  «Разработка классических приложений на C++». Нужен компилятор MSVC v143;
  проверялось на `cl.exe` 19.44.35219 из набора инструментов 14.44. Проект
  собирается по стандарту C++23 на обеих платформах: `std::jthread`, `std::print`
  и `std::optional<std::function<void()>>` требуют именно его, а отдельного
  режима C++26 у MSVC всё равно нет — самый новый у него `/std:c++latest`,
  который CMake отдаёт под именем CXX23.
- **CMake ≥ 3.25** и **Ninja** на `PATH`. Оба приезжают вместе с компонентом
  Visual Studio «C++ CMake tools for Windows», если ставить их отдельно не хочется.
  Проверялось на CMake 4.1.1.
- **Conan 2** (`pip install conan`) — из него берётся gtest. Готовый бинарник под
  msvc 194 в ConanCenter есть, так что первый запуск занимает минуту, а не полчаса.

Отдельно запускать «Developer Command Prompt» не требуется — скрипт сам находит
`vcvars64.bat` (через `vswhere`, с запасными путями) и вносит окружение MSVC в
свой процесс.

### Как собрать

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build_windows.ps1
```

Скрипт ставит зависимости через Conan, конфигурирует CMake, собирает всё,
прогоняет `ctest` и в конце печатает пути к получившимся `.exe`. Результат лежит
в `build\windows`, основной исполняемый файл — `build\windows\dispatcher.exe`:

```powershell
build\windows\dispatcher.exe
```

Демонстрация печатает 100 строк: пять потоков кладут в диспетчер по 10 задач
обычного и по 10 задач высокого приоритета, пул разбирает их из очереди.

Полезные ключи:

| Ключ | Зачем |
| --- | --- |
| `-BuildType Debug` | сборка с отладочной информацией в `build\windows-debug` |
| `-Clean` | удалить каталог сборки и собрать с нуля |
| `-SkipTests` | не запускать `ctest` |

Две правки в коде понадобились именно ради MSVC:

- `#include <flat_map>` в `include/queue/priority_queue.hpp` подключается через
  `__has_include`: в libstdc++ 15 заголовок есть, в MSVC STL 14.44 его ещё нет,
  а сама `PriorityQueue` работает на `std::map`, так что включение было лишним.
- `/utf-8` в `CMakeLists.txt` под `if(MSVC)`: исходники в UTF-8 и с кириллицей,
  без этого флага `cl.exe` читает их в кодовой странице системы и портит
  литералы вроде `"сообщение №"`.

### Пресеты CMake

`CMakePresets.json` описывает обе платформы, поэтому IDE (Visual Studio, VS Code,
CLion) подхватывает конфигурацию сама:

| Пресет | Платформа | Генератор | Каталог сборки |
| --- | --- | --- | --- |
| `linux-default` | Linux / dev-контейнер | Unix Makefiles | `build/` |
| `windows-msvc-release` | Windows, MSVC x64 | Ninja | `build/windows/` |
| `windows-msvc-debug` | Windows, MSVC x64 | Ninja | `build/windows-debug/` |

Каждый пресет предполагает, что `conan install` уже отработал: путь к
`conan_toolchain.cmake` прописан прямо в пресете, а раскладку каталогов задаёт
`cmake_layout()` из `conanfile.py`. Windows-пресеты рассчитаны на окружение MSVC —
если запускать их руками, а не через `scripts\build_windows.ps1`, делать это надо
из «Developer PowerShell for VS 2022», иначе CMake не найдёт `cl.exe`.
