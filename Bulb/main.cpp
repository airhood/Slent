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
#include <format>

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
	int line;

	Token(TokenType type, string value, int line) {
		this->type = type;
		this->value = value;
		this->line = line;
	}
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
	"deep",
	"const"
};

enum class MessageType {
	ERROR,
	WARNING,
	MESSAGE
};

struct CompileMessage {
	MessageType type;
	string message;
	string file_name;
	int line_index;

	CompileMessage(MessageType type, string message, string file_name, int line_index) {
		this->type = type;
		this->message = message;
		this->file_name = file_name;
		this->line_index = line_index;
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
					throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, h + 1));
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
						tokens[h][k] = currentToken;
						currentToken = "";
						isLiteralOpen = false;
						continue;
					}
					currentToken += code_lines[h][i];
					continue;
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
			if (isLiteralOpen) {
				throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor literal. Quotation mark should be closed.", currentFileName, h + 1));
				isLiteralOpen = false;
			}
		}

		return tokens;
	}

	string preprocess(string code) {
		const int IF_TRUE = 1;
		const int IF_FALSE = 2;
		vector<string> code_lines = split(code, '\n');
		string** preprocessor_tokens = getPreprocessorTokens(code);

		string processed_code = "";

		vector<int> waitingTokenTree;

		vector<string> defined_keywords;
		for (int i = 0; i < code_lines.size(); i++) {
			if (code_lines[i][0] == '#') {
				vector<string> code_line_splits = split(code_lines[i], ' ');

				if (code_lines[i].size() == 0) {
					continue;
				}

				if (code_line_splits.size() == 0) {
					continue;
				}

				if (code_line_splits[0] == "#define") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					defined_keywords.push_back(preprocessor_tokens[i][1]);
					continue;
				}
				
				if (code_line_splits[0] == "#undef") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
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
				
				if (code_line_splits[0] == "#if") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
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
				
				if (code_line_splits[0] == "#elif") {
					if (waitingTokenTree.size() == 0) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "#elif can be used after #if", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
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
						waitingTokenTree.erase(waitingTokenTree.end() - 1);
						waitingTokenTree.push_back(IF_TRUE);
					}
					
					continue;
				}
				
				if (code_line_splits[0] == "#else") {
					if (waitingTokenTree.size() == 0) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "#else can be used after #if", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1] != "") || (preprocessor_tokens[i][2] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (waitingTokenTree.back() == IF_TRUE) {
						i = p_find_next(preprocessor_tokens, code_lines.size(), i + 1, vector<string> {"#endif"}) - 1;
						continue;
					}

					waitingTokenTree.erase(waitingTokenTree.end() - 1);
					waitingTokenTree.push_back(IF_TRUE);

					continue;
				}
				
				if (code_line_splits[0] == "#endif") {
					if (waitingTokenTree.size() == 0) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "#endif can be used after #if", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1] != "") || (preprocessor_tokens[i][2] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					waitingTokenTree.erase(waitingTokenTree.end() - 1);

					continue;
				}
				
				if (code_line_splits[0] == "#message") {
					if (code_line_splits[1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][2] != "") || (preprocessor_tokens[i][3] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1].front() != '\"') || (preprocessor_tokens[i][1].back() != '\"')) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter. Literal required.", currentFileName, i + 1));
						continue;
					}

					string message;
					preprocessor_tokens[i][1].erase(preprocessor_tokens[i][1].begin());
					preprocessor_tokens[i][1].erase(preprocessor_tokens[i][1].end() - 1);
					message = preprocessor_tokens[i][1];

					throwCompileMessage(CompileMessage(MessageType::MESSAGE, "[Preprocessor]" + message, currentFileName, i + 1));

					continue;
				}
				
				if (code_line_splits[0] == "#warning") {
					if (code_line_splits[1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][2] != "") || (preprocessor_tokens[i][3] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1].front() != '\"') || (preprocessor_tokens[i][1].back() != '\"')) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter. Literal required.", currentFileName, i + 1));
						continue;
					}

					preprocessor_tokens[i][1].erase(preprocessor_tokens[i][1].begin());
					preprocessor_tokens[i][1].erase(preprocessor_tokens[i][1].end() - 1);
					string message;
					message = preprocessor_tokens[i][1];

					throwCompileMessage(CompileMessage(MessageType::WARNING, "[Preprocessor]" + message, currentFileName, i + 1));

					continue;
				}
				
				if (code_line_splits[0] == "#error") {
					if (code_line_splits[1] == "") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Preprocessor parameter missing", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][2] != "") || (preprocessor_tokens[i][3] != "")) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter", currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1].front() != '\"') || (preprocessor_tokens[i][1].back() != '\"')) {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected preprocessor parameter. Literal required.", currentFileName, i + 1));
						continue;
					}

					preprocessor_tokens[i][1].erase(preprocessor_tokens[i][1].begin());
					preprocessor_tokens[i][1].erase(preprocessor_tokens[i][1].end() - 1);
					string message;
					message = preprocessor_tokens[i][1];

					throwCompileMessage(CompileMessage(MessageType::ERROR, "[Preprocessor]" + message, currentFileName, i + 1));

					continue;
				}

				if (code_line_splits[0] == "#pragma") {
					if (code_line_splits.size() > 1) {
						if (code_line_splits[1] == "warning") {
							if (code_line_splits.size() > 2) {
								if (code_line_splits[2] == "disable") {

								}
								else if (code_line_splits[2] == "enable") {

								}
							}
						}
					}

					continue;
				}

				throwCompileMessage(CompileMessage(MessageType::ERROR, string("Unsupported preprocessor command. ").append(code_line_splits[0]).append(" command doesn't exist"), currentFileName, i + 1));
				continue;
			}

			if (processed_code != "") {
				processed_code += "\n";
			}
			processed_code += code_lines[i];
			continue;
		}

		return processed_code;
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

		vector<string> code_split = split(code, '\n');
		for (int i = 0; i < code_split.size(); i++) {
			while (regex_search(code_split[i], match, tokenRegex)) {
				string matched = match.str();
				if (find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
					tokens.push_back(Token(TokenType::KEYWORD, matched, i));
				}
				else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
					tokens.push_back(Token(TokenType::IDENTIFIER, matched, i));
				}
				else if (regex_match(matched, regex("[0-9]+(\\.[0-9]+)?"))) {
					tokens.push_back(Token(TokenType::CONSTANT, matched, i));
				}
				else if (regex_match(matched, regex("\"[^\"]*\""))) {
					tokens.push_back(Token(TokenType::LITERAL, matched, i));
				}
				else if (regex_match(matched, regex(R"(\+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!)"))) {
					tokens.push_back(Token(TokenType::OPERATOR, matched, i));
				}
				else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|;|<|>|\.)"))) {
					tokens.push_back(Token(TokenType::SPECIAL_SYMBOL, matched, i));
				}
				else {
					throw invalid_argument("Invalid Syntax");
				}
				code = match.suffix().str();
			}
		}

		return tokens;
	}

	Constructor parser(vector<Token> tokens) {
		Constructor root = Constructor();
		root.setName("root");
		for (int i = 0; i < tokens.size(); i++) {
			Token token = tokens[i];
			if (token.value == "import") {
				Constructor import = Constructor();
				import.setName("import");
				import.addProperty("name", tokens[i + 1].value);
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
		if (tokens[cursor + 1].type != TokenType::IDENTIFIER) {
			throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected class name. Class name should not be Keywords, Operators, Special_Characters. Try using other name.", currentFileName, tokens[cursor + 1].line + 1));
			return make_tuple(class_define, cursor, false);
		}
		class_define.addProperty("name", tokens[cursor + 1].value);
		if (tokens[cursor + 2].value != "{") {
			throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected class declear syntax. Class body missing. Class class_name {\n  ...\n}\n is the correct syntax for class declearation.", currentFileName, tokens[cursor + 1].line + 1));
			return make_tuple(class_define, cursor + 1, false);
		}
		class_define = getClassMembers(tokens, Scope(cursor + 3, findBraceClose(tokens, cursor + 3, 1)));
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
		for (int i = scope.start; i <= scope.end; i++) {
			if (tokens[i].value == "var") {
				Constructor var_decl = Constructor();
				var_decl.setName("var_decl");
				classVariables.addProperty(var_decl);
			}
		}
	}

	Constructor getClassFunctions(vector<Token> tokens, Scope scope) {
		Constructor classFunctions = Constructor();
		classFunctions.setName("func");
		for (int i = scope.start; i <= scope.end; i++) {
			if (tokens[i].value == "func") {
				if (tokens[i + 1].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(MessageType::ERROR, string("Unexpected function name. Keywords cannot be used as function name. Use other name instead of \'").append(tokens.at(i + 3).value).append("\'."), currentFileName, tokens[i + 1].line));
					continue;
				}

				Constructor func_decl = Constructor();
				func_decl.setName("func_decl");
				func_decl.addProperty("name", tokens[i + 1].value);

				// check return type
				if (tokens[i + 2].value != "->") {
					if (tokens[i + 2].value != "(") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function declear syntax.\nfunc function_name(param a, param b, ... ) -> return_type {\n  ...\n}\n is the correct syntax for function declearation.", currentFileName, tokens[i + 2].line));
						continue;
					}
					func_decl.addProperty("return_exist", "0");
					i = i + 2;
				}
				else {
					func_decl.addProperty("return_exist", "1");
					func_decl.addProperty("return_type", tokens[i + 3].value);

					if (tokens[i + 4].value != "(") {
						throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function declear syntax.\nfunc function_name(param a, param b, ... ) -> return_type {\n  ...\n}\n is the correct syntax for function declearation.", currentFileName, tokens[i + 4].line));
						continue;
					}
					i = i + 4;
				}

				// TODO: get function parameters
				int j = i + 1;
				while (j < findBracketClose(tokens, i + 1, 1)) {

				}

				classFunctions.addProperty(func_decl);
			}
		}
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
			if (tokens[i].value == "{") {
				brackets++;
			}
			else if (tokens[i].value == "}") {
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
			if (tokens[i].value == "(") {
				brackets++;
			}
			else if (tokens[i].value == ")") {
				brackets--;
			}

			if (brackets == 0) {
				return i;
			}
		}
	}

	int findNextSemicolon(vector<Token> tokens, int cursor) {
		for (int i = cursor; i < tokens.size(); i++) {
			if (tokens[i].value == ";") {
				return i;
			}
		}
	}

	bool classExist(vector<Token> tokens, string className) {
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens[i].value == "class") {
				if (tokens[i + 1].value == className) {
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
			default:
				return;
		}
		string str = colorString(string("[").append(type).append("] ").append(compileMessage.message), color);
		cout << str << endl;
	}

	void optimize() {

	}

	vector<tuple<string, string>> code_files;

	string currentFileName;

	void compile_file(string file_name, string code) {
		currentFileName = file_name;

#ifdef _DEBUG
		try {
			string preprocessed_code = preprocess(code);
			cout << "preprocessed_code:" << endl << preprocessed_code << endl;
			regex commentRegex("//.*");
			code = regex_replace(preprocessed_code, commentRegex, "");

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

		currentFileName = "";
	}

public:
	void AddFile(string file_name, string code) {
		code_files.push_back(make_tuple(file_name, code));
	}

	void Compile() {
		for (int i = 0; i < code_files.size(); i++) {
			compile_file(get<0>(code_files[i]), get<1>(code_files[i]));
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
	string exampleCode = "#if LEL\n#endif\n#define TEST\n\n#if L_TEST\nimport Test.UnitTest.ITestable;\n#else\n#error \"test is required\"\n#endif\nimport System;\n\nclass Foo {\n    var public int moo;\n    var public int foo;\n    \n    construct(int _moo, int _foo, int _boo) {\n        moo = _moo;\n        foo = _foo;\n        var boo = _boo + 1;\n        foo += boo;\n    }\n    \n    func ConvertTo16() -> string {\n        return Boo + 1;\n    }\n}\n\nclass Main {\n    \n    @Entry\n    func main() {\n        Foo foo = new Foo(10, 7, 1);\n        foo.ConvertTo16();\n        \n        string str = \"+\";\n        \n        if (true) {\n            \n        }\n    }\n}";
	//string exampleCode = "#if LEL\n#endif\n#define TEST\n\n#if L_TEST\nimport Test.UnitTest.ITestable;\n#else\n#error \"test is required\"\n#endif\nimport System;\n\nclass Foo {\n\tvar public int moo;\n\tvar public int foo;\n\t\n\tconstruct(int _moo, int _foo, int _boo) {\n\t\tmoo = _moo;\n\t\tfoo = _foo;\n\t\tvar boo = _boo + 1;\n\t\tfoo += boo;\n\t}\n\t\n\tfunc ConvertTo16() -> string {\n\t\treturn Boo + 1;\n\t}\n}\n\nclass Main {\n\t\n\t@Entry\n\tfunc main() {\n\t\tFoo foo = new Foo(10, 7, 1);\n\t\tfoo.ConvertTo16();\n\t\t\n\t\tstring str = \"+\";\n\t\t\n\t\tif (true) {\n\t\t\t\n\t\t}\n\t}\n}";
	
	BulbCompiler* compiler = new BulbCompiler;
	compiler->AddFile("main.bulb", exampleCode);
	compiler->Compile();

	delete compiler;
#endif

	return 0;
}