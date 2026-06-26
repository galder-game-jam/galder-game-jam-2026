# Galder Game Jam 2026 — CLAUDE.md

## Project overview

A 2D top-down RPG game jam project built with C++23, CMake, and raylib. The codebase is split into:

- `src/gamedev-base/` — platform-agnostic base library: interfaces, network primitives, system utilities
- `src/galder-game-jam-library/` — game-specific library: game objects, managers, graphics, networking
- `src/galder-game-jam-2026/` — the game executable
- `src/ggj-server/` — standalone server application
- `src/experiments/` — scratch/prototype code
- `tests/` — Catch2 unit tests with FakeIt mocking
- `external_libs/` — vendored dependencies (never edit)
- `content/game/content/` — runtime assets (auto-copied to bin by CMake)

## Build

```bash
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

- **Modern, target-based CMake** (minimum 3.23). Cross-platform: Linux, Windows (MSVC), macOS.
- Default build type is `Release`. `Debug` adds `-DGAME_DEV_DEBUG` and enables debug-only features.
- Static linking: `BUILD_SHARED_LIBS OFF`.
- Recommended compiler: GCC 14+. MSVC (Visual Studio 2022 v17.4+) for Windows.
- Build outputs go to `cmake-build-<type>/bin/`.

## External libraries

| Library | Purpose |
|---|---|
| raylib / raylib-cpp | Window, graphics, input |
| Box2D | 2D physics |
| fmt | String formatting |
| GameNetworkingSockets | Netplay |
| Boost.DI | Dependency injection |
| Catch2 | Unit tests |
| FakeIt | Mocking/faking in tests |
| Tileson | Tiled JSON map parsing |

Never add includes from `external_libs/` into library headers that leak outward beyond their own module.

## Dependency injection (Boost.DI)

- **Injected plain structs lose their in-class member initializers.** When Boost.DI
  constructs an *aggregate* (a struct/class with no user-declared constructor), it
  aggregate-injects each member — building every scalar as `bool{}` / `int{}` and
  **silently discarding the `{ ... }` defaults**. A `struct Foo { int n{-1}; bool on{true}; };`
  comes out of DI with `n == 0` and `on == false`.
- **Fix:** give any injected config/POD struct a user-declared constructor (even
  `Foo() = default;` is enough to make it a non-aggregate) so DI calls the constructor
  and honours the initializers. See `DebugState` in `managers/DebugMenu.hpp`.
- **Defense in depth:** validate map keys with `map.count(k) > 0` before `operator[]`,
  which default-inserts a null `unique_ptr` on a miss.

## Language & style rules

- **C++23.** Use modern features where they improve clarity.
- **Dynamic allocation only through smart pointers** (`std::unique_ptr`, `std::shared_ptr`). No raw `new`/`delete`.
- **Const-correctness everywhere.** Mark methods, parameters, and locals `const` by default.
- **Design for testability.** Components with collaborators expose an `I`-prefixed pure-virtual interface class so they can be faked in tests. Depend on interfaces, not concretions.

## Layout & naming conventions

- **Headers split across `.h` + `.cpp`** for most classes. Use `.hpp` for header-only (templates, small inline-only types).
- **File naming: PascalCase matching the class name** — `Player.h`, `AnimationManager.cpp`, `IGameObject.h`.
- **No more than three uppercase letters in a row** — write acronyms PascalCase: `IpAddress` not `IPAddress`.
- **Namespace:** everything lives in `namespace ggj`.
- **Folders:** lowercase (`game/`, `managers/`, `graphics/`, `interfaces/`, `enums/`, `system/`, `network/`, `data/`).

## Code style

Formatting is enforced by **`.clang-format`** at the repo root — run
`clang-format -i` on changed files (never on `external_libs/`).
**`CodeConvention.hpp` is the canonical worked example — match it.**
 
Key rules:
- **Allman braces** — opening brace on its own line for namespaces, classes, structs, enums, and function definitions.
- **Indentation:** 4 spaces, no tabs. Namespace bodies are indented. Access specifiers get their own indentation level.
- **Member variables:** `m_` + camelCase for private/protected (`m_lives`, `m_isVisible`). Plain camelCase for public members.
- **Boolean names:** predicate style with `is` / `has` / `can` / `use` prefix — `m_isDead`, `m_hasClearedLevel`, `isVisible()`. Avoid bare nouns.
- **Constants:** `SCREAMING_SNAKE_CASE` — including `constexpr` locals.
- **Types:** PascalCase classes/structs/enums; `enum class` with PascalCase enumerators (`PlayerState::Idle`). Interfaces are `I`-prefixed with a virtual default destructor.
- **Integer types:** use unqualified `<cstdint>` / `<cstddef>` typedefs (`uint16_t`, `int32_t`, `size_t`). Keep `std::` on standard utilities (`std::vector`, `std::unique_ptr`).
- **Annotations:** `[[nodiscard]]` on value-returning accessors; `override` / `final` where applicable; `= default` for defaulted special members.
- **Pointers/references** bind to the type: `int* p`, `const Foo& f`.
- **Prefer explicit types over `auto`.** Reserve `auto` for lambdas, iterators, and structured bindings (`auto [a, b] = ...`).

## Documentation

Document public API with **Doxygen block comments** (`/*! … */`). Put the description first, then tag parameters and return value.

```cpp
/*!
 * Moves the player by the given delta.
 *
 * @param timeDelta Time since last frame in seconds
 */
void update(float timeDelta) override;
```

- Use `/*! … */` for namespaces, types, and functions.
- `@param` for each parameter, `@return` for non-void (omit for trivial getters).
- Trailing inline member/enumerator docs use `///< …`.
- Explain *why*, not the obvious.

## Testing

- Tests live in `tests/galder-game-jam-library-tests/` and use **Catch2** (header already pulled in via `main_tests.cpp`).
- Use **FakeIt** to mock/fake interface collaborators — never mock the raylib or Box2D layer directly.
- Run tests: `cmake --build cmake-build-debug --target galder-game-jam-library-tests && ./cmake-build-debug/tests/galder-game-jam-library-tests/galder-game-jam-library-tests`

## Maps & content

- Maps are authored in **Tiled** and parsed at runtime via Tileson.
- Always open map files through `gamedev.tiled-project` at the repo root (object types are resolved via the shared project).
- Runtime content (textures, maps, audio) lives in `content/game/content/` — CMake copies it to the output bin on project reload.
