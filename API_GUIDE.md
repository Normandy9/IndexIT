# Advanced Add-on API Guide

## DocumentStats
Analyzes an in-memory string. Character count is byte count, not Unicode code-point
count. Words are sequences separated by whitespace. Lines count newline-separated
segments; an empty input has zero lines.

## SearchAnalytics
Summarizes caller-supplied query events. A successful query is one with at least
one result. Non-positive durations are ignored in the duration total but each event
still contributes to the average denominator. Frequent queries are sorted by
descending frequency and then alphabetically for ties.

## ResultDeduplicator
Normalizes slash direction and ASCII letter case for comparison. It retains the
first occurrence and its score. It does not resolve `..`, symbolic links, network
share aliases, or filesystem case-sensitivity rules.

## RankingMetrics
Computes precision, recall and F1 using unique retrieved IDs and unique relevant
IDs. Duplicate retrieved IDs count once. If the denominator is zero, the related
metric is zero.

## PathUtilities
Performs lexical path string operations only. It does not query the filesystem,
resolve absolute paths, or canonicalize symlinks. A dotfile such as `.gitignore`
is treated as having no extension.
