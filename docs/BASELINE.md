# IndexIt Baseline Report (Step 0)

Date: 2026-10-29
Branch: `master` (`329afbd`)
Repo URL: `https://github.com/Normandy9/IndexIT`
Workspace: `D:\IndexIT_Final`

---

## 1. Toolchain & Environment

- **Compiler**: MinGW.org GCC 6.3.0 (`C:\MinGW\bin\gcc.exe`, `C:\MinGW\bin\g++.exe`)
  - Target: `mingw32`
  - C standard: C11 (`-std=c11`)
  - C++ standard: C++14/C++17 (`-std=c++17` flag accepted by GCC 6.3.0; lacks full `<filesystem>`, `<optional>`, and pthread-based standard threading)
  - Toolchain strategy: As permitted by Step 0 instructions, code will be restricted to features supported by the MinGW runtime (Win32 native threading/APIs, custom lightweight optional or pointer/status representations, Win32 `FindFirstFileW`/`MoveFileExW`/`GetDiskFreeSpaceExW` for filesystem operations).
- **Build System**: CMake 3.29.3 (portable copy placed at `D:\IndexIT_Final\tools\cmake\bin\cmake.exe`)
- **Generator**: `"MinGW Makefiles"` with `mingw32-make.exe`

---

## 2. CMake Configuration Output

```text
-- The C compiler identification is GNU 6.3.0
-- The CXX compiler identification is GNU 6.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/MinGW/bin/gcc.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/MinGW/bin/c++.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (5.5s)
-- Generating done (0.0s)
-- Build files have been written to: D:/IndexIT_Final/build
```

---

## 3. Build Output & Compiler Warnings

Both targets built successfully:
1. `indexit.exe` (CLI and UI)
2. `test_runner.exe` (Unit test executable)

Warnings observed during baseline build:
- `src/core/winfs.c`: implicit declaration of function `_wcsicmp`
- `src/core/tokenizer.c`: implicit declaration of function `strdup`

```text
[  3%] Building CXX object CMakeFiles/indexit.dir/src/main.cpp.obj
[  7%] Building C object CMakeFiles/indexit.dir/src/core/winfs.c.obj
[ 11%] Building C object CMakeFiles/indexit.dir/src/core/extractor.c.obj
[ 14%] Building C object CMakeFiles/indexit.dir/src/core/tokenizer.c.obj
[ 18%] Building C object CMakeFiles/indexit.dir/src/index/trie.c.obj
[ 22%] Building C object CMakeFiles/indexit.dir/src/index/hashtable.c.obj
[ 25%] Building C object CMakeFiles/indexit.dir/src/index/inverted_index.c.obj
[ 29%] Building CXX object CMakeFiles/indexit.dir/src/IExtractor.cpp.obj
[ 33%] Building CXX object CMakeFiles/indexit.dir/src/IRanker.cpp.obj
[ 37%] Building CXX object CMakeFiles/indexit.dir/src/ITermIndex.cpp.obj
[ 40%] Building CXX object CMakeFiles/indexit.dir/src/CppInvertedIndex.cpp.obj
[ 44%] Building CXX object CMakeFiles/indexit.dir/src/db.cpp.obj
[ 48%] Building CXX object CMakeFiles/indexit.dir/src/UI.cpp.obj
[ 51%] Linking CXX executable indexit.exe
[ 51%] Built target indexit
[ 55%] Building CXX object CMakeFiles/test_runner.dir/tests/test_runner.cpp.obj
...
[100%] Linking CXX executable test_runner.exe
[100%] Built target test_runner
```

---

## 4. Baseline CTest Results

```text
Test project D:/IndexIT_Final/build
    Start 1: IndexItTests
1/1 Test #1: IndexItTests .....................   Passed    0.82 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.83 sec
```

Direct run output:
```text
Tokenizer tests passed.
HashTable tests passed.
TopK tests passed.
All tests passed successfully.
```

---

## 5. Baseline Functional Verification

- `indexit.exe --help`: Functional, prints CLI help.
- `indexit.exe index <dir>`: Parses files into memory, builds index, exits without persisting.
- `indexit.exe search <dir> <term>`: Re-indexes directory from scratch on every run, performs BM25 ranking, outputs matches.
- `indexit.exe stats <dir>`: Re-indexes directory from scratch, prints total documents and average doc length.
- `indexit.exe clean`: Stub text only ("Clean command: In-memory structures freed. (SQLite persistence clearing is stubbed).").
- `indexit.exe ui <dir>`: Functional interactive terminal UI using Win32 Console API.

---

## 6. Inventory of Known Gaps (to address in Steps 1-9)

1. C++ OOP layer (`IExtractor`, `ITermIndex`, `IRanker`) bypassed by CLI & UI (they call C `extract_text` directly).
2. Duplicated file indexing callback in `main.cpp` and `UI.cpp`.
3. Trie populated but never queried.
4. Hash table fixed at 1,024 buckets (no dynamic rehashing).
5. Postings not guaranteed sorted by doc ID (cannot do linear merge).
6. Dead/unused classes: `CacheManager.hpp`, `LruCache.hpp`, `ScopedTimer`.
7. Non-existent persistence: SQLite is stubbed in `db.cpp`, no binary `index.cache`.
8. Extraction limited to plain text; DOCX/PPTX/XLSX/ODT/PDF/ZIP return placeholder strings.
9. No prefix search (`comp*`), autocomplete, snippets, fuzzy search, or metadata filtering.
10. Stale README and missing architectural/class documentation.
