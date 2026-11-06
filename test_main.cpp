#include "StringUtilities.hpp"
#include "QueryParser.hpp"
#include "MetadataFilter.hpp"
#include "SnippetBuilder.hpp"
#include "ResultExporter.hpp"

#include <cassert>
#include <iostream>
#include <vector>

using namespace paras_phase3;

static void test_string_utilities() {
    assert(trim("  hello \t") == "hello");
    assert(asciiLower("MaCHine") == "machine");
    assert(normalizeExtension(" TXT ") == ".txt");
    assert(containsCaseInsensitive("D:/Notes/AI.txt", "notes"));
    assert(jsonEscape("a\nb") == "a\\nb");
    assert(csvEscape("a,b") == "\"a,b\"");
    assert(csvEscape("a\"b") == "\"a\"\"b\"");
}

static void test_query_parser() {
    QueryParser parser;
    QueryPlan plan = parser.parse("machine learning -draft type:TXT path:notes");
    assert(plan.valid());
    assert(plan.requiredTerms.size() == 2);
    assert(plan.requiredTerms[0] == "machine");
    assert(plan.excludedTerms.size() == 1);
    assert(plan.excludedTerms[0] == "draft");
    assert(plan.extension == ".txt");
    assert(plan.pathContains == "notes");

    QueryPlan phrase = parser.parse("\"machine learning\"");
    assert(phrase.valid());
    assert(phrase.phrases.size() == 1);

    assert(!parser.parse("machine \"unfinished").valid());
    assert(!parser.parse("type:").valid());
    assert(!parser.parse("-").valid());
    assert(!parser.parse("unknown:value").valid());
}

static void test_metadata_filter() {
    std::vector<FileMetadata> files;
    FileMetadata a; a.path="D:/Notes/one.txt"; a.extension=".txt";
    a.sizeBytes=100; a.modifiedEpoch=1000; files.push_back(a);
    FileMetadata b; b.path="D:/Images/two.png"; b.extension=".png";
    b.sizeBytes=500; b.modifiedEpoch=2000; files.push_back(b);

    MetadataFilter filter;
    filter.extension="TXT";
    filter.pathContains="notes";
    filter.minimumBytes=50; filter.useMinimum=true;
    filter.maximumBytes=200; filter.useMaximum=true;

    MetadataFilterEngine engine;
    std::vector<FileMetadata> found=engine.apply(files,filter);
    assert(found.size()==1);
    assert(found[0].path==a.path);
}

static void test_snippet_builder() {
    SnippetBuilder builder;
    std::string content="The first paragraph contains background information. "
                        "Machine learning helps computers find patterns in data. "
                        "The final paragraph contains a summary.";
    std::vector<std::string> terms;
    terms.push_back("learning");
    Snippet snippet=builder.build(content,terms,90);
    assert(!snippet.text.empty());
    assert(!snippet.matchedTerms.empty());
    assert(snippet.matchedTerms[0]=="learning");
}

static void test_exporters() {
    SearchResultRecord item;
    item.path="D:/Notes/a,b.txt";
    item.title="A \"quoted\" title";
    item.score=0.875;
    item.snippet="machine learning\npreview";

    std::vector<SearchResultRecord> records(1,item);
    ResultExporter exporter;
    const std::string csv=exporter.toCsv(records);
    const std::string json=exporter.toJson(records);
    assert(csv.find("\"D:/Notes/a,b.txt\"")!=std::string::npos);
    assert(json.find("\\\"quoted\\\"")!=std::string::npos);
    assert(json.find("\\n")!=std::string::npos);
}

int main() {
    test_string_utilities();
    test_query_parser();
    test_metadata_filter();
    test_snippet_builder();
    test_exporters();
    std::cout << "All Paras Phase III add-on tests passed.\n";
    return 0;
}
