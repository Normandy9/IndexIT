# Paras Phase III Advanced Add-ons

This is a second optional, isolated C++ add-on package. It does not modify the
original IndexIt source, root CMakeLists.txt, CLI, UI, database, cache or ranking.

Copy this directory into the repository as `paras_phase3_advanced/`.
Build it independently from this directory:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Components

- `DocumentStats`: computes word, line, character and average-word-length statistics.
- `SearchAnalytics`: summarizes query history and computes query frequency and success rate.
- `ResultDeduplicator`: removes duplicate result paths without changing the first result's order.
- `RankingMetrics`: calculates precision, recall and F1 from relevance judgments.
- `PathUtilities`: lexical path normalization and filename/extension helpers.

The package works only on values passed by the caller. It does not read files,
connect to SQLite, inspect the Windows filesystem, or change IndexIt's existing
ranking or search behaviour. Integration is optional and requires code review.
