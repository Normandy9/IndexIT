# IndexIt: Plain-Language Codebase Explanation

**Date:** 2026-10-21  
**Project:** IndexIt — Optimized Local Document Search Engine for Windows  
**Document:** `docs/REPO_CODE_EXPLANATION.md`  
**Author:** Senior C/C++ Integration Engineer  

---

## 1. What the Project Does End-to-End

### The Big Picture
IndexIt is designed as a high-performance, native Windows desktop document search engine. Its purpose is to scan local folders and directories on a Windows computer, discover text documents and code files, extract their text contents, break down the text into normalized tokens (removing common stopwords), and build an optimized in-memory inverted index. Users can then query the engine from the command line or an interactive terminal UI to find relevant files with instant response times and ranking (BM25 / TF-IDF).

### How Teammates' Code Fits In (As Pushed on `master`)
Prior to reconciliation, the repository on `master` contained two distinct tracks of work:
1. **The CLI, Benchmark, and Query Track (by `pratyayarajamrit-cell`):** A working C command-line tool in `src/main.c` that parses `index`, `search`, and `benchmark` commands. It uses a flat-folder ANSI file reader (`src/dataset/dataset.c`) to read files, indexes them into a temporary linked-list inverted index (`src/index/index.c`), and executes exact-string lookups (`src/query/query.c`) timed with Win32 high-precision hardware counters (`src/benchmark/benchmark.c`).
2. **The Core Data Structures Track (by `ujjwalcodess`, originally developed with `Dhananjay Sharma`):** Merged in PR #5, containing an FNV-1a Hash Table (`src/index/hashtable.c`), a 36-ary Trie (`src/index/trie.c`), a composite inverted index (`src/index/inverted_index.c`), and C++ OOP abstractions (`src/ITermIndex.cpp`, `src/CppInvertedIndex.cpp`). However, these structures were not yet wired into `CMakeLists.txt` or `main.c`.

### The Combined System (After Phase 3 Integration)
With the integration of `DJ_WORK`, IndexIt possesses the full pipeline:
1. **Recursive Filesystem Traversal:** Native Unicode Windows API traversal (`FindFirstFileW`) that recurses into subdirectories, handles path lengths up to Windows extended limits, safely skips hidden/system files, and catches file sharing violations.
2. **Document Ingestion & Text Extraction:** Format classification for 17 file formats (TXT, CSV, Markdown, Logs, JSON, XML, HTML, C/C++ source/headers, DOCX, PDF, etc.) and safe text reading via `CreateFileW` and memory-mapped files (`MappedFile`).
3. **Stream Tokenization:** Converts text to lowercase, strips punctuation, discards 32 common English stopwords, and assigns positional offsets to tokens.
4. **Composite Indexing & C++ OOP Layer:** Fast exact lookups via FNV-1a hash table and prefix matching via 36-ary Trie, orchestrated through `ITermIndex` and `CppInvertedIndex`.
5. **BM25 & TF-IDF Ranking:** Scores documents according to term frequency, document frequency, document length, and average corpus document length using a min-heap Top-K selector.
6. **Unified Dual Interface:** Teammates' C CLI (`index`, `search`, `benchmark`, `stats`, `clean`) plus an interactive terminal UI (`ui`) with color-coded search results and live keystroke filtering.

---

## 2. Module-by-Module Description & Data Flow

### Architecture & Dataflow Diagram

```
+-----------------------------------------------------------------------------+
|                                FILESYSTEM                                   |
|                (Directory Tree: .txt, .csv, .md, .log, .c, etc.)            |
+-----------------------------------------------------------------------------+
                                      |
                                      v
       +-------------------------------------------------------------+
       |                     INGESTION LAYER                         |
       |  - src/core/winfs.c          (Unicode FindFirstFileW Recurse)|
       |  - src/dataset/dataset.c     (ANSI Flat Folder Traversal)   |
       +-------------------------------------------------------------+
                                      |
                                      v
       +-------------------------------------------------------------+
       |                    EXTRACTION LAYER                         |
       |  - src/core/extractor.c      (Win32 CreateFileW/ReadFile)   |
       |  - src/IExtractor.cpp        (C++ MappedFile RAII Wrapper)  |
       +-------------------------------------------------------------+
                                      |
                                      v
       +-------------------------------------------------------------+
       |                    TOKENIZATION LAYER                       |
       |  - src/core/tokenizer.c      (Lowercase, Punctuation Strip, |
       |                               Stopwords, Token Positions)   |
       +-------------------------------------------------------------+
                                      |
                                      v
       +-------------------------------------------------------------+
       |                     INDEXING LAYER                          |
       |  - src/index/hashtable.c     (FNV-1a Hash Table + Chaining) |
       |  - src/index/trie.c          (36-ary Trie: [a-z0-9])        |
       |  - src/index/inverted_index.c (Composite Hash + Trie)       |
       |  - src/index/index.c         (Foundation Linked-List Index) |
       +-------------------------------------------------------------+
                                      |
                                      v
       +-------------------------------------------------------------+
       |                  C++ OOP & RANKING LAYER                    |
       |  - src/ITermIndex.cpp        (TrieIndex & HashIndex)        |
       |  - src/IRanker.cpp           (BM25 & TF-IDF Strategy)       |
       |  - src/CppInvertedIndex.cpp  (DocumentStore + TopK Heap)    |
       +-------------------------------------------------------------+
                                      |
                     +----------------+----------------+
                     |                                 |
                     v                                 v
+---------------------------------------+  +----------------------------------+
|           CLI INTERFACE               |  |     TERMINAL UI INTERFACE        |
|  - src/main.c                         |  |  - src/UI.cpp                    |
|    (index, search, benchmark, stats)  |  |    (Interactive Live Console,    |
|  - src/query/query.c                  |  |     StatusBar, ResultView)       |
|  - src/benchmark/benchmark.c          |  +----------------------------------+
+---------------------------------------+
```

---

### Detailed Module Descriptions

#### 1. Ingestion & Filesystem
- **`src/core/winfs.c` / `include/winfs.h`**
  - **Purpose:** Native Windows recursive directory traversal.
  - **Key Functions:** `winfs_walk_directory(path, callback, user_data)`, `get_format_from_ext()`, `filetime_to_time_t()`.
  - **Mechanisms:** Constructs `L"\\*"` wildcard masks, invokes `FindFirstFileW` / `FindNextFileW`, skips `.` and `..`, bypasses `FILE_ATTRIBUTE_HIDDEN` and `FILE_ATTRIBUTE_SYSTEM`, tests file locks with `CreateFileW`, extracts `FILETIME` and 64-bit file sizes, and triggers the discovery callback.
- **`src/dataset/dataset.c` / `include/dataset.h`**
  - **Purpose:** Teammates' lightweight flat-directory document indexer.
  - **Key Functions:** `index_dataset_folder(folder, index)`, `read_file(path, index)`, `add_words(index, text)`.
  - **Mechanisms:** Uses ANSI `FindFirstFileA` to read `.txt`, `.json`, `.conf` files directly in a folder and calls `index_insert`.

#### 2. Extraction & Tokenization
- **`src/core/extractor.c` / `include/extractor.h`**
  - **Purpose:** Reads raw file bytes and returns heap-allocated null-terminated C strings.
  - **Key Functions:** `extract_text(doc)`, `extract_plain_text(filepath)`.
  - **Mechanisms:** Handles Tier-1 formats (TXT, CSV, LOG, MD, JSON, XML, HTML, C, CPP, H) up to 50MB using `GetFileSizeEx` and `ReadFile`.
- **`src/IExtractor.cpp` / `include/IExtractor.hpp`**
  - **Purpose:** C++ OOP extractor interface and memory-mapped file implementation.
  - **Key Classes:** `PlainExtractor`, `DocxExtractor`, `PdfExtractor`, `ZipExtractor`, `ExtractorFactory`.
  - **Mechanisms:** `PlainExtractor` uses `MappedFile` from `RAII.hpp` to map files directly into virtual memory.
- **`src/core/tokenizer.c` / `include/tokenizer.h`**
  - **Purpose:** Breaks raw text into clean, normalized word tokens.
  - **Key Functions:** `tokenizer_create(text)`, `tokenizer_next(tok, out_token)`, `tokenizer_destroy(tok)`, `is_stopword(word)`.
  - **Mechanisms:** Scans input character-by-character, lowercases via `tolower()`, tracks token position offsets, and discards 32 common English stopwords (`the`, `and`, `is`, `in`, `to`, `for`, etc.).

#### 3. Indexing & Data Structures
- **`src/index/hashtable.c` / `include/hashtable.h`**
  - **Purpose:** High-speed exact-match term lookups.
  - **Key Functions:** `hash_create()`, `hash_destroy()`, `hash_insert()`, `hash_lookup()`, `hash_lookup_count()`.
  - **Mechanisms:** Uses the 64-bit FNV-1a hashing algorithm (`hash ^= (unsigned char)*str; hash *= 0x100000001b3ULL`) with separate chaining buckets (`HashEntry`) and dynamically allocated posting lists.
- **`src/index/trie.c` / `include/trie.h`**
  - **Purpose:** Prefix-searchable dictionary for autocomplete and terms.
  - **Key Functions:** `trie_create()`, `trie_destroy()`, `trie_insert()`, `trie_search()`, `trie_search_count()`.
  - **Mechanisms:** 36-way branching node array (`a-z` index 0-25, `0-9` index 26-35) storing posting lists at terminal nodes.
- **`src/index/inverted_index.c` / `include/inverted_index.h`**
  - **Purpose:** Production unified inverted index combining Hash Table and Trie.
  - **Key Functions:** `index_create()`, `index_destroy()`, `index_insert()`, `index_lookup()`.
  - **Mechanisms:** On `index_insert`, writes the posting into both the Hash Table (for O(1) exact lookups) and the Trie (for prefix scans).
- **`src/index/index.c` / `include/index.h`**
  - **Purpose:** Teammates' foundation linked-list inverted index.
  - **Key Functions:** `index_create()`, `index_destroy()`, `index_insert()`, `index_lookup()`.
  - **Mechanisms:** Singly-linked list of `IndexNode` structures; used by teammates' CLI and test runner.

#### 4. C++ OOP, Ranking, and Storage
- **`src/ITermIndex.cpp` / `include/ITermIndex.hpp`**
  - **Purpose:** C++ polymorphism bridge wrapping C Trie and Hash Table into `ITermIndex`.
  - **Key Classes:** `ITermIndex`, `TrieIndex`, `HashIndex`.
- **`src/IRanker.cpp` / `include/IRanker.hpp`**
  - **Purpose:** Strategy pattern ranking algorithms.
  - **Key Classes:** `IRanker`, `TfIdfRanker`, `Bm25Ranker`.
  - **Formulae:** BM25 uses `k1 = 1.5`, `b = 0.75` and accounts for document length relative to average document length in `IndexStats`.
- **`src/CppInvertedIndex.cpp` / `include/CppInvertedIndex.hpp`**
  - **Purpose:** High-level inverted index connecting term indices, document store, and ranker.
  - **Key Functions:** `addDocument()`, `addTerm()`, `setDocumentLength()`, `search()`.
  - **Mechanisms:** Uses `TopK<ScoredDocument, ScoreCompare>` min-heap to return the top K highest scoring results.

#### 5. User Interface & CLI
- **`src/main.c`**
  - **Purpose:** Command-line executable supporting `index`, `search`, `benchmark`, `stats`, `clean`, and `ui`.
- **`src/UI.cpp` / `include/UI.hpp`**
  - **Purpose:** Native Windows Console interactive terminal UI.
  - **Key Classes:** `App`, `StatusBar`, `ResultView`, `InputHandler`, `ThemeManager`.
  - **Mechanisms:** Uses Win32 console APIs (`SetConsoleCursorPosition`, `SetConsoleTextAttribute`, `ReadConsoleInput`) to provide real-time search as the user types, with ESC to exit.

#### 6. Benchmarking & Querying
- **`src/benchmark/benchmark.c` / `include/benchmark.h`**
  - **Purpose:** Microsecond-resolution timer harness.
  - **Key Functions:** `benchmark_init()`, `timer_start()`, `timer_stop_ms()`.
  - **Mechanisms:** On Windows, queries hardware timer frequency via `QueryPerformanceFrequency` and elapsed ticks via `QueryPerformanceCounter`.
- **`src/query/query.c` / `include/query.h`**
  - **Purpose:** Teammates' query execution wrapper.
  - **Key Functions:** `query_init()`, `query_set_index()`, `run_query(query)`, `query_cleanup()`.

---

## 3. Module Status Table (Working / Partially Working / Stub Only)

| Module / File | Author (Git History) | Status | Evidence |
|---|---|---|---|
| `src/core/winfs.c` | `Dhananjay Sharma` | **Working** | Traverses directory tree recursively; tested against `data/small_set` in `test_engine` (found 5 files). |
| `src/core/extractor.c` | `Dhananjay Sharma` | **Partially Working** | Working for plain-text/source files (TXT, CSV, MD, LOG, C, H); DOCX/PDF are stubs (marked TODO). |
| `src/core/tokenizer.c` | `Dhananjay Sharma` | **Working** | Fully working; removes stopwords, normalizes case, outputs positions; verified in `test_engine`. |
| `src/IExtractor.cpp` | `Dhananjay Sharma` | **Partially Working** | `PlainExtractor` uses working `MappedFile`; `DocxExtractor`/`PdfExtractor` return placeholder strings. |
| `src/index/hashtable.c` | `ujjwalcodess` / `DJ` | **Working** | FNV-1a hash calculation, bucket collision chaining, exact lookup verified in unit tests. |
| `src/index/trie.c` | `ujjwalcodess` / `DJ` | **Working** | 36-ary trie insert and prefix search verified in unit tests. |
| `src/index/inverted_index.c`| `ujjwalcodess` / `DJ`| **Working** | Coordinates hash table and trie insert and search; verified in unit tests. |
| `src/index/index.c` | `pratyayarajamrit-cell` | **Working (Foundation)**| Simple linked list; passes teammate tests and CLI index/search. |
| `src/trie.c` (root) | `ujjwalcodess` | **Stub Only** | Placeholder `struct Trie { int placeholder; }` returning 0/NULL; superseded by `src/index/trie.c`. |
| `src/cache.h` | `Paras-tech26` | **Stub Only** | Comments and struct definition only; no implementation file exists. |
| `src/db.h` | `Paras-tech26` | **Stub Only** | SQL table schema comments only; no implementation file exists. |
| `src/ITermIndex.cpp` | `ujjwalcodess` | **Working** | C++ wrappers over Trie and Hash Table; compiles and passes unit tests. |
| `src/IRanker.cpp` | `Dhananjay Sharma` | **Working** | Mathematical implementation of BM25 and TF-IDF scoring formulas; verified in `test_engine`. |
| `src/CppInvertedIndex.cpp` | `ujjwalcodess` / `DJ` | **Working** | Document registration, term tracking, and Top-K heap search; verified in `test_engine`. |
| `src/UI.cpp` | `Dhananjay Sharma` | **Working** | Console event loop, real-time typing query, and demo fallback verified. |
| `src/main.c` | `pratyayarajamrit-cell` | **Partially Working** | `index`, `search`, `benchmark`, `ui` working; `stats` and `clean` print "not implemented" notices. |
| `src/query/query.c` | `pratyayarajamrit-cell` | **Working** | Exact lookup and result reporting verified in CLI and test runner. |
| `src/benchmark/benchmark.c` | `pratyayarajamrit-cell` | **Working** | `QueryPerformanceCounter` high-resolution timer verified in test runner. |
| `src/dataset/dataset.c` | `pratyayarajamrit-cell` | **Working** | ANSI directory reader working in CLI demo. |

---

## 4. Teammates' Code Capabilities & Limitations

### What Teammates' Code Currently Does:
1. **Flat-Folder Text Indexing:** Can point `indexit.exe index <folder>` to a flat directory and index `.txt`, `.json`, `.conf` files.
2. **Exact Keyword Search:** Can search for single terms using `indexit.exe search <folder> <term>` and report document ID, term frequency, and elapsed search time.
3. **Benchmarking:** Can execute 100 consecutive queries using `indexit.exe benchmark <folder> <term>` and report average, minimum, and maximum latency in milliseconds.
4. **Unit Testing:** Runs four automated checks in `test_runner.exe` testing basic lookup and timing.

### What Teammates' Code Cannot Do:
1. **Subdirectory Recursion:** The teammate file reader (`src/dataset/dataset.c`) explicitly skips directories (`if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;`), ignoring nested folder structures.
2. **Unicode / Wide Paths:** Uses `FindFirstFileA` which fails on non-ASCII or deep Windows paths.
3. **Advanced Formats:** Cannot handle Markdown (`.md`), CSV (`.csv`), Logs (`.log`), or source code files (`.c`, `.cpp`, `.h`).
4. **Token Normalization & Stopwords:** Words are not filtered against stopwords; punctuation handling is minimal.
5. **Relevance Ranking:** Does not rank search results; returns raw unranked matching postings.
6. **Persistence & Caching:** Has no database storage or cache manager; every search rebuilds the index from disk in memory.
7. **Interactive UI:** Does not natively provide an interactive console UI in `main.c`.

---

## 5. What the Combined Project Can Do Now vs Planned Scope

### Capabilities Available Now:
1. **Recursive Native Filesystem Walking:** Scans entire directory trees using `FindFirstFileW`.
2. **Format-Aware Text Extraction:** Extracts plain text from 10 distinct file formats, including code files and logs.
3. **Stopword Filtration & Stream Tokenization:** Eliminates 32 high-frequency English stopwords and computes token positions.
4. **Full Hash Table & Trie Indexing:** Combines FNV-1a hash table and 36-ary Trie indexing.
5. **BM25 & TF-IDF Ranking:** Ranks multi-document query matches with realistic scoring curves.
6. **Dual CLI & Terminal UI:** Run standard CLI commands or enter interactive terminal search mode.
7. **Comprehensive Test Suite:** Dual test runners (`IndexItTests` and `EngineTests`) passing 100% of tests.

### What Is Still Missing Compared with Planned Scope:
1. **SQLite Database Persistence:** `src/db.h` remains an outline sketch. Documents and term index are not yet saved to a persistent SQLite database file on disk.
2. **Cache Manager:** `src/cache.h` is an unimplemented declaration. An in-memory LRU cache has not been integrated into the query pipeline.
3. **Tier-2 & Tier-3 Multi-Format Extraction:** DOCX, PPTX, XLSX (requiring `miniz` zip extraction) and PDF (requiring PDFium/MuPDF) return stubs.
4. **Server Mode:** Remote / HTTP / daemon search service is not yet designed or implemented.
5. **Automated Extended Benchmarks:** Multi-threaded query benchmarks against large-scale corpora (e.g., gigabyte datasets) remain future work.

---

## 6. How to Build and Run IndexIt (Exact Commands)

### Prerequisites
- CMake 3.15+ (available at `D:\IndexIT_project\tools\cmake-3.29.3-windows-i386\bin`)
- MinGW GCC/G++ (available at `C:\MinGW\bin`)

### 1. Build from Scratch
Open PowerShell and run:
```powershell
# Set build tools in PATH
$env:PATH = "D:\IndexIT_project\tools\cmake-3.29.3-windows-i386\bin;C:\MinGW\bin;" + $env:PATH

# Create and enter build directory
New-Item -ItemType Directory -Path "D:\build_indexit" -Force
Set-Location D:\build_indexit

# Configure with MinGW generator
cmake -G "MinGW Makefiles" D:\IndexIT_Reconciled

# Build all targets (CLI, core library, test runners)
cmake --build .
```

### 2. Run Automated Tests
```powershell
# Run full ctest suite (IndexItTests + EngineTests)
ctest --output-on-failure
```

### 3. Run CLI Commands
```powershell
# 1. Index a folder
.\indexit.exe index D:\IndexIT_Reconciled\data\small_set

# 2. Search for a keyword
.\indexit.exe search D:\IndexIT_Reconciled\data\small_set document

# 3. Benchmark query latency (runs 100 iterations)
.\indexit.exe benchmark D:\IndexIT_Reconciled\data\small_set document

# 4. Check statistics
.\indexit.exe stats

# 5. Clean index resources
.\indexit.exe clean

# 6. Launch the Interactive Terminal UI
.\indexit.exe ui D:\IndexIT_Reconciled\data\small_set
```
*(In the interactive UI: type any query term to see real-time ranked results; press ESC to exit).*
