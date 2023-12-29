#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

using namespace std;

struct Token {
    string type;
    string value;
};

vector<string> keywords = {
    "import",
    "class",
    "public",
    "int",
    "string",
    "void",
    "return",
    "@Entry",
    "new"
};

vector<Token> lexer(string code) {
    vector<Token> tokens;

    regex tokenRegex(R"(\+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"|\(|\)|\{|\}|\[|\]|:|;|<|>)");
    // \+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"
    smatch match;
    while (regex_search(code, match, tokenRegex)) {
        cout << "wow:" << match.str() << endl;
        string matched = match.str();
        if (find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
            tokens.push_back({ "KEYWORD", matched});
        }
        else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
            tokens.push_back({ "IDENTIFIER", matched });
        }
        else if (regex_match(matched, regex("[0-9]+(\\.[0-9]+)?"))) {
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
        else {
            throw invalid_argument("Invalid Syntax");
        }
        code = match.suffix().str();
    }

    return tokens;
}

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual void print(int indent = 0) const = 0;
};

class ClassDeclarationNode : public ASTNode {
public:
    string className;
    vector<string> members;

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Class: " << className << endl;
        for (const auto& member : members) {
            cout << string(indent + 2, ' ') << "Member: " << member << endl;
        }
    }
};

class Parser {
private:
    vector<Token> tokens;
    size_t currentTokenIndex;

public:
    Parser(const vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

    void parse() {

    }
};

int main() {
    string exampleCode = R"(import System;

class Foo {
	
	public int moo; // auto size
	public int:4: foo; // 4 byte
	public int:2: boo; // 2 byte
    
    public Foo(int _moo, int:4: _foo, int:2: _boo) {
        moo = _moo;
        foo = _foo;
        boo = _boo;
    }
	
	public string ConvertTo16() {
		return Boo + 1;
	}
}

class Main {
	
	@Entry
	public void main() {
		Foo foo = new Foo(10, 7, 1);
	}
}
    )";

    regex commentRegex("//.*");
    exampleCode = regex_replace(exampleCode, commentRegex, "");

    try {
        vector<Token> tokens = lexer(exampleCode);

        // 토큰 출력
        for (const auto& token : tokens) {
            cout << "Type: " << token.type << ", Value: " << token.value << endl;
        }

        Parser parser(tokens);
        parser.parse();
    }
    catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}