# IndexIt

**IndexIt: Optimized Local Document Search Engine for Windows**

This repository contains the IndexIt project foundation plus the Phase 2 query/CLI/benchmark integration.

## Phase 2 contribution

The current Phase 2 work provides:

- `query.c` with the `run_query()` search interface
- Exact-term lookup through the project `InvertedIndex` API
- `main.c` CLI handling for:
  - `index`
  - `search`
  - `stats`
  - `clean`
- High-resolution benchmark timing through:
  - `timer_start()`
  - `timer_stop_ms()`
- A small in-memory index implementation so the query layer can be compiled and tested while the full indexing pipeline is still under development
- CMake integration and basic tests

## Current query flow

```text
CLI
 |
 +-- search <query>
       |
       +-- timer_start()
       |
       +-- run_query(query)
              |
              +-- index_lookup()
       |
       +-- timer_stop_ms()
```

The final production flow is expected to replace the temporary integration index with the team's full:

```text
File Walker
   -> Format Adapter / Text Extraction
   -> Tokenization
   -> Inverted Index
   -> Query Engine
   -> Ranking
   -> Results
```

## CLI

```text
indexit index <folder>
indexit search <query>
indexit stats
indexit clean
```

The `stats` and `clean` commands currently remain integration placeholders because their corresponding production modules are not present in this repository branch.

## Build

```bash
cmake -S . -B build
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

On Windows with Visual Studio:

```text
build/Debug/indexit.exe
```

Example:

```bash
.\build\Debug\indexit.exe search machine
```

## Benchmarking

The timer measures elapsed search time in milliseconds:

```c
timer_start();
run_query("machine");
double elapsed_ms = timer_stop_ms();
```

For the actual project benchmark, use the same document dataset and query set for both:

```text
Sequential Search
        vs
Indexed Search (IndexIt)
```

Run repeated trials and report the measured values rather than hard-coded/example numbers.

Recommended benchmark dimensions:

- 1,000 documents
- 5,000 documents
- 10,000 documents
- repeated queries
- cold-cache and warm-cache measurements once the cache layer is integrated

## Important implementation note

The repository currently does not contain the full document walker, format extraction, persistent SQLite layer, ranking implementation, or production cache implementation. The small in-memory index in `src/index/index.c` exists only to make the Phase 2 query/CLI code testable now. It should be replaced/connected to the team's production indexing module when that module is merged.

## Project structure

```text
IndexIT/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── benchmark.h
│   ├── document.h
│   ├── hashtable.h
│   ├── inverted_index.h
│   ├── query.h
│   ├── token.h
│   └── trie.h
├── src/
│   ├── main.c
│   ├── benchmark/
│   │   ├── benchmark.c
│   │   └── benchmark.h
│   ├── index/
│   │   ├── index.c
│   │   └── index.h
│   ├── query/
│   │   ├── query.c
│   │   └── query.h
│   ├── cache.h
│   └── db.h
└── tests/
    └── test_runner.c
```
