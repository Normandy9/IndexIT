# Architecture

```text
CLI / terminal UI
       |
       v
CppInvertedIndex -- IExtractor / ExtractorFactory
       |                     |
       |                     +-- plain, XML-in-ZIP, HTML, RTF, EML, PDF fallback
       v
Tokenizer -> HashIndex (exact terms) + TrieIndex (prefix terms)
       |
       +-- AND postings intersection -> IRanker (BM25 / TF-IDF) -> TopK
       |
       +-- BinaryIndexCache (index.cache, CRC, atomic replace, manifest)
       +-- DatabaseManager (SQLite document metadata and search history)
```

The Win32 walker reports document records to one indexing callback in `CppInvertedIndex`. The extractor factory chooses an implementation by `DocumentFormat`. Extracted UTF-8 text is tokenized; each term is inserted into the hash and trie indexes with postings kept in document-ID order. Search intersects the postings for each query term and passes term frequencies and corpus statistics to the selected ranker.

The binary cache stores the searchable inverted-index state. SQLite stores document metadata, manifest rows, and search history. On a binary cache miss or invalid cache, the CLI can rebuild from the supplied directory. The UI shares indexing and search types, though some advanced search features remain CLI/index-only.

Extraction security bounds in this version cap input files at 50 MiB, ZIP entries at 16 MiB, aggregate ZIP text at 64 MiB, and ZIP compression ratio at 200. PDF text extraction is intentionally limited; see `DECISIONS.md`.
