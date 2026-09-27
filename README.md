# IndexIt

**IndexIt: Optimized Local Document Search Engine for Windows**

This repository contains the initial project foundation for IndexIt.

## Current scope

- Cache interface (`CacheEntry`, `cache_get()`, `cache_put()`)
- Initial database schema plan for `documents` and `terms`
- Lightweight C tests using the standard `assert()` library
- CMake build configuration
- Minimal `indexit.exe` executable

The cache and database are currently **interfaces/plans only**. Their full implementations will be added in later development stages.

## Project Structure

```text
IndexIt/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── main.c
│   ├── cache.h
│   └── db.h
└── tests/
    └── test_runner.c
```

## Requirements

- C compiler such as GCC/MinGW or MSVC
- CMake 3.15 or newer

## Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

On Windows with a Visual Studio/ multi-config generator, the executable is normally located under:

```text
build/Debug/indexit.exe
```

Run it with:

```bash
.\build\Debug\indexit.exe
```

With a single-config generator such as MinGW Makefiles, it may be:

```text
build/indexit.exe
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

The initial test runner demonstrates the lightweight `assert()`-based approach.

## Development note

This is the foundation stage. No real cache storage, SQLite database, document indexing, or search functionality is implemented yet.
