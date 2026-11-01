# IndexIt

IndexIt is a Windows-native local document content search prototype written in C and C++17. It walks a directory, extracts supported document text, builds an inverted index, and ranks AND queries with BM25.

## Features currently implemented

- Win32 directory walking and a C++ indexing/search path.
- FNV-1a hash index with resizing and sorted postings; trie prefix search and suggestions.
- BM25 ranking, multi-term AND queries, top-K results, and CLI/UI entry points.
- Binary `index.cache` persistence with checksum, atomic replacement, incremental metadata checks, free-space guard, manifest, and confirmed clean operation.
- SQLite metadata and search history (`indexit.db`).
- Text, DOCX, PPTX, XLSX, ODT, ZIP-contained text, RTF, EML, HTML, and limited PDF extraction.
- UTF-8 byte-preserving tokenization and UTF-16 BOM text decoding.

## Project layout

- `src/core/`: Win32 walker, tokenizer, and legacy C extraction interface.
- `src/index/`: C hash table, trie, and inverted-index primitives.
- `src/`: C++ extractors, index, ranker, CLI/UI, binary cache, and SQLite manager.
- `include/`: public interfaces and data structures.
- `third_party/sqlite3/`, `third_party/miniz/`: vendored SQLite and miniz.
- `tests/`: CTest test runner and generated format fixtures in `data/test_fixtures/`.
- `docs/`: baseline, decisions, architecture, class guide, completion report, and known issues.

## Prerequisites and build

Windows with MinGW GCC/G++ and CMake. The verified workspace toolchain is MinGW.org GCC 6.3.0 and the portable CMake 3.29.3 in `tools/cmake`.

```powershell
.\tools\cmake\bin\cmake.exe -S . -B build -G 'MinGW Makefiles'
.\tools\cmake\bin\cmake.exe --build build -j2
.\tools\cmake\bin\ctest.exe --test-dir build --output-on-failure
```

## CLI

```text
indexit --help
indexit index <dir> [--cache-dir <path>]
indexit search <dir> <query> [--cache-dir <path>]
indexit suggest <dir> <prefix> [--cache-dir <path>]
indexit stats <dir> [--cache-dir <path>]
indexit history [--cache-dir <path>]
indexit clean [--cache-dir <path>] [--yes]
indexit ui <dir>
```

The default cache directory is `%LOCALAPPDATA%\IndexIt`. `clean` lists manifest-tracked files and sizes before prompting; `--yes` skips the prompt. Search results require an index, which is loaded from the cache or built from the supplied directory.

## Known limitations

- PDF extraction is a fallback parser for simple literal strings in uncompressed and FlateDecode streams. It is not a general PDF parser and has no OCR, robust font maps, object-stream support, or encrypted-PDF support.
- Step 6 filters, snippets/highlighting, duplicate detection, and UI filter controls are incomplete. Fuzzy matching currently enumerates indexed terms rather than using trie pruning.
- Unicode support preserves UTF-8 multibyte sequences but does not implement full Unicode category classification or case folding. Long-path prefix handling and complete wide-console UI output need further work.
- The full benchmark suite, Windows Search comparison, charts/report, expanded fuzz and integration tests, and Step 10 server mode are not implemented. See `docs/COMPLETION_REPORT.md` and `docs/KNOWN_ISSUES.md`.
