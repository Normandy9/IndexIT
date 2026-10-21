# IndexIt Reconciled — Integration Notes

**Date:** 2026-10-21  
**Target Repository:** `IndexIT` (`D:\IndexIT_Reconciled`)  
**Base Branch:** `master` (HEAD: `c2274c8`)  
**Integrated Work Package:** `D:\DJ_WORK`  

---

## 1. Overview of Integration Strategy
In accordance with project constraints:
1. All changes were applied strictly to `WORKDIR` (`D:\IndexIT_Reconciled`).
2. No git commits, branches, or tags were created; all changes remain uncommitted for review.
3. Teammates' working code was never overwritten or deleted.
4. All interface divergences were reconciled via non-breaking alternative headers and adapters.
5. All integration issues and modifications have been recorded below.

---

## 2. Category A: Additions from `DJ_WORK` (Missing from Repo)

The following files existed only in `DJ_WORK` and were integrated into `WORKDIR`:
- **Ingestion & Windows Filesystem Traversal:**
  - `src/core/winfs.h`: Public C interface for recursive Win32 directory traversal.
  - `src/core/winfs.c`: Native Win32 walker using `FindFirstFileW` / `FindNextFileW`, handling Unicode paths, access denied errors, and file attributes.
- **Document Text Extractor:**
  - `src/core/extractor.h`: C interface for format-aware text extraction.
  - `src/core/extractor.c`: Win32 file extraction implementation (`CreateFileW`, `GetFileSizeEx`, `ReadFile`) for plain text, CSV, Markdown, log files, and code sources.
- **Tokenization:**
  - `src/core/tokenizer.c`: Stream tokenizer featuring lowercase conversion, punctuation stripping, position indexing, and a 32-term stopword filter.
- **C++ OOP Extractor Layer:**
  - `include/IExtractor.hpp`: Abstract `IExtractor` class, concrete `PlainExtractor`, `DocxExtractor`, `PdfExtractor`, `ZipExtractor`, and `ExtractorFactory`.
  - `src/IExtractor.cpp`: Memory-mapped file implementation using RAII wrappers.
- **Documentation & Test Fixtures:**
  - `ASSIGNED_WORK.md`, `CHANGES.md`, `docs/architecture.md`, `docs/OWNERSHIP_SPLIT.md`.
  - `data/small_set/file1.txt`, `file2.txt`, `file3.csv`, `file4.md`, `file5.log`.

---

## 3. Conflicting Files: Both Versions Real & Differing

### Conflict 1: `include/document.h` vs `include/document_dj.h`

#### Teammates' Repo Version (`include/document.h`):
```c
#ifndef INDEXIT_DOCUMENT_H
#define INDEXIT_DOCUMENT_H

#include <stdint.h>
#include <time.h>

typedef enum {
    FORMAT_UNKNOWN,
    FORMAT_TXT,
    FORMAT_PDF,
    FORMAT_DOCX
} DocFormat;

typedef struct {
    uint32_t id;
    char *path;
    DocFormat format;
    uint64_t size_bytes;
    time_t last_modified;
} Document;

#endif
```

#### DJ_WORK Version (`include/document_dj.h`):
```c
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <stdint.h>
#include <time.h>
#include <wchar.h>

typedef enum {
    FORMAT_UNKNOWN = 0,
    FORMAT_TXT,
    FORMAT_CSV,
    FORMAT_LOG,
    FORMAT_MD,
    FORMAT_JSON,
    FORMAT_XML,
    FORMAT_HTML,
    FORMAT_C,
    FORMAT_CPP,
    FORMAT_H,
    FORMAT_DOCX,
    FORMAT_PPTX,
    FORMAT_XLSX,
    FORMAT_ODT,
    FORMAT_PDF,
    FORMAT_ZIP
} DocumentFormat;

typedef struct {
    uint64_t id;
    wchar_t *path;
    uint64_t size;
    time_t timestamp;
    DocumentFormat format;
} Document;

#endif
```

#### Reconciliation Resolution:
- Preserved the teammate's `include/document.h` without alteration.
- Created `include/document_dj.h` to supply DJ's wide-character Win32 types to `winfs.h`, `extractor.h`, and `IExtractor.hpp`.
- Teammates' code continues to see `char *path` and `DocFormat`, while Windows Win32 ingestion functions see `wchar_t *path` and `DocumentFormat`.

---

### Conflict 2: `include/token.h` vs `include/tokenizer.h`

#### Teammates' Repo Version (`include/token.h`):
```c
#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

typedef struct {
    char *text;
    uint32_t position;
} Token;

typedef struct {
    uint64_t document_id;
    uint32_t frequency;
} Posting;

#endif
```

#### DJ_WORK Additions:
```c
typedef struct Tokenizer Tokenizer;

Tokenizer* tokenizer_create(const char *text);
int tokenizer_next(Tokenizer *tokenizer, Token *out_token);
void tokenizer_destroy(Tokenizer *tokenizer);
```

#### Reconciliation Resolution:
- Kept `include/token.h` containing base structs `Token` and `Posting`.
- Introduced `include/tokenizer.h` with the `Tokenizer` struct declaration and streaming prototypes.
- Included `tokenizer.h` in `src/core/tokenizer.c` and consumer modules.

---

### Conflict 3: Dual Inverted Index (`src/index/index.c` vs `src/index/inverted_index.c`)

#### Description:
`src/index/index.c` (written by Pratyay) and `src/index/inverted_index.c` (written by Ujjwal / DJ) both export the exact same C symbols:
- `InvertedIndex* index_create(void)`
- `void index_destroy(InvertedIndex *index)`
- `int index_insert(InvertedIndex *index, const char *term, Posting posting)`
- `Posting* index_lookup(const InvertedIndex *index, const char *term)`

#### Reconciliation Resolution:
- Kept `src/index/index.c` linked directly into `indexit` and `test_runner` to guarantee zero regression for teammates' CLI and benchmark tests.
- Packaged `src/index/inverted_index.c`, `src/index/hashtable.c`, and `src/index/trie.c` into `indexit_core`, and verified them via `test_engine.cpp`.
- Neither symbol set conflicts at link time because the static archive symbols are only resolved when referenced.

---

## 4. CMakeLists.txt Modifications

The root `CMakeLists.txt` was updated with the following minimal changes:
1. **Added CXX Support:** Changed project declaration to `project(IndexIt VERSION 0.1.0 LANGUAGES C CXX)` and enabled C++17 (`set(CMAKE_CXX_STANDARD 17)`).
2. **Defined Unicode Build Flags:** Added `add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)` to match Win32 wide-API expectations.
3. **Fixed Teammate Link Error:** Added `src/dataset/dataset.c` to `add_executable(indexit)` to resolve unresolved external symbol `index_dataset_folder`.
4. **Added `indexit_core` Target:** Static library compiling `winfs.c`, `extractor.c`, `tokenizer.c`, `IExtractor.cpp`, `hashtable.c`, `trie.c`, `inverted_index.c`, `ITermIndex.cpp`, `IRanker.cpp`, `CppInvertedIndex.cpp`, and `UI.cpp`.
5. **Linked `indexit_core` to `indexit`:** Enables terminal UI and core engine execution from the CLI.
6. **Added `test_engine` Target:** C++ test runner testing tokenizer, trie, hash table, composite index, filesystem walker, and BM25 ranker.

---

## 5. Integration Fixes Log

| Issue # | Component | Description of Problem | Exact Resolution |
|---|---|---|---|
| **Fix 1** | `CMakeLists.txt` | `main.c` referenced `index_dataset_folder`, but `dataset.c` was omitted from build targets. | Added `src/dataset/dataset.c` to `add_executable(indexit)`. |
| **Fix 2** | `include/hashtable.h` | `src/index/hashtable.c` implements `hash_lookup_count`, but it was omitted from the header, causing implicit declaration warnings. | Added `size_t hash_lookup_count(const HashTable *table, const char *key);` and `#include <stddef.h>` with `extern "C"`. |
| **Fix 3** | `include/trie.h` | `src/index/trie.c` implements `trie_search_count`, but it was omitted from the header. | Added `size_t trie_search_count(const Trie *trie, const char *term);` and `#include <stddef.h>` with `extern "C"`. |
| **Fix 4** | `src/IExtractor.cpp` | Depended on `MappedFile` and `IOException` from `RAII.hpp` and `Exceptions.hpp`, which were missing from the work package. | Retrieved `include/RAII.hpp` and `include/Exceptions.hpp` from git commit history and saved in clean UTF-8. |
| **Fix 5** | `src/UI.cpp` | In non-interactive terminals (e.g. redirected output), `GetConsoleScreenBufferInfo` failed, causing negative buffer dimensions and triggering `std::length_error: basic_string::_M_replace_aux`. | Added boundary checks in `StatusBar::draw` and a non-interactive detection guard in `ui_launch` to cleanly run ingestion and report stats without crashing. |
| **Fix 6** | `src/UI.cpp` | `extern "C"` inclusion of core headers was placed inside nested `namespace indexit::ui`, causing global `ui_launch` to fail to find `extractor_init`. | Moved C header includes to file global scope outside namespace. |
| **Fix 7** | `tests/test_engine.cpp` | Out-of-source CMake build directories could not find relative test dataset `data\small_set`. | Added path resolution fallback checking `data\small_set` and `D:\IndexIT_Reconciled\data\small_set`. |
| **Fix 8** | `src/main.c` | Lacked CLI command dispatch for `ui`. | Added `int ui_launch(const char* folder)` prototype, updated `usage()`, and added `else if (strcmp(argv[1], "ui") == 0)` dispatch. |
