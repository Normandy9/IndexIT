# Architecture

```text
+-------------+
| CLI Driver  | (main.c)
+-------------+
       |
       v
+-------------+      +-------------------+
| File Walker | ---> | Format Extractors |
|   (winfs)   |      |  (txt/csv/etc.)   |
+-------------+      +-------------------+
                             |
                             v
                     +---------------+
                     |   Tokenizer   | (lowercasing, stopword removal)
                     +---------------+
                             |
                             v
                     +---------------+
                     | Inverted Index|
                     | (Hash / Trie) |
                     +---------------+
```

The system uses standard C structures to index files locally.
`winfs` safely avoids locked files. Tokenization strips standard stopwords.
The Inverted Index is powered by a high-speed FNV-1a Hash Table for exact lookups, and a Trie structure for prefix matching.
