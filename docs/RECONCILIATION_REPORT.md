# IndexIt — Comprehensive Code Reconciliation Report

**Date:** 2026-10-21  
**Target Repository:** `https://github.com/Normandy9/IndexIT`  
**Workspace:** `D:\IndexIT_Reconciled`  
**Source Work Package:** `D:\DJ_WORK` (Read-only)  
**Author:** Senior C/C++ Integration Engineer  

---

## Executive Summary
A complete sync, inventory, and gap analysis was conducted between the `IndexIT` team repository (default branch `master`, commit `c2274c8`) and the local work package `DJ_WORK`. 

- **Total files analyzed:** 58 distinct files
- **Category A (Only in DJ_WORK):** 16 files (Core ingestion engine, format extractor, WinFS walker, small dataset, architectural documentation)
- **Category B (Only in Repo `master`):** 36 files (Teammates' query engine, benchmark harness, dataset ingestion, C++ OOP wrappers, index structures, test runner, binaries)
- **Category C (Present in both):** 6 files (1 identical, 1 whitespace/line-ending identical, 4 differing implementations)
- **Top Interface Conflicts Identified:**
  1. `Document` struct definition: Narrow ANSI `char *path` in repo vs Wide Unicode `wchar_t *path` in `DJ_WORK`, differing format enums (`DocFormat` vs `DocumentFormat`), and field name differences (`size_bytes`/`last_modified` vs `size`/`timestamp`).
  2. Duplicate `InvertedIndex` symbol collision: `src/index/index.c` (foundation linked-list index) vs `src/index/inverted_index.c` (composite Hash+Trie production index).
  3. `Tokenizer` forward declarations missing in repo's `include/token.h`.
- **Build Status Before Integration:**
  - Repo as-is: CMake configure succeeds; build fails at link stage due to missing `src/dataset/dataset.c` in `CMakeLists.txt` (`undefined reference to 'index_dataset_folder'`). `test_runner` alone builds and passes 4/4 tests.
  - DJ_WORK as-is: CMake configure fails during generation because referenced targets (`src/main.cpp`, `test_runner.c`) do not exist within the isolated work package.

---

## A. Files Only in DJ_WORK (Missing from Repo)

These 16 files exist in `D:\DJ_WORK` but are absent from the team repository on `master`:

| File Path | LOC | Size (Bytes) | Type / Purpose | Logic Status |
|---|---|---|---|---|
| `ASSIGNED_WORK.md` | 27 | 909 | Assignment specification for Dhananjay | Documentation |
| `CHANGES.md` | 12 | 997 | Summary of architectural modifications from base | Documentation |
| `data/small_set/file1.txt` | 1 | 44 | Test fixture: plain text | Test Data |
| `data/small_set/file2.txt` | 1 | 60 | Test fixture: plain text with repeated terms | Test Data |
| `data/small_set/file3.csv` | 3 | 25 | Test fixture: CSV data | Test Data |
| `data/small_set/file4.md` | 2 | 42 | Test fixture: Markdown document | Test Data |
| `data/small_set/file5.log` | 2 | 64 | Test fixture: Log file entry | Test Data |
| `docs/architecture.md` | 26 | 968 | High-level dataflow and module architecture | Documentation |
| `docs/OWNERSHIP_SPLIT.md` | 57 | 2946 | Team component ownership matrix | Documentation |
| `include/IExtractor.hpp` | 39 | 829 | C++ abstract interface `IExtractor`, concrete classes & factory | Real Interface |
| `src/IExtractor.cpp` | 58 | 1604 | `PlainExtractor` (memory-mapped file) & format dispatch | Real / Stubs for Tier 2/3 |
| `src/core/extractor.c` | 87 | 2152 | Win32 plain text extractor (`CreateFileW`/`ReadFile`) | Real Logic |
| `src/core/extractor.h` | 21 | 319 | Public C API for document extraction | Real Header |
| `src/core/tokenizer.c` | 87 | 2609 | Stream tokenizer with stopword filter & position indexing | Real Logic |
| `src/core/winfs.c` | 107 | 4231 | Native recursive directory walker (`FindFirstFileW`) | Real Logic |
| `src/core/winfs.h` | 18 | 306 | Public C API for Windows filesystem traversal callback | Real Header |

---

## B. Files Only in Repo (Teammates' Work Not in DJ_WORK)

These 36 files exist in `D:\IndexIT_Reconciled` (`master`) but are absent from `DJ_WORK`:

| File Path | LOC | Author (Git History) | Logic Classification | Description |
|---|---|---|---|---|
| `benchmark result.png` | - | `pratyayarajamrit-cell` | Binary / Image | Terminal execution benchmark screenshot |
| `indexit.exe` | - | `pratyayarajamrit-cell` | Binary Executable | Pre-built Windows executable committed to repo |
| `Resultubbuntu2.jpeg` | - | `pratyayarajamrit-cell` | Binary / Image | Test execution output on Ubuntu |
| `Resultubuntu1.jpeg` | - | `pratyayarajamrit-cell` | Binary / Image | Test execution output on Ubuntu |
| `benchmark_dataset/small/config.conf` | 4 | `pratyayarajamrit-cell` | Real Config | Benchmark config file |
| `benchmark_dataset/small/doc1.txt` | 3 | `pratyayarajamrit-cell` | Test Data | Benchmark sample document 1 |
| `benchmark_dataset/small/doc2.txt` | 3 | `pratyayarajamrit-cell` | Test Data | Benchmark sample document 2 |
| `include/benchmark.h` | 10 | `pratyayarajamrit-cell` | Real Header | High-precision timing interface (`QueryPerformanceCounter`) |
| `include/CppInvertedIndex.hpp` | 36 | `ujjwalcodess` | Real Header | C++ wrapper integrating term index with `IRanker` |
| `include/dataset.h` | 8 | `pratyayarajamrit-cell` | Real Header | Declaration of `index_dataset_folder` |
| `include/hashtable.h` | 14 | `ujjwalcodess` / `DJ` | Real Header | FNV-1a Hash Table declarations |
| `include/inverted_index.h` | 14 | `ujjwalcodess` / `DJ` | Real Header | Declarations for `index_create`, `index_lookup`, etc. |
| `include/ITermIndex.hpp` | 39 | `ujjwalcodess` | Real Header | C++ abstract interface for `TrieIndex` and `HashIndex` |
| `include/query.h` | 11 | `pratyayarajamrit-cell` | Real Header | Query dispatch interface (`run_query`) |
| `include/trie.h` | 14 | `ujjwalcodess` / `DJ` | Real Header | 36-ary Trie declarations |
| `src/cache.h` | 35 | `Paras-tech26` | Stub Header | Planned LRU cache sketch; no implementation file |
| `src/cache/.gitkeep` | 0 | `Paras-tech26` | Empty Placeholder | Directory placeholder |
| `src/CppInvertedIndex.cpp` | 48 | `ujjwalcodess` | Real Logic | Implementation of C++ index wrapper |
| `src/db.h` | 28 | `Paras-tech26` | Stub Header | Planned SQLite schema sketch; no implementation file |
| `src/db/.gitkeep` | 0 | `Paras-tech26` | Empty Placeholder | Directory placeholder |
| `src/ITermIndex.cpp` | 55 | `ujjwalcodess` | Real Logic | C++ adapters delegating to C Trie and Hash Table |
| `src/main.c` | 347 | `pratyayarajamrit-cell` | Mixed (Real / Stubs) | CLI commands (`index`, `search`, `benchmark` real; `stats`, `clean` stubs) |
| `src/trie.c` | 39 | `ujjwalcodess` | Stub Only | Placeholder `struct Trie { int placeholder; }` returning 0 |
| `src/benchmark/benchmark.c` | 68 | `pratyayarajamrit-cell` | Real Logic | Timer implementation using `QueryPerformanceCounter` on Win32 |
| `src/core/.gitkeep` | 0 | `Paras-tech26` | Empty Placeholder | Directory placeholder |
| `src/dataset/dataset.c` | 119 | `pratyayarajamrit-cell` | Real Logic | Flat folder traversal (`FindFirstFileA`) & word indexer |
| `src/dataset/datasetubuntu.c` | 105 | `pratyayarajamrit-cell` | Real Logic | POSIX `dirent.h` traversal for Linux |
| `src/index/.gitkeep` | 0 | `Paras-tech26` | Empty Placeholder | Directory placeholder |
| `src/index/hashtable.c` | 146 | `ujjwalcodess` | Real Logic | FNV-1a Hash Table with chaining |
| `src/index/index.c` | 118 | `pratyayarajamrit-cell` | Real Logic / Stub | Minimal linked-list index implementation for testing |
| `src/index/index.h` | 12 | `pratyayarajamrit-cell` | Real Header | Internal prototypes for `index.c` |
| `src/index/inverted_index.c` | 52 | `ujjwalcodess` | Real Logic | Production index joining Hash Table and Trie |
| `src/index/trie.c` | 143 | `ujjwalcodess` | Real Logic | 36-ary Trie implementation with posting lists |
| `src/query/.gitkeep` | 0 | `Paras-tech26` | Empty Placeholder | Directory placeholder |
| `src/query/query.c` | 63 | `pratyayarajamrit-cell` | Real Logic | Query execution printing matching posting |
| `tests/test_runner.c` | 83 | `pratyayarajamrit-cell` | Real Test Runner | Unit tests for index lookup, query execution, and timer |

---

## C. Files Present in Both

| File Path | Status | Diff Summary |
|---|---|---|
| `.gitignore` | **Identical** | SHA256 hashes match identically (380 bytes). |
| `include/TopK.hpp` | **Identical** | Code logic is identical; difference is CRLF vs LF line endings and minor whitespace. |
| `include/document.h` | **Both Real, Differ** | **Repo:** Defines `DocFormat` (4 values) and `Document` with narrow `char *path`, `uint64_t size_bytes`, and `time_t last_modified`.<br>**DJ_WORK:** Defines `DocumentFormat` (17 values) and `Document` with wide `wchar_t *path`, `uint64_t size`, and `time_t timestamp`. |
| `include/token.h` | **Both Real, Differ** | Both share identical definitions for `Token` and `Posting`. DJ_WORK adds `extern "C"` and C prototypes for `Tokenizer` (`tokenizer_create`, `tokenizer_next`, `tokenizer_destroy`). |
| `CMakeLists.txt` | **Both Real, Differ** | **Repo:** C11-only project compiling `main.c`, `query.c`, `benchmark.c`, `index.c` into `indexit` executable.<br>**DJ_WORK:** C11/C++17 project configuring sources across `src/core/`, `src/index/`, C++ OOP layer, and test runner. |
| `README.md` | **Both Real, Differ** | **Repo:** 149 lines of detailed team documentation, benchmark numbers, CLI instructions, and architecture diagrams.<br>**DJ_WORK:** 24 lines starter placeholder. |

---

## D. Detailed Interface Conflicts

### 1. Document Data Model (`include/document.h`)
- **Repo (`D:\IndexIT_Reconciled\include\document.h#L8-L23`):**
  ```c
  typedef enum {
      FORMAT_UNKNOWN, FORMAT_TXT, FORMAT_PDF, FORMAT_DOCX
  } DocFormat;

  typedef struct {
      uint32_t id;
      char *path;
      DocFormat format;
      uint64_t size_bytes;
      time_t last_modified;
  } Document;
  ```
- **DJ_WORK (`D:\DJ_WORK\include\document.h#L8-L34`):**
  ```c
  typedef enum {
      FORMAT_UNKNOWN = 0, FORMAT_TXT, FORMAT_CSV, FORMAT_LOG,
      FORMAT_MD, FORMAT_JSON, FORMAT_XML, FORMAT_HTML,
      FORMAT_C, FORMAT_CPP, FORMAT_H, FORMAT_DOCX,
      FORMAT_PPTX, FORMAT_XLSX, FORMAT_ODT, FORMAT_PDF, FORMAT_ZIP
  } DocumentFormat;

  typedef struct {
      uint64_t id;
      wchar_t *path;
      uint64_t size;
      time_t timestamp;
      DocumentFormat format;
  } Document;
  ```
- **Callers Affected:**
  - `src/core/winfs.c` and `src/core/winfs.h` populate `wchar_t *path`, `size`, `timestamp`, and `DocumentFormat`.
  - `src/core/extractor.c` inspects `doc->format` against the 17 format enum values and passes `wchar_t *path` to Win32 wide-character file APIs (`CreateFileW`).
  - `include/IExtractor.hpp` and `src/IExtractor.cpp` rely on `DocumentFormat`.
  - If the repo header replaces DJ_WORK's header, all 6 ingestion files fail compilation immediately with type mismatch and undeclared identifier errors.

### 2. Inverted Index Function Symbol Collision
- **Files:** `src/index/index.c` (Pratyay) vs `src/index/inverted_index.c` (Ujjwal / DJ)
- Both files define identical global C symbols:
  - `InvertedIndex* index_create(void)`
  - `void index_destroy(InvertedIndex *index)`
  - `int index_insert(InvertedIndex *index, const char *term, Posting posting)`
  - `Posting* index_lookup(const InvertedIndex *index, const char *term)`
- **Conflict:** Linking both object files into a single binary causes `duplicate symbol` linker errors.
- **Root Cause:** Pratyay committed a temporary linked-list stub in `index.c` to allow CLI and query tests to proceed while Ujjwal worked on the true Trie and Hash Table implementation. When Ujjwal's PR #5 merged `inverted_index.c`, `hashtable.c`, and `trie.c`, `index.c` was left in the tree.

### 3. Missing `Tokenizer` Declarations in Repo's `include/token.h`
- Repo's `include/token.h` only defines `Token` and `Posting`.
- `src/core/tokenizer.c` implements `tokenizer_create`, `tokenizer_next`, and `tokenizer_destroy`.
- Without header declarations, external callers cannot consume the tokenizer without generating implicit declaration compiler warnings/errors.

---

## E. Build Comparison (Before Integration)

### 1. Repository Build As-Is (`master` at commit `c2274c8`)
- **Configure Command:**
  ```powershell
  cmake -G "MinGW Makefiles" D:\IndexIT_Reconciled
  ```
  *Result:* Configured cleanly in 3.3 seconds.
- **Build Command:**
  ```powershell
  cmake --build .
  ```
  *Result:* **FAILED** with linker error:
  ```
  [ 10%] Building C object CMakeFiles/indexit.dir/src/main.c.obj
  [ 20%] Building C object CMakeFiles/indexit.dir/src/query/query.c.obj
  [ 30%] Building C object CMakeFiles/indexit.dir/src/benchmark/benchmark.c.obj
  [ 40%] Building C object CMakeFiles/indexit.dir/src/index/index.c.obj
  [ 50%] Linking C executable indexit.exe
  CMakeFiles\indexit.dir/objects.a(main.c.obj):main.c:(.text+0x173): undefined reference to `index_dataset_folder'
  CMakeFiles\indexit.dir/objects.a(main.c.obj):main.c:(.text+0x2b6): undefined reference to `index_dataset_folder'
  CMakeFiles\indexit.dir/objects.a(main.c.obj):main.c:(.text+0x4be): undefined reference to `index_dataset_folder'
  collect2.exe: error: ld returned 1 exit status
  CMakeFiles\indexit.dir\build.make:149: recipe for target 'indexit.exe' failed
  ```
- **Analysis:** Teammate `pratyayarajamrit-cell` added calls to `index_dataset_folder` in `src/main.c`, and implemented it in `src/dataset/dataset.c`, but forgot to include `src/dataset/dataset.c` in `add_executable(indexit)` in `CMakeLists.txt`.
- **Target `test_runner`:**
  Building `test_runner` alone succeeded:
  ```powershell
  cmake --build . --target test_runner
  ctest --output-on-failure
  ```
  *Result:* **100% tests passed (1 of 1 passed in 0.63s)**.

### 2. DJ_WORK Build As-Is
- **Configure Command:**
  ```powershell
  cmake -G "MinGW Makefiles" D:\DJ_WORK
  ```
  *Result:* **FAILED** during CMake generation:
  ```
  CMake Error at CMakeLists.txt:33 (add_executable):
    Cannot find source file: src/main.cpp
  CMake Error at CMakeLists.txt:44 (add_executable):
    Cannot find source file: tests/test_runner.c
  CMake Error at CMakeLists.txt:33 (add_executable):
    No SOURCES given to target: indexit
  CMake Error at CMakeLists.txt:44 (add_executable):
    No SOURCES given to target: test_runner
  ```
- **Analysis:** `DJ_WORK` is an isolated developer work package containing assigned modules (`src/core/*`, `IExtractor`, small data set, docs). It does not contain `src/main.cpp` or `tests/test_runner.c`. Therefore, it cannot build standalone without the rest of the project.

---

## F. Recommendations Per File

| File | Recommendation | Justification |
|---|---|---|
| `ASSIGNED_WORK.md` | **Take DJ_WORK** | Missing from repo; clarifies scope and ownership. |
| `CHANGES.md` | **Take DJ_WORK** | Missing from repo; documents historical delta. |
| `data/small_set/*` | **Take DJ_WORK** | Missing test fixtures required for local document engine verification. |
| `docs/architecture.md` | **Take DJ_WORK** | Missing architectural document. |
| `docs/OWNERSHIP_SPLIT.md`| **Take DJ_WORK** | Missing ownership matrix document. |
| `include/IExtractor.hpp` | **Take DJ_WORK** | Missing C++ extractor interface. |
| `src/IExtractor.cpp` | **Take DJ_WORK** | Missing C++ extractor implementation (uses `RAII.hpp`). |
| `src/core/extractor.c` | **Take DJ_WORK** | Missing Win32 text extraction module. |
| `src/core/extractor.h` | **Take DJ_WORK** | Missing extraction header. |
| `src/core/tokenizer.c` | **Take DJ_WORK** | Missing stream tokenizer. |
| `src/core/winfs.c` | **Take DJ_WORK** | Missing Windows native recursive directory walker. |
| `src/core/winfs.h` | **Take DJ_WORK** | Missing directory walker header. |
| `include/document.h` | **Keep Repo & Add Adapter** | Keep teammate's `document.h` as-is. Add `include/document_dj.h` for wide-character Win32 traversal to strictly follow Rule 3 without breaking either side. |
| `include/token.h` | **Keep Repo & Add Tokenizer API**| Keep teammate's `Token` and `Posting` struct. Provide `include/tokenizer.h` with `Tokenizer` declarations so callers can stream tokens. |
| `include/TopK.hpp` | **Keep Repo** | Functionally identical; keep repo version. |
| `README.md` | **Keep Repo** | Repo version contains comprehensive team documentation and benchmark results. |
| `CMakeLists.txt` | **Merge Manually** | Add `src/dataset/dataset.c` to fix teammate link error. Add CXX support and compilation targets for DJ's core modules and C++ OOP interfaces. |
| `src/trie.c` | **Keep Repo (Exclude from build)** | Empty stub; `src/index/trie.c` contains the real implementation. |
| `src/index/index.c` | **Keep Repo (Main CLI target)** | Keep in `indexit` to preserve teammate's working CLI and test runner. |
| `src/index/inverted_index.c`| **Keep Repo (Core library target)**| Build alongside `hashtable.c` and `trie.c` in `indexit_core` and engine tests. |
