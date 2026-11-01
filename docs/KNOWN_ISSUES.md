# Known issues

- Step 5 encrypted Office/ZIP entries are skipped by the ZIP reader when the entry is not stored/deflated or cannot be decoded. There is no per-file logging at the extractor interface.
- The PDF fallback is only suitable for simple literal strings. It does not correctly decode general PDF text encodings or object streams.
- Step 6 remains partial: CLI metadata filters, match-context snippets/highlighting, duplicate-content reporting, and UI filter controls are not implemented. Fuzzy matching uses full hash-term enumeration instead of trie pruning.
- Step 7 remains partial: Unicode token boundaries preserve non-ASCII UTF-8 bytes, but lowercase is ASCII-only. ANSI code-page decoding, UTF-16 BE edge cases, comprehensive long paths, locked/corrupt-file diagnostics, and guaranteed `WriteConsoleW` output are not covered by the tests.
- Step 8 benchmark harness, Windows Search/Select-String comparison, charts, benchmark report, and minimum 100-run statistical series are not implemented. No performance numbers are claimed.
- Step 9 expanded unit and integration coverage is incomplete. Existing CTest is one aggregate executable and does not exercise every requested corruption, invalidation, disk-guard, filter, extractor, and incremental-clean scenario.
- Step 10 named-pipe server and remote UI are not implemented.
