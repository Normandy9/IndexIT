# Add-on API Notes

## QueryParser

`QueryParser::parse()` returns a `QueryPlan`. It does not execute a search.
Callers should check `valid()` before reading the parsed fields.

The parser supports whitespace-separated terms, quoted phrases, a leading minus
for excluded terms, `type:` for an extension, and `path:` for a path substring.
Only one `type:` and one `path:` field are accepted. Unknown fields and malformed
quotes are reported through `errors`.

## MetadataFilterEngine

The engine filters metadata records already supplied by the caller. It does not
scan directories or inspect timestamps itself. The caller is responsible for
populating `FileMetadata`.

Size limits are inclusive. Modification time limits are inclusive epoch seconds.

## SnippetBuilder

The snippet builder returns a bounded preview around the earliest matching term.
It preserves the original bytes and does not inject HTML. `matchedTerms` reports
which requested terms occur in the generated preview. It is not a rich-text
highlighter and does not alter the original document.

## ResultExporter

CSV fields are quoted and embedded quotes doubled when required. JSON strings
escape quotes, backslashes, control characters, and common whitespace escapes.
The exporter returns strings; writing files is the caller's responsibility.

## StringUtilities

Case conversion is ASCII-only. UTF-8 bytes outside ASCII are preserved, not
Unicode-case-folded. This limitation is deliberate and should not be described
as full Unicode case-insensitive search.
