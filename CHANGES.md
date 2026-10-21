# Changes from SOURCE_REPO

- **`include/document.h`**: Reused and heavily expanded. Updated paths to `wchar_t` to support Unicode and long paths correctly.
- **`src/main.c`**: Rewritten completely to provide a real functional CLI, parse arguments properly using `CommandLineToArgvW` and wire up all modules.
- **`src/index/trie.c` / `trie.h`**: Wrote the implementation of the Trie from scratch.
- **`src/index/hashtable.c` / `hashtable.h`**: Wrote the implementation (FNV-1a) from scratch.
- **`src/index/inverted_index.c`**: Wrote the integration between hash table and trie.
- **`src/core/winfs.c`**: Newly written robust Windows API directory traversal.
- **`src/core/extractor.c`**: Newly written format handling logic (Tier-1 plaintext extraction).
- **`src/core/tokenizer.c`**: Newly written tokenizer logic with stopword removal.
- **`CMakeLists.txt`**: Rewritten to include all new source files and ensure `-municode` compatibility.
- **`third_party/`**: Vendored `sqlite3` and `miniz`.
