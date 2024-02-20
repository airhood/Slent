#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <tuple>
#include "Constructor.h"
#include "Execution.h"
#include <sstream>

using namespace std;

#define COMPILER_VERSION = "0.1"
#define MAX_COMPILE_SCRIPT_VERSION = "0.1"

#define printf //

#ifdef _DEBUG
#undef printf
#endif


/*
 *        ____        ____
 *       / __ )__  __/ / /_
 *      / __  / / / / / __ \
 *     / /_/ / /_/ / / /_/ /
 *    /_____/\__,_/_/_.___/
 *
 */


enum class TokenType {
	KEYWORD,
	IDENTIFIER,
	CONSTANT,
	LITERAL,
	OPERATOR,
	SPECIAL_SYMBOL
};

struct Token {
	TokenType type;
	string value;
};

vector<string> keywords = {
	"import",

	"public",
	"private",

	"var",
	"func",
	"class",
	"struct",
	"construct",

	"->",

	"int",
	"string",
	"bool",
	"float",
	"double",
	"char",
	"short",
	"null",
	"void",

	"return",

	"@Entry",

	"new",
	"const"
};

enum class MessageType {
	ERROR,
	WARNING,
	MESSAGE
};

struct CompileMessage {
	MessageType type;
	string name;
	string message;

	CompileMessage(MessageType type, string name, string message) {
		this->type = type;
		this->name = name;
		this->message = message;
	}
};

struct RuntimeError {
	string location;
	string name;
	string message;

	RuntimeError(string location, string name, string message) {
		this->location = location;
		this->name = name;
		this->message = message;
	}
};

struct Scope {
	int start;
	int end;

	Scope(int start, int end) {
		this->start = start;
		this->end = end;
	}
};

vector<string> split(string str, char Delimiter) {
	istringstream iss(str);             // istringstream에 str을 담는다.
	string buffer;                      // 구분자를 기준으로 절삭된 문자열이 담겨지는 버퍼

	vector<string> result;

	// istringstream은 istream을 상속받으므로 getline을 사용할 수 있다.
	while (getline(iss, buffer, Delimiter)) {
		result.push_back(buffer);               // 절삭된 문자열을 vector에 저장
	}
	return result;
}

const int BLACK = 30;
const int RED = 31;
const int GREEN = 32;
const int YELLOW = 33;
const int BLUE = 34;
const int MAGENTA = 35;
const int CYAN = 36;
const int WHITE = 37;

const int BLACK_LIGHT = 90;
const int RED_LIGHT = 91;
const int GREEN_LIGHT = 92;
const int YELLOW_LIGHT = 93;
const int BLUE_LIGHT = 94;
const int MAGENTA_LIGHT = 95;
const int CYAN_LIGHT = 96;
const int WHITE_LIGHT = 97;

class BulbCompiler {
private:
	string** getPreprocessorTokens(string code) {
		vector<string> code_lines = split(code, '\n');

		string** tokens = new string * [code_lines.size()];
		for (int i = 0; i < code_lines.size(); i++) {
			tokens[i] = new string[4];
			tokens[i][0] = "";
			tokens[i][1] = "";
			tokens[i][2] = "";
			tokens[i][3] = "";
		}
		string currentToken = "";
		bool isLiteralOpen = false;
		bool isFirstToken = true;
		int k = 0;
		for (int h = 0; h < code_lines.size(); h++) {
			if (code_lines[h][0] != '#') {
				continue;
			}
			isFirstToken = true;
			for (int i = 0; i < code_lines[h].size(); i++) {
				if (k > 3) {
					throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
					break;
				}
				if (isFirstToken) {
					if (code_lines[h][i] == ' ') {
						tokens[h][k] = currentToken;
						currentToken = "";
						isFirstToken = false;
						k++;
						continue;
					}
					else if (i == (code_lines[h].size() - 1)) {
						currentToken += code_lines[h][i];
						tokens[h][0] = currentToken;
						currentToken = "";
						isFirstToken = false;
						k++;
						continue;
					}

					currentToken += code_lines[h][i];
					continue;
				}

				if (isLiteralOpen) {
					if (code_lines[h][i] == '\"') {
						currentToken += "\"";
						tokens[h][k] == currentToken;
						currentToken = "";
						isLiteralOpen = false;
						k++;
						continue;
					}
					currentToken += code_lines[h][i];
				}
				else {
					if (code_lines[h][i] == '\"') {
						currentToken = "";
						currentToken += "\"";
						isLiteralOpen = true;
						continue;
					}
					if (code_lines[h][i] == ' ') {
						tokens[h][k] = currentToken;
						currentToken = "";
						k++;
						continue;
					}
					else if (i == (code_lines[h].size() - 1)) {
						currentToken += code_lines[h][i];
						tokens[h][k] = currentToken;
						currentToken = "";
						k++;
						continue;
					}
					currentToken += code_lines[h][i];
				}
			}
			k = 0;
		}

		return tokens;
	}

	string preprocess(string code) {
		const int IF_TRUE = 0;
		const int IF_FALSE = 1;
		vector<string> code_lines = split(code, '\n');
		string** preprocessor_tokens = getPreprocessorTokens(code);

		string processed_code = "";

		vector<int> waitingTokenTree;

		vector<string> defined_keywords;
		for (int i = 0; i < code_lines.size(); i++) {
			if (code_lines[i][0] = '#') {
				vector<string> code_line_splits = split(code_lines[i], ' ');
				if (code_line_splits.at(0) == "#define") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					defined_keywords.push_back(preprocessor_tokens[i][1]);
					continue;
				}
				
				if (code_line_splits.at(0) == "#undef") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					// remove from defined keywords
					for (int j = 0; j < defined_keywords.size(); j++) {
						if (defined_keywords[j] == preprocessor_tokens[i][1]) {
							defined_keywords.erase(defined_keywords.begin() + j);
						}
					}
					
					continue;
				}
				
				if (code_line_splits.at(0) == "#if") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					auto result = find(defined_keywords.begin(), defined_keywords.end(), preprocessor_tokens[i][1]);
					if (result == defined_keywords.end()) { // false
						waitingTokenTree.push_back(IF_FALSE);
						i = p_find_next(preprocessor_tokens, code_lines.size(), i + 1, vector<string> {"#elif", "#else", "#endif"}) - 1;
					}
					else { // true
						waitingTokenTree.push_back(IF_TRUE);
					}

					continue;
				}
				
				if (code_line_splits.at(0) == "#elif") {
					if (waitingTokenTree.size() == 0) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "#elif can be used after #if", ""));
						continue;
					}

					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (waitingTokenTree.back() == IF_TRUE) {
						i = p_find_next(preprocessor_tokens, code_lines.size(), i + 1, vector<string> {"#endif"}) - 1;
						continue;
					}

					auto result = find(defined_keywords.begin(), defined_keywords.end(), preprocessor_tokens[i][1]);
					if (result == defined_keywords.end()) { // false
						i = p_find_next(preprocessor_tokens, code_lines.size(), i + 1, vector<string> {"#elif", "#else", "#endif"});
					}
					else { // true
						waitingTokenTree.erase(waitingTokenTree.end());
						waitingTokenTree.push_back(IF_TRUE);
					}
					
					continue;
				}
				
				if (code_line_splits.at(0) == "#else") {
					if (waitingTokenTree.size() == 0) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "#else can be used after #if", ""));
						continue;
					}

					if ((preprocessor_tokens[i][1] != "") || (preprocessor_tokens[i][2] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (waitingTokenTree.back() == IF_TRUE) {
						i = p_find_next(preprocessor_tokens, code_lines.size(), i + 1, vector<string> {"#endif"}) - 1;
						continue;
					}

					waitingTokenTree.erase(waitingTokenTree.end());
					waitingTokenTree.push_back(IF_TRUE);

					continue;
				}
				
				if (code_line_splits.at(0) == "#endif") {
					if (waitingTokenTree.size() == 0) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "#endif can be used after #if", ""));
						continue;
					}

					if ((preprocessor_tokens[i][1] != "") || (preprocessor_tokens[i][2] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					waitingTokenTree.erase(waitingTokenTree.end());

					continue;
				}
				
				if (code_line_splits.at(0) == "#message") {
					if ((code_line_splits.at(1) == "") || (code_line_splits.at(2) == "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					string name, message;
					name = code_line_splits.at(1);
					message = code_line_splits.at(2);

					throwCompileMessage(CompileMessage(MessageType::MESSAGE, "[Preprocessor]" + name, message));

					continue;
				}
				
				if (code_line_splits.at(0) == "#warning") {
					if ((code_line_splits.at(1) == "") || (code_line_splits.at(2) == "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					string name, message;
					name = code_line_splits.at(1);
					message = code_line_splits.at(2);

					throwCompileMessage(CompileMessage(MessageType::WARNING, "[Preprocessor]" + name, message));

					continue;
				}
				
				if (code_line_splits.at(0) == "#error") {
					if ((code_line_splits.at(1) == "") || (code_line_splits.at(2) == "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", ""));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", ""));
						continue;
					}

					string name, message;
					name = code_line_splits.at(1);
					message = code_line_splits.at(2);

					throwCompileMessage(CompileMessage(MessageType::ERROR, "[Preprocessor]" + name, message));

					continue;
				}

				// not ready
				if (code_line_splits.at(0) == "#pragma") {
					if (code_line_splits.size() > 1) {
						if (code_line_splits.at(1) == "warning") {
							if (code_line_splits.size() > 2) {
								if (code_line_splits.at(2) == "disable") {

								}
								else if (code_line_splits.at(2) == "enable") {

								}
							}
						}
					}

					continue;
				}
			}
		}

		return code;
	}

	int p_find_next(string** preprocessor_tokens, int lines, int cursor, vector<string> target) {
		for (int i = cursor; i < lines; i++) {
			auto result = find(target.begin(), target.end(), preprocessor_tokens[i][0]);
			if (result != target.end()) {
				return i;
			}
		}
	}

	int p_find_next_ELSE(string** preprocessor_tokens, int lines, int cursor) {
		for (int i = cursor; i < lines; i++) {
			if (preprocessor_tokens[i][0] == "#ELSE") {
				return i;
			}
		}
	}

	vector<Token> lexer(string code) {
		vector<Token> tokens;

		regex tokenRegex(R"(->|\+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"|\(|\)|\{|\}|\[|\]|:|;|<|>|\.)");
		// \+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"
		smatch match;
		while (regex_search(code, match, tokenRegex)) {
			string matched = match.str();
			if (find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
				tokens.push_back({ TokenType::KEYWORD, matched });
			}
			else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
				tokens.push_back({ TokenType::IDENTIFIER, matched });
			}
			else if (regex_match(matched, regex("[0-9]+(\\.[0-9]+)?"))) {
				tokens.push_back({ TokenType::CONSTANT, matched });
			}
			else if (regex_match(matched, regex("\"[^\"]*\""))) {
				tokens.push_back({ TokenType::LITERAL, matched });
			}
			else if (regex_match(matched, regex(R"(\+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!)"))) {
				tokens.push_back({ TokenType::OPERATOR, matched });
			}
			else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|;|<|>|\.)"))) {
				tokens.push_back({ TokenType::SPECIAL_SYMBOL, matched });
			}
			else {
				throw invalid_argument("Invalid Syntax");
			}
			code = match.suffix().str();
		}

		return tokens;
	}

	Constructor parser(vector<Token> tokens) {
		Constructor root = Constructor();
		root.setName("root");
		for (int i = 0; i < tokens.size(); i++) {
			Token token = tokens.at(i);
			if (token.value == "import") {
				Constructor import = Constructor();
				import.setName("import");
				import.addProperty("name", tokens.at(i + 1).value);
				root.addProperty(import);
			}
			else if (token.value == "class") {
				tuple<Constructor, int, bool> getClass_result = getClass(tokens, i);
				i = get<1>(getClass_result);
				if (!get<2>(getClass_result)) {
					continue;
				}
				Constructor class_define = get<0>(getClass_result);
				root.addProperty(class_define);
			}
		}

		return root;
	}

	tuple<Constructor, int, bool> getClass(vector<Token> tokens, int cursor) {
		Constructor class_define = Constructor();
		class_define.setName("class");
		if (tokens.at(cursor + 1).type != TokenType::IDENTIFIER) {
			throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected class name", "Class name should not be Keywords, Operators, Special_Characters. Try using other name."));
			return make_tuple(class_define, cursor, false);
		}
		class_define.addProperty("name", tokens.at(cursor + 1).value);
		if (tokens.at(cursor + 2).value != "{") {
			throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected class declear syntax", "class class_name {\n  ...\n}\n is the correct syntax for class declearation."));
			return make_tuple(class_define, cursor + 1, false);
		}
	}

	Constructor getClassMembers(vector<Token> tokens, Scope scope) {
		Constructor classMembers = Constructor();
		classMembers.setName("members");
		Constructor classVariables = getClassVariables(tokens, scope);
		classMembers.addProperty(classVariables);
		Constructor classFunctions = getClassFunctions(tokens, scope);
		classMembers.addProperty(classFunctions);
	}

	Constructor getClassVariables(vector<Token> tokens, Scope scope) {
		Constructor classVariables = Constructor();
		classVariables.setName("variables");
	}

	Constructor getClassFunctions(vector<Token> tokens, Scope scope) {
		Constructor classFunctions = Constructor();
		classFunctions.setName("functions");
	}

	string colorString(string str, int color) {
		return string("\033[0;").append(to_string(color)).append("m").append(str).append("\033[0m");
	}

	// check if the type is available to use for variable type or function return type
	bool checkType(Token token) {
		if (token.type != TokenType::IDENTIFIER) {
			cout << token.value << endl;
			auto v = token.value;
			if ((v == "int") || (v == "string") || (v == "bool") || (v == "float") || (v == "double") || (v == "char") || (v == "short")) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return true;
		}
	}

	int findBraceClose(vector<Token> tokens, int cursor, int current_brace) {
		int brackets = current_brace;
		for (int i = cursor; i < tokens.size(); i++) {
			if (tokens.at(i).value == "{") {
				brackets++;
			}
			else if (tokens.at(i).value == "}") {
				brackets--;
			}

			if (brackets == 0) {
				return i;
			}
		}
	}

	int findBracketClose(vector<Token> tokens, int cursor, int current_bracket) {
		int brackets = current_bracket;
		for (int i = cursor; i < tokens.size(); i++) {
			if (tokens.at(i).value == "(") {
				brackets++;
			}
			else if (tokens.at(i).value == ")") {
				brackets--;
			}

			if (brackets == 0) {
				return i;
			}
		}
	}

	int findNextSemicolon(vector<Token> tokens, int cursor) {
		for (int i = cursor; i < tokens.size(); i++) {
			if (tokens.at(i).value == ";") {
				return i;
			}
		}
	}

	bool classExist(vector<Token> tokens, string className) {
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens.at(i).value == "class") {
				if (tokens.at(i + 1).value == className) {
					return true;
				}
			}
		}
		return false;
	}

	void throwCompileMessage(CompileMessage compileMessage) {
		string type;
		int color;
		switch (compileMessage.type) {
			case MessageType::ERROR:
				type = "Error";
				color = RED;
				break;
			case MessageType::WARNING:
				type = "Warning";
				color = YELLOW;
				break;
			case MessageType::MESSAGE:
				type = "Message";
				color = CYAN;
				break;
		}
		if (compileMessage.message == "") {
			string str = colorString(string("[").append(type).append("] ").append(compileMessage.name), color);
			cout << str << endl;
			return;
		}
		string str = colorString(string("[").append(type).append("] ").append(compileMessage.name).append(": ").append(compileMessage.message), color);
		cout << str << endl;
	}

	void parser(string bytecode) {

	}

	void optimize() {

	}

#define PARSER
#undef PARSER
public:
	void Compile(string code) {

		throwCompileMessage(CompileMessage(MessageType::MESSAGE, "Message Test", "Testing compile message"));
		throwCompileMessage(CompileMessage(MessageType::WARNING, "Warning Test", "Testing compile warning"));
		throwCompileMessage(CompileMessage(MessageType::ERROR, "Error Test", "Testing compile error"));

		return;

		string preprocessed_code = preprocess(code);
		regex commentRegex("//.*");
		code = regex_replace(preprocessed_code, commentRegex, "");

#ifdef _DEBUG
		try {
			vector<Token> tokens = lexer(code);

			// 토큰 출력
			for (const auto& token : tokens) {
				string tokenTypeStr;
				switch (token.type) {
					case TokenType::KEYWORD:
						tokenTypeStr = "KEYWORD";
						break;
					case TokenType::IDENTIFIER:
						tokenTypeStr = "IDENTIFIER";
						break;
					case TokenType::CONSTANT:
						tokenTypeStr = "CONSTANT";
						break;
					case TokenType::LITERAL:
						tokenTypeStr = "LITERAL";
						break;
					case TokenType::OPERATOR:
						tokenTypeStr = "OPERATOR";
					case TokenType::SPECIAL_SYMBOL:
						tokenTypeStr = "SPECIAL_SYMBOL";
						break;
				}
				cout << "Type: " << tokenTypeStr << ", Value: " << token.value << endl;
			}
#endif

#ifdef PARSER
			Constructor root = parser(tokens); // AST
#ifdef _DEBUG
			cout << "bytecode:" << endl << root.toString() << endl;
#endif
#endif
		}
		catch (const invalid_argument& e) {
			cerr << "Compiler internal error: " << e.what() << endl;
		}
	}
};

class BulbVirtualMachine {
private:

public:
	void Run(string code, int stackSize) {

	}
};

int main() {
	//    string preprocessor = R"(#if LEL
	//#endif
	//#define TEST
	//
	//#if TEST
	//import Test.UnitTest.ITestable;
	//#else
	//#error "test is required"
	//#endif)";
	//    
	//    string** preprocessorTokens = getPreprocessorTokens(preprocessor);

#ifdef _DEBUG
	string exampleCode = R"(import System;

class Foo {
	
	var public int moo;
	var public int foo;
    
    construct(int _moo, int _foo, int _boo) {
        moo = _moo;
        foo = _foo;
        var boo = _boo + 1;
        foo += boo;
    }
	
	func ConvertTo16() -> string {
		return Boo + 1;
	}
}

class Main {
	
    @Entry
	func main() {
		Foo foo = new Foo(10, 7, 1);
        foo.ConvertTo16();
        
        string str = "+";
        
        if (true) {
            
        }
	}
}
    )";

	BulbCompiler* compiler = new BulbCompiler;
	compiler->Compile(exampleCode);

	delete compiler;
#endif

	return 0;
}