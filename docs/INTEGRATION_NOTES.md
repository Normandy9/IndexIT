# Integration Notes

During the integration of eature/core-engine-and-cli into the shared team repository, several stub files originally committed by teammates were replaced with the functional C and C++ implementations built in my local workspace.

### Conflicts Resolved

1. **include/document.h**
   - *Team Version:* Defined char *path;
   - *My Version:* Replaced with wchar_t *path; to properly support long Windows Unicode paths via FindFirstFileW. The format enum was also expanded.
   - *Resolution:* Used my version since the team version was a stub.

2. **include/trie.h & include/hashtable.h**
   - *Team Version:* Declared basic _search() functions returning a single Posting*.
   - *My Version:* Included _search_count() functions to return the size of the array, essential for the C++ index layer iteration. Also wrapped in #ifdef __cplusplus extern "C".
   - *Resolution:* Overwrote with my version to support the OOP layer.

3. **include/token.h**
   - *Team Version:* Defined Token but lacked C++ extern bindings.
   - *My Version:* Includes extern "C" to prevent C++ name mangling during linkage with the C tokenizer.
   - *Resolution:* Kept my version.

4. **src/main.c vs src/main.cpp**
   - *Team Version:* A placeholder main.c returning 0.
   - *My Version:* Replaced with a C++ main.cpp providing the fully functional CLI (index, search, ui) and tying into the CppInvertedIndex.
   - *Resolution:* Deleted the team's main.c and committed my main.cpp.

5. **src/db.h & src/cache.h**
   - *Team Version:* Barebones stubs left in src/.
   - *Resolution:* Left the team's .h files untouched as per requirements (they belong to another member's future work), but integrated alongside my LruCache.hpp and SqliteConnection RAII wrappers in include/.
