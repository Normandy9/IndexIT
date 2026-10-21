# Ownership Split

### M1: Index Core
- **Files Owned**: `ITermIndex.hpp`, `IRanker.hpp`, `CppInvertedIndex.hpp`
- **Viva Questions**: 
  1. What is the difference between TF-IDF and BM25?
  2. How does the Trie structure help with prefix matching?
  3. Why use FNV-1a for the hash table?
  4. How does `TopK` maintain only K elements?
  5. What is the time complexity of a lookup?
  6. How is polymorphism used in `IRanker`?
  7. How do you resolve hash collisions?
  8. Why separate Exact and Prefix indices?
  9. How does the Facade pattern apply to `CppInvertedIndex`?
  10. Explain the adapter pattern over the C structures.
- **Exercise**: Add a new ranker (e.g., BooleanRanker) that returns 1.0 if term is present.

### M2: Ingestion
- **Files Owned**: `winfs.c`, `tokenizer.c`, `IExtractor.hpp`, `document.h`
- **Viva Questions**: 
  1. Why use `FindFirstFileW` instead of standard `dirent`?
  2. How does the Factory pattern work for `ExtractorFactory`?
  3. What is the role of the stopword list?
  4. Why avoid allocating large strings for every token?
  5. What are the tier 2 and tier 3 extractors?
  6. How is polymorphism achieved in `IExtractor`?
  7. Why use `wchar_t`?
  8. Explain how we handle locked files safely.
  9. Describe the memory lifetime of extracted text.
  10. How would you extract text from a zip?
- **Exercise**: Add an XML extractor using `miniz` and a basic tag stripper.

### M3: Storage & Cache
- **Files Owned**: `CacheManager.hpp`, `LruCache.hpp`, `RAII.hpp`, `db.cpp`
- **Viva Questions**: 
  1. What is RAII and why is it used?
  2. Explain the exception safety of `MappedFile`.
  3. How does `LruCache` achieve O(1) lookups and evictions?
  4. Why use a `std::list` alongside `std::unordered_map` in LRU?
  5. What is the purpose of `SqliteConnection`?
  6. How do you handle disk-space guarantees?
  7. Why use templates for `LruCache`?
  8. Explain how you simulated crash-safety for cache writes.
  9. What happens if `MapViewOfFile` fails?
  10. How is threading safety handled in LRU cache?
- **Exercise**: Implement the disk-write fallback for LRU cache using SQLite.

### M4: Query, UI & Server
- **Files Owned**: `main.cpp`, `UI.hpp`, `UI.cpp`, `TopK.hpp`
- **Viva Questions**: 
  1. How does the Windows Console API differ from ncurses?
  2. What is the Single Responsibility Principle as seen in UI classes?
  3. How do you poll for keyboard events without blocking?
  4. Why use a `StatusBar` class?
  5. How does search-as-you-type update the results?
  6. What is named pipe communication?
  7. How would you parse JSON without an external library?
  8. Explain the MVC pattern (if applicable) or separation of concerns in the terminal App.
  9. What does the CLI parsing `CommandLineToArgvW` solve?
  10. How do you gracefully handle terminal resizing?
- **Exercise**: Add a `--remote` toggle in `main.cpp` that sends queries over a named pipe instead of directly to `CppInvertedIndex`.
