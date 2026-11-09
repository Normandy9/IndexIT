#include "QueryValidator.hpp"
#include "FileTypeClassifier.hpp"
#include "SearchSuggestionEngine.hpp"
#include "DelimitedTextReader.hpp"
#include <cassert>
#include <iostream>
int main() {
    using namespace paras;
    assert(QueryValidator::validate("machine learning").valid);
    assert(!QueryValidator::validate("").valid);
    assert(!QueryValidator::validate("   ").valid);
    assert(!QueryValidator::validate("long query", 3).valid);
    assert(QueryValidator::trim("  hello  ") == "hello");
    assert(QueryValidator::containsOnlyWhitespace("\t \n"));
    assert(!QueryValidator::containsOnlyWhitespace("a"));

    assert(FileTypeClassifier::extensionOf("D:\\docs\\Report.PDF") == ".pdf");
    assert(FileTypeClassifier::extensionOf("folder.with.dot/name") == "");
    assert(FileTypeClassifier::classify("main.cpp").category == "code");
    assert(FileTypeClassifier::isTextLike("notes.txt"));
    assert(!FileTypeClassifier::isTextLike("photo.png"));
    assert(FileTypeClassifier::classify("unknown.zzz").category == "unknown");
    assert(!FileTypeClassifier::supportedExtensions().empty());

    SearchSuggestionEngine engine;
    assert(engine.empty());
    engine.add("Machine", 2);
    engine.add("machine", 3);
    engine.add("Management", 1);
    engine.add("map", 4);
    engine.add("", 5);
    engine.add("ignored", 0);
    std::vector<Suggestion> suggestions = engine.suggest("ma", 3);
    assert(suggestions.size() == 3);
    assert(suggestions[0].frequency == 5);
    assert(suggestions[1].term == "map");
    assert(engine.remove("MANAGEMENT"));
    assert(!engine.remove("not-present"));
    assert(engine.size() == 2);
    engine.clear();
    assert(engine.empty());

    DelimitedReadResult parsed =
        DelimitedTextReader::parse("name,city\nParas,\"New Delhi, India\"\n");
    assert(parsed.success);
    assert(parsed.rows.size() == 2);
    assert(parsed.rows[1][1] == "New Delhi, India");
    assert(!DelimitedTextReader::parse("a,\"unfinished").success);
    std::vector<std::string> fields;
    fields.push_back("a,b");
    fields.push_back("say \"hi\"");
    std::string encoded = DelimitedTextReader::joinRow(fields);
    DelimitedReadResult decoded = DelimitedTextReader::parse(encoded);
    assert(decoded.success);
    assert(decoded.rows[0] == fields);
    std::cout << "All add-on tests passed.\n";
    return 0;
}
