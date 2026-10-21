# IndexIt Reconciled — Architectural & Integration Decisions

**Date:** 2026-10-21  
**Project:** IndexIt: Optimized Local Document Search Engine for Windows  
**Author:** Senior C/C++ Integration Engineer  

---

## 1. Branch Strategy and Integration Base
- **Context:** The repository has three branches: `master`, `Benchmarking`, and `feature/core-engine-and-cli`.
- **Finding:** Git inspection shows that `Benchmarking` (commit `2de3dfd` by Pratyay) was merged into `master` via PR #4 (`0539434`). Subsequently, `ujjwalcodess` merged PR #5 (`c2274c8`, commit `1d5aee6`) on October 17, 2026. The `feature/core-engine-and-cli` branch (commit `da268e9` by Dhananjay Sharma, Oct 2, 2026) was never merged into `master`.
- **Decision:** Base all work in `WORKDIR` on `master` (the repository default and newest branch containing teammates' work). Reconcile `DJ_WORK` directly into `WORKDIR` while preserving all teammate commits uncommitted.

---

## 2. Document Struct Conflict (`include/document.h`)
- **Context:** In `D:\IndexIT_Reconciled\include\document.h`, Document is defined with:
  ```c
  typedef enum { FORMAT_UNKNOWN, FORMAT_TXT, FORMAT_PDF, FORMAT_DOCX } DocFormat;
  typedef struct { uint32_t id; char *path; DocFormat format; uint64_t size_bytes; time_t last_modified; } Document;
  ```
  In `D:\DJ_WORK\include\document.h`, Document is defined with:
  ```c
  typedef enum { FORMAT_UNKNOWN = 0, FORMAT_TXT, FORMAT_CSV, ..., FORMAT_ZIP } DocumentFormat;
  typedef struct { uint64_t id; wchar_t *path; uint64_t size; time_t timestamp; DocumentFormat format; } Document;
  ```
- **Rule Applied:** Phase 3, Rule 3 ("Where both versions are real and differ, do NOT overwrite the teammate's code. Keep the repo version, bring DJ_WORK's in under a clearly named alternative file or adapter if needed to preserve functionality...").
- **Decision:** Keep the repository's `include/document.h` unmodified. Introduce `include/document_dj.h` containing DJ's wide-character Win32-compatible `Document` and `DocumentFormat`. Wire DJ's `winfs.h`, `extractor.h`, and `IExtractor.hpp` to use `document_dj.h`. This ensures 100% safety for existing repo headers while allowing Windows native UTF-16 path traversal to build without issue.

---

## 3. Tokenizer Interface Separation (`include/token.h` & `include/tokenizer.h`)
- **Context:** Teammates' `include/token.h` defines `Token` and `Posting`. DJ_WORK's `include/token.h` defines the same `Token` and `Posting` plus forward declarations for `Tokenizer` (`tokenizer_create`, `tokenizer_next`, `tokenizer_destroy`).
- **Decision:** Rather than overwriting the teammate's `include/token.h`, provide `include/tokenizer.h` (or keep `token.h` compatible with `Tokenizer` declarations in a non-breaking way). This cleanly decouples the token storage model from the tokenizer streaming engine.

---

## 4. Inverted Index Dual Implementation Collision
- **Context:** `src/index/index.c` (written by Pratyay) and `src/index/inverted_index.c` (written by Ujjwal / DJ) define the identical function prototypes:
  `index_create`, `index_destroy`, `index_insert`, `index_lookup`.
- **Finding:** Pratyay's `index.c` is a linked-list stub created so that `query.c` and `main.c` could be developed before the real Hash+Trie index was merged.
- **Rule Applied:** Phase 3, Rule 5 ("Never remove or rewrite teammates' working modules").
- **Decision:** Retain `src/index/index.c` in the main `indexit` CLI target so that teammate's `query.c`, `dataset.c`, and `benchmark.c` function without disruption. Simultaneously, compile `src/index/inverted_index.c`, `src/index/hashtable.c`, and `src/index/trie.c` into a dedicated core library target (`indexit_core`) and unit test target (`test_engine`) to ensure the full FNV-1a Hash Table and 36-ary Trie are verified without symbol collision.

---

## 5. Teammate Build Fix: Missing `src/dataset/dataset.c` in CMakeLists.txt
- **Context:** Running `cmake --build .` on the unmodified repo produces:
  `undefined reference to 'index_dataset_folder'` at `main.c:44, 92, 163`.
- **Finding:** Pratyay created `src/dataset/dataset.c` which implements `index_dataset_folder`, but forgot to add `src/dataset/dataset.c` to `add_executable(indexit)` in `CMakeLists.txt`.
- **Rule Applied:** Phase 3, Rule 4 ("Update CMakeLists.txt minimally so everything compiles. Note every change").
- **Decision:** Add `src/dataset/dataset.c` to `add_executable(indexit)` in `CMakeLists.txt`. This restores the teammate's intended build without altering any logic.

---

## 6. Dependency Resolution for `src/IExtractor.cpp` (`RAII.hpp`)
- **Context:** `DJ_WORK/src/IExtractor.cpp` includes `"RAII.hpp"`, which uses `MappedFile` and throws `IOException`. However, `RAII.hpp` was not included in the isolated `DJ_WORK` folder. It exists in the project git history on `origin/feature/core-engine-and-cli`.
- **Decision:** Import `include/RAII.hpp` and `include/Exceptions.hpp` from the project's feature branch. This resolves the header dependency cleanly and enables `IExtractor.cpp` to compile without rewriting its memory-mapped file logic.

---

## 7. Handling of Root `src/trie.c`
- **Context:** `src/trie.c` in the repository root is a 39-line placeholder stub (returns 0, empty struct) from an early skeleton commit. `src/index/trie.c` is the real 143-line 36-ary Trie implementation.
- **Decision:** Do not delete or alter `src/trie.c` in git tracking, but exclude it from CMake build targets in favor of `src/index/trie.c`.

---

## 8. CLI Command Integration (`stats`, `clean`, `ui`)
- **Context:** Teammate's `src/main.c` implements `index`, `search`, `benchmark`, prints stubs for `stats` and `clean`, and does not support `ui`. Phase 3 Rule 6 requests running CLI demo commands `index`, `search`, `stats`, `clean`, `ui`.
- **Decision:** Preserve `src/main.c` as the primary C CLI. Add lightweight, non-invasive handling in the CLI or complementary demo commands so that `ui`, `stats`, and `clean` can be demonstrated on sample data without breaking teammate's `main.c`.
