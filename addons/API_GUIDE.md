# API Guide

## QueryValidator
`validate(query, maximumLength)` returns a ValidationResult. The result has a
boolean named `valid` and a list of issue codes and messages. `trim` removes
whitespace at the beginning and end of a string. This module checks input only;
it does not execute or rewrite the user's search.

## FileTypeClassifier
`extensionOf(path)` extracts the suffix after the last path separator and converts
it to lowercase. `classify(path)` returns the extension, broad category, description,
and a text-like flag. Classification is based on the extension, not file contents.
A `.pdf` result does not mean PDF text extraction is implemented here.

## SearchSuggestionEngine
`add(term, frequency)` inserts a term or increases its frequency. Matching is
case-insensitive for the characters handled by the C locale. `suggest(prefix, limit)`
returns matching terms ordered by frequency and then alphabetically. Data stays
in memory and is not written to the IndexIt database.

## DelimitedTextReader
`parse(content, delimiter)` reads rows and supports quoted delimiters, doubled quote
characters and CR/LF line endings. An unclosed quote produces an error result.
`escapeField` quotes fields when needed. `joinRow` serializes a vector of fields.

## Scope
All modules use standard C++11. They do not access the file system, SQLite, or
the main IndexIt application. Review encoding, large-input limits and error
handling before production integration.
