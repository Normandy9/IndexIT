# Paras Phase III Add-on Modules (Non-invasive)

This package adds independent utilities for IndexIt without modifying the existing
IndexIt source files, CMake configuration, executable, database schema, cache format,
or search behaviour.

## Safety boundary

- Copy this folder into the repository as `paras_phase3_addons/`.
- Do not overwrite any existing file.
- The add-on has its own `CMakeLists.txt` and its own test executable.
- It is not linked to `indexit` and does not change the current application.
- Integration into the main application should happen only after separate review.

## Modules

1. `QueryParser` — validates and parses a small, documented query syntax.
2. `MetadataFilter` — filters caller-provided metadata records by extension,
   size range, and path substring.
3. `SnippetBuilder` — creates short text previews and marks query-term matches.
4. `ResultExporter` — exports result records as CSV or JSON with proper escaping.
5. `StringUtilities` — trimming, ASCII case folding, and safe extension normalization.

These modules operate on plain C++ data structures. They do not access the file
system, database, cache, or Windows APIs.

## Build separately

From this folder:

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

A C++11-compatible compiler is sufficient. The package deliberately avoids
`std::filesystem` so it can be built separately with older MinGW toolchains.

## Query syntax

- `machine learning` — two positive terms
- `"machine learning"` — one quoted phrase
- `-draft` — excluded term
- `type:txt` — extension filter
- `path:notes` — path substring filter

The parser is a utility only. The existing IndexIt CLI does not automatically
use this syntax until a developer explicitly integrates it.

## Integration policy

Do not change the existing `CMakeLists.txt`. Build this folder independently.
When ready, a developer can review the APIs and selectively connect them to
existing application code. The original executable remains unaffected.
