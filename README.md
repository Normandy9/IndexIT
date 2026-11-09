# IndexIt

IndexIt is a Windows document indexing and search application. Its native C++
window is backed by the command-line engine. Application code is under `src/`,
public headers are under `include/`, and the main test runner is under `tests/`.

## Build

Configure and build with CMake on Windows:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The optional, standalone C++ add-on package is in
[`addons`](addons/README.md). It has its own
CMake project and is not linked into the IndexIt application.

To launch the Windows windowed interface after building, run
`build\IndexItWindow.exe`. It offers Index, Search text, Find document,
Suggestions, History, Statistics, and Clean cache actions with hover descriptions.
Search text shows live term suggestions while you type; double-click one to
complete the current word. The existing command-line app runs invisibly to
perform each action. By default, each selected folder gets its own index cache,
so searching one folder cannot show results from another folder's index.
