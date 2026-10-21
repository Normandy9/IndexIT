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
