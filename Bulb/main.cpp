#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <map>

using namespace std;

struct Token {
    string type;
    string value;
};

map<string, string> keywords = {
    {"import", "KEYWORD"},
    {"class", "KEYWORD"},
    {"public", "KEYWORD"},
    {"int", "KEYWORD"},
    {"string", "KEYWORD"},
    {"void", "KEYWORD"},
    {"return", "KEYWORD"},
    {"@Entry", "KEYWORD"}
};

vector<Token> lexer(string code) {
    vector<Token> tokens;

    regex tokenRegex(R"(\+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"|\(|\)|\{|\}|\[|\]|:|;|<|>)");
    // \+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"
    smatch match;
    while (regex_search(code, match, tokenRegex)) {
        string matched = match.str();
        if (keywords.find(matched) != keywords.end()) {
            tokens.push_back({ keywords[matched], matched });
        }
        else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
            tokens.push_back({ "IDENTIFIER", matched });
        }
        else if (regex_match(matched, regex("\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b"))) {
            tokens.push_back({ "CONSTANT", matched });
        }
        else if (regex_match(matched, regex("\"[^\"]*\""))) {
            tokens.push_back({ "LITERAL", matched });
        }
        else if (regex_match(matched, regex(R"(\+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!)"))) {
            tokens.push_back({ "OPERATOR", matched });
        }
        else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|:|;|<|>)"))) {
            tokens.push_back({ "SPECIAL_SYMBOL", matched });
        }
        code = match.suffix().str();
    }

    return tokens;
}

int main() {
    string exampleCode = R"(
import System;

class Foo {
	
	public int Moo; // auto size
	public int:4: Foo; // 64 byte
	public int:32: Boo;
	
	public string ConvertTo16() {
		return Boo + 1;
	}
}

class Main {
	
	@Entry
	public void main() {
		
	}
}
)";

    regex commentRegex("//.*");
    exampleCode = regex_replace(exampleCode, commentRegex, "");

    try {
        vector<Token> tokens = lexer(exampleCode);

        for (const auto& token : tokens) {
            cout << "Type: " << token.type << ", Value: " << token.value << endl;
        }
    }
    catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}