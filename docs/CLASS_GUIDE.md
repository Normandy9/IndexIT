# Class guide

| Class / type | Purpose and OOP concept | File | Main API |
|---|---|---|---|
| `IExtractor` | Extraction contract; abstraction and polymorphism | `include/IExtractor.hpp` | `extract(path)` |
| `PlainExtractor`, `HtmlExtractor`, `RtfExtractor`, `EmlExtractor`, `DocxExtractor`, `PptxExtractor`, `XlsxExtractor`, `OdtExtractor`, `ZipExtractor`, `PdfExtractor` | Format-specific extraction; interface implementation | `src/IExtractor.cpp` | `extract(path)` |
| `ExtractorFactory` | Selects an extractor from document format | `include/IExtractor.hpp` | `create(format)` |
| `ITermIndex` | Exact/prefix index contract | `include/ITermIndex.hpp` | `insert`, `lookup`, `lookupPrefix` |
| `HashIndex`, `TrieIndex` | Hash and trie implementations behind the term-index interface | `src/ITermIndex.cpp` | `lookup`, `insert`, prefix suggestions |
| `CppInvertedIndex` | Owns term indexes and document metadata; indexes directories and performs AND search | `include/CppInvertedIndex.hpp` | `indexDirectory`, `search`, `suggest` |
| `IRanker` | Ranking contract | `include/IRanker.hpp` | `score(term, tf, docId, stats)` |
| `Bm25Ranker`, `TfIdfRanker` | Ranking strategy implementations | `src/IRanker.cpp` | `score(...)` |
| `DocumentStore` | Maps document IDs and paths to metadata | `include/DocumentStore.hpp` | add/find/remove and metadata access |
| `TopK<T, Compare>` | Bounded result selection using a heap | `include/TopK.hpp` | `add`, `get` |
| `LruCache<K,V>` / `CacheManager` | In-memory LRU with byte cap | `include/LruCache.hpp`, `include/CacheManager.hpp` | `put`, `get`, eviction |
| `SqliteConnection`, `SqliteStatement`, `SqliteTransaction` | SQLite RAII and prepared statement/transaction wrappers | `include/RAII.hpp`, `src/db.cpp` | `execute`, bind/step, `commit` |
| `DatabaseManager` | Schema, document metadata, manifest, and history persistence | `src/db.h`, `src/db.cpp` | `saveDocuments`, `recordSearchHistory`, `getSearchHistory` |
| `BinaryIndexCache` / `CacheManifest` | Binary index persistence, integrity check, update and cleanup | `include/BinaryIndexCache.hpp`, `src/cache/BinaryIndexCache.cpp` | `load`, `save`, `updateIndexIncremental`, `clean` |

Example:

```cpp
indexit::CppInvertedIndex index;
index.indexDirectory(L"C:\\Documents");
indexit::Bm25Ranker ranker;
auto results = index.search("project notes", ranker, 10);
```
