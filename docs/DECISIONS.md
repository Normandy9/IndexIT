# Decisions Log

### Scope 1: File Walker
- **Unicode long paths:** Attempted to ensure native Windows handling via #define UNICODE and wprintf/wchar_t. MAX_PATH * 4 buffers are used internally. Documented that actual long-path support deep inside the OS relies on the Windows 10 Long Path registry enablement, but our buffers safely prevent overflows.
- **50MB Limit:** Added an explicit size check in winfs.c to reject files > 50MB before ever attempting to MapViewOfFile them, protecting against std::bad_alloc OOM crashes.
- **Symlink Loops:** Prevented infinite loops by checking for FILE_ATTRIBUTE_REPARSE_POINT and skipping these directories/files.

### Scope 2: Tokenizer
- **Stopwords:** Added 	okenizer_init_stopwords(filepath) to dynamically load stopwords from a file, allocating them safely up to a threshold. Reverts to the static list if the file is absent.
- **Garbled Text:** Verified that the current 	olower((unsigned char)c) parsing is robust against negative byte values in non-UTF-8 strings and does not crash, maintaining stability without requiring heavy UTF-16 conversions for this prototype phase.

### Scope 3: Trie & Hash Index
- **Prefix Search:** Added 	rie_prefix_search with an internal node traversal and a posting array collection algorithm to aggregate matching prefix document lists safely.
- **Benchmarking Hooks:** Added 	rie_insert_timed, hash_insert_timed, and lookup timed wrappers using Windows QueryPerformanceCounter to cleanly feed the future Benchmark module without disrupting core logic.

### Scope 4: Inverted Index & Ranker
- **Multi-word AND queries:** Modified CppInvertedIndex::search() to use the core 	okenizer.c. If a query contains multiple words, it tokenizes them, retrieves the postings for each word, and intersects them efficiently by document ID (AND-logic).
- **Edge cases:** Bm25Ranker mathematics correctly degrade to positive logical scores for single-document corpus queries.

### Scope 5: OOP Layer
- **Threading support (MinGW constraints):** Removed std::mutex and <mutex> includes from LruCache.hpp. The MinGW GCC 6.3 Windows build environment used lacks POSIX <mutex> support by default. Since the current application runs entirely single-threaded on the frontend, this was removed to allow successful compilation against the team's build toolchain.

### Explicitly Out of Scope
As requested, I did not implement or stub out the actual backend persistence logic for CacheManager (LRU cache disk flushing), db.cpp (SQLite DB initialization/closing), Server Mode (named pipes/JSON parsing), or tier-2/3 extractors. Any UI or core hooks to these either print placeholder logs or safely no-op to allow the team's persistence engineers to implement them later.


### Step 1: Consolidated Architecture
- **OOP Ingestion Pipeline**: Ingestion now flows entirely through `IExtractor`, `ExtractorFactory`, `ITermIndex`, `IRanker`, and `CppInvertedIndex::indexDirectory()`. Trampoline callbacks and static pointers in `main.cpp` and `UI.cpp` were eliminated in favor of contextual callback passing.
- **Hash Table Resizing**: Implemented dynamic table growth in `hashtable.c`. The table starts at a configurable initial capacity (default 1,024, or custom via `hash_create_with_capacity`), and doubles its bucket array whenever `(num_entries + 1) / num_buckets > 0.75`.
- **Pre-sorted Postings**: Both `HashTable` and `Trie` now maintain posting lists in strictly sorted order by `document_id` via binary search insertion. This eliminates per-query sorting during multi-term queries and enables linear two-pointer merge intersection in `CppInvertedIndex::search()`.
- **CacheManager Toolchain Compatibility**: Replaced problematic `std::optional` signatures in `CacheManager.hpp` with standard boolean return `get(key, out_value)`, making it fully compatible with MinGW GCC 6.3 while maintaining clean LRU semantics.
- **ScopedTimer**: Added microsecond, millisecond, and fractional second profiling methods in `RAII.hpp` based on `std::chrono::high_resolution_clock`.


### Step 2: Trie Prefix Search and Autocomplete
- **Query Prefix Syntax**: Supported `term*` wildcard syntax. The query parser identifies tokens ending in `*` and routes them to `prefixIndex_->lookupPrefix()`, which leverages `trie_prefix_search` to aggregate postings for all matching terms while preserving document ID sort order.
- **Trie Autocomplete Algorithm**: Implemented `trie_autocomplete()` in `trie.c`. It walks to the prefix node, collects all descendents with positive postings counts via DFS, and ranks them by document frequency (descending) with alphabetical tie-breaking.
- **Interactive UI Autocomplete**: Connected live suggestions to Line 1 of the terminal UI right below the search input, rendering real-time matching suggestions with document frequency counts on every keypress.
- **CLI Suggest Command**: Added `suggest <dir> <prefix>` command to index a directory and output the top 10 ranked prefix completions with document frequencies.


### Step 3: Persistent Index and Cache Subsystem
- **Binary Format & CRC32**: Formatted binary `index.cache` with magic header `"INDEXIT\0"`, version 1, and IEEE 802.3 CRC-32 checksum covering the entire payload. Corrupted headers, version mismatches, or CRC checksum errors trigger an immediate graceful fallback with discard/rebuild without crashing.
- **Crash-Safe Atomic Write**: Writes are directed to a per-process temporary file `index.cache.tmp.<pid>`, flushed via `FlushFileBuffers()`, closed, and atomically swapped into place via Win32 `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH`.
- **Incremental Index Invalidation**: During index updates, `DocumentStore` metadata (file size in bytes and 64-bit Win32 `FILETIME` last write timestamp) is compared against on-disk attributes. Files with unchanged metadata are preserved without re-reading or re-tokenizing. Deleted files are pruned.
- **Disk-Space Guard**: Queries volume space via `GetDiskFreeSpaceExW()` prior to initiating write operations. If free space is beneath a configurable threshold (default 50MB), the write is safely aborted with a warning, allowing the search engine to continue operating in memory-only mode.
- **Safe Cache Manifest & Clean**: Every generated cache artifact is tracked in `manifest.txt`. The `clean` command inspects tracked files, outputs their individual and aggregate sizes, demands interactive confirmation (bypassable with `--yes`), and validates path boundaries before deleting files exclusively within the designated cache directory.
- **LRU Document Text Byte Cap**: Enhanced `CacheManager` with a hard byte capacity (default 32MB) alongside item count bounds, tracking cumulative bytes and evicting LRU parsed document contents when the threshold is exceeded.


### Step 4: SQLite Persistence Layer
- **Vendored Amalgamation**: Vendored official SQLite 3.45.2 amalgamation in `third_party/sqlite3/sqlite3.c` and `sqlite3.h`.
- **RAII SQLite Wrappers**: Built `SqliteConnection` (handles open/close and raw SQL executions), `SqliteStatement` (wraps preparation, parameterized bindings for int64/int/text/wide UTF-8 conversions, stepping, and column extractions), and `SqliteTransaction` (guarantees automated rollback upon failure unless explicitly committed).
- **Normalized Schema**:
  - `documents(id INTEGER PRIMARY KEY, path TEXT NOT NULL UNIQUE, size INTEGER, mtime INTEGER, format INTEGER, doc_len INTEGER)` with index `idx_documents_path`
  - `cache_manifest(path TEXT NOT NULL UNIQUE, size INTEGER, created TEXT)` with index `idx_manifest_path`
  - `search_history(id INTEGER PRIMARY KEY AUTOINCREMENT, query TEXT NOT NULL, ts TEXT NOT NULL, result_count INTEGER)`
- **Persistent Search History & CLI**: Every search query and result count is logged to `search_history`. Added `indexit history [--cache-dir <dir>]` CLI command to inspect recent searches.

### Step 5: Multi-format Extraction
- **Miniz packaging repair**: The checked-in miniz source/header pair referenced component headers that were absent. Added the matching 3.1.2 headers and inflate/deflate components under `third_party/miniz`; the engine links the needed inflate components. The archive central directory is parsed locally so output limits can be enforced before indexing.
- **PDF fallback**: PDFium/MuPDF were not introduced. The fallback extracts literal strings inside uncompressed streams and FlateDecode streams, supporting the generated fixture. It is not a general PDF parser: object streams, font encodings, encrypted PDFs, and OCR are unsupported.
- **Archive limits**: ZIP input is capped at 50 MiB, each decompressed entry at 16 MiB, aggregate extracted text at 64 MiB, entry count at 10,000, and entries with a reported compression ratio above 200 are skipped.

### Step 6: Query features
- Implemented bounded edit-distance suggestions as a first increment and display CLI suggestions when an exact query term has no postings. The current implementation enumerates hash terms; trie-based pruning, metadata filters, snippets/highlighting, duplicate detection, and UI filters remain follow-up work and are marked partial in `COMPLETION_REPORT.md`.

### Step 7: Unicode handling
- Tokenization treats high-bit UTF-8 bytes as word bytes and lowercases ASCII only. This preserves multibyte character boundaries in common UTF-8 text without claiming full Unicode classification/case folding; complete Windows path and console Unicode coverage remains outstanding.
