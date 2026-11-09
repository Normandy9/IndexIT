# IndexIt

**Search your documents by their contents or find them by name.** IndexIt is a
Windows desktop application with a native C/C++ indexing and search engine. It
also includes a command-line interface for running the same core operations.

![IndexIt desktop application](assets/indexit-screenshot.jpg)

## Features

- Index a folder and update its index incrementally.
- Search indexed document contents, with live suggestions as you type.
- Find indexed documents by filename.
- Browse recent searches, view index statistics, and clean the cache.
- Use the desktop window or the command line.
- Keep each folder's index in its own cache, so results stay scoped to the
  folder you selected.

## Get started

### Build on Windows

Install CMake and a C/C++ compiler with Windows resource compiler support, then
run these commands from the repository folder in PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

With the MinGW Makefiles generator used for development, launch the app with:

```powershell
.\build\IndexItWindow.exe
```

With a multi-configuration generator such as Visual Studio, the executable is
usually under `build\Release\IndexItWindow.exe`.

### Use the desktop app

1. Choose a folder with **Browse...** or enter its path.
2. Click **Index** to scan the folder and build or update its index.
3. Choose **Search text** to search file contents, or **Find document** to search
   filenames.
4. Enter a query and click **Run**. While searching text, suggestions appear as
   you type; double-click one to complete the current word.

The app also has **Suggestions**, **History**, **Statistics**, and **Clean
cache** actions. Hover over an action for a short description. The command-line
engine runs in the background while the desktop app is open.

## Command line

The CLI executable is `indexit.exe`, built alongside the desktop app. Get the
full command list with:

```powershell
.\build\indexit.exe --help
```

Examples:

```powershell
# Index or update a folder
.\build\indexit.exe index "C:\Users\You\Documents"

# Search document contents
.\build\indexit.exe search "C:\Users\You\Documents" "project schedule"

# Find files by filename
.\build\indexit.exe find "C:\Users\You\Documents" "meeting notes"

# Limit a content search to PDFs
.\build\indexit.exe search "C:\Users\You\Documents" "invoice" --ext .pdf

# Show index statistics
.\build\indexit.exe stats "C:\Users\You\Documents"
```

Other commands include `suggest`, `history`, `ui`, and `clean`. Run `--help` for
all supported options, including search filters and custom cache locations.

## Supported file types

| Types | What IndexIt reads |
| --- | --- |
| `.txt`, `.csv`, `.log`, `.md`, `.json`, `.xml` | Plain text; UTF-8 and BOM-marked UTF-16 are supported. |
| `.c`, `.cpp`, `.h` | Source code as text. |
| `.html`, `.htm`, `.rtf` | Text with basic markup/formatting removal. |
| `.eml` | Raw email text, including headers. |
| `.docx`, `.pptx`, `.xlsx`, `.odt` | Text stored in the document's XML parts. Legacy `.doc`, `.ppt`, and `.xls` files are not supported. |
| `.zip` | Text from contained `.txt`, `.md`, `.csv`, and `.log` files. |
| `.pdf` | Best-effort extraction from some uncompressed or Flate-compressed text streams. |

PDF extraction is limited: encrypted files, scanned pages requiring OCR, and
some font encodings or compressed object formats may not yield searchable text.
IndexIt does not perform OCR.

## Tests

Build and run the CTest suite with:

```powershell
ctest --test-dir build --output-on-failure
```

## Add-ons

The optional C++ add-on library is kept separate from the application. It has
its own CMake project and tests; see [`addons/README.md`](addons/README.md).

## Project layout

```text
src/          C and C++ application, index, extractors, CLI, and Windows UI
include/      Public headers
addons/       Standalone optional C++ add-on library
tests/        Core test runner and document fixtures
third_party/  Miniz and SQLite source dependencies
assets/       App icon and README screenshot
docs/         Architecture, decisions, and project notes
```
