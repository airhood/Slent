#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <tuple>
#include "Constructor.h"
#include <sstream>
#include <functional>

#include "Slent.h"

#include "CompileMessage.h"

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

namespace Slent {

	string colorString(string str, int color) {
		return string("\033[0;").append(to_string(color)).append("m").append(str).append("\033[0m");
	}

	vector<string> split(string str, char Delimiter) {
		istringstream iss(str);
		string buffer;

		vector<string> result;

		while (getline(iss, buffer, Delimiter)) {
			result.push_back(buffer);
		}
		return result;
	}
	
	string** SlentCompiler::getPreprocessorTokens(string code) {
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
					throwCompileMessage(CompileMessage(SL0001, currentFileName, h + 1));
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
				throwCompileMessage(CompileMessage(SL0002, currentFileName, h + 1));
				isLiteralOpen = false;
			}
		}

		return tokens;
	}

	string SlentCompiler::preprocess(string code) {
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
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					defined_keywords.push_back(preprocessor_tokens[i][1]);
					continue;
				}

				if (code_line_splits[0] == "#undef") {
					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
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
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
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
						throwCompileMessage(CompileMessage(SL0004, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][1] == "") {
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}
					if (preprocessor_tokens[i][2] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
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
						throwCompileMessage(CompileMessage(SL0004, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1] != "") || (preprocessor_tokens[i][2] != "")) {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
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
						throwCompileMessage(CompileMessage(SL0004, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1] != "") || (preprocessor_tokens[i][2] != "")) {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if (preprocessor_tokens[i][3] != "") {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					waitingTokenTree.erase(waitingTokenTree.end() - 1);

					continue;
				}

				if (code_line_splits[0] == "#message") {
					if (code_line_splits[1] == "") {
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][2] != "") || (preprocessor_tokens[i][3] != "")) {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1].front() != '>') || (preprocessor_tokens[i][1].back() != '<')) {
						throwCompileMessage(CompileMessage(SL0005, currentFileName, i + 1));
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
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][2] != "") || (preprocessor_tokens[i][3] != "")) {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1].front() != '>') || (preprocessor_tokens[i][1].back() != '<')) {
						throwCompileMessage(CompileMessage(SL0005, currentFileName, i + 1));
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
						throwCompileMessage(CompileMessage(SL0003, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][2] != "") || (preprocessor_tokens[i][3] != "")) {
						throwCompileMessage(CompileMessage(SL0001, currentFileName, i + 1));
						continue;
					}

					if ((preprocessor_tokens[i][1].front() != '>') || (preprocessor_tokens[i][1].back() != '<')) {
						throwCompileMessage(CompileMessage(SL0005, currentFileName, i + 1));
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

				throwCompileMessage(CompileMessage(SL0006(code_line_splits[0]), currentFileName, i + 1));
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

	int SlentCompiler::p_find_next(string** preprocessor_tokens, int lines, int cursor, vector<string> target) {
		for (int i = cursor; i < lines; i++) {
			auto result = find(target.begin(), target.end(), preprocessor_tokens[i][0]);
			if (result != target.end()) {
				return i;
			}
		}
		return -1;
	}

	int SlentCompiler::t_find_next(vector<Token> tokens, int cursor, vector<string> target) {
		for (int i = cursor; i < tokens.size(); i++) {
			auto result = find(target.begin(), target.end(), tokens[i].value);
			if (result != target.end()) {
				return i;
			}
		}
		return -1;
	}

	vector<Token> SlentCompiler::lexer(string code) {
		vector<Token> tokens;

		regex tokenRegex(R"(->|==|=|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|\+|\-|\*|\/|<|>|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"|\(|\)|\{|\}|\[|\]|:|;|<|>|\.|,)");
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
				else if (regex_match(matched, regex(R"(==|=|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|\+|\-|\*|\/|<|>|\|\||&&|!)"))) {
					tokens.push_back(Token(TokenType::OPERATOR, matched, i));
				}
				else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|;|<|>|\.|\,)"))) {
					tokens.push_back(Token(TokenType::SPECIAL_SYMBOL, matched, i));
				}
				else {
					throwCompileMessage(CompileMessage(SL0007(matched), currentFileName, i));
				}
				code_split[i] = match.suffix().str();
			}
		}

		return tokens;
	}

	Constructor SlentCompiler::parser(vector<Token> tokens) {
		Constructor root = Constructor();
		root.setName("root");
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens[i].value == "import") {
				Constructor import = Constructor();
				import.setName("import");
				// TODO: Parse import
				root.addProperty(import);
			}
			else if (tokens[i].value == "class") {
				cout << "class" << tokens[i].line << endl;
				tuple<Constructor, int, bool> getClass_result = getClass(tokens, i);
				i = get<int>(getClass_result);
				if (!get<bool>(getClass_result)) {
					continue;
				}
				Constructor class_define = get<Constructor>(getClass_result);
				root.addProperty(class_define);
			}
		}

		return root;
	}

	tuple<Constructor, int, bool> SlentCompiler::getClass(vector<Token> tokens, int cursor) {
		Constructor class_define = Constructor();
		class_define.setName("class");
		if (tokens[cursor + 1].type != TokenType::IDENTIFIER) {
			throwCompileMessage(CompileMessage(SL0008, currentFileName, tokens[cursor + 1].line));
			return make_tuple(class_define, cursor, false);
		}
		class_define.addProperty("name", tokens[cursor + 1].value);
		if (tokens[cursor + 2].value != "{") {
			throwCompileMessage(CompileMessage(SL0009, currentFileName, tokens[cursor + 1].line));
			return make_tuple(class_define, cursor + 1, false);
		}
		Constructor members = getClassMembers(tokens, Scope(cursor + 3, findBraceClose(tokens, cursor + 3, 1) - 1));
		class_define.addProperty(members);

		return make_tuple(class_define, findBraceClose(tokens, cursor + 3, 1), true);
	}

	Constructor SlentCompiler::getClassMembers(vector<Token> tokens, Scope scope) {
		Constructor classMembers = Constructor();
		classMembers.setName("members");
		vector<Constructor> class_constructor = getClassConstructors(tokens, scope);
		for (int i = 0; i < class_constructor.size(); i++) {
			classMembers.addProperty(class_constructor[i]);
		}
		vector<Constructor> class_variables = getClassVariables(tokens, scope);
		for (int i = 0; i < class_variables.size(); i++) {
			classMembers.addProperty(class_variables[i]);
		}
		vector<Constructor> class_functions = getClassFunctions(tokens, scope);
		for (int i = 0; i < class_functions.size(); i++) {
			classMembers.addProperty(class_functions[i]);
		}

		return classMembers;
	}

	vector<Constructor >SlentCompiler::getClassConstructors(vector<Token> tokens, Scope scope) {
		vector<Constructor> class_constructors;
		for (int i = scope.start; i <= scope.end; i++) {
			if (tokens[i].value == "construct") {
				Constructor constructor = Constructor();
				constructor.setName("constructor");

				if (tokens[i + 1].value != "(") {
					throwCompileMessage(CompileMessage(SL0010, currentFileName, tokens[i + 4].line));
					continue;
				}

				Constructor parameters = Constructor();
				parameters.setName("parameters");
				int k = 0;
				for (int j = i + 2; j < findBracketClose(tokens, i + 2, 1); j++) {
					if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
						throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[j].line));
						i = t_find_next(tokens, j, vector<string> {","});
						continue;
					}
					if (tokens[j + 1].type != TokenType::IDENTIFIER) {
						if ((j + 1) >= findBracketClose(tokens, i + 2, 1)) {
							break;
						}
						throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[j + 1].line));
						i = t_find_next(tokens, j + 1, vector<string> {","});
						continue;
					}
					if (tokens[j + 2].value != ",") {
						if ((j + 2) >= findBracketClose(tokens, i + 2, 1)) {
							break;
						}
						throwCompileMessage(CompileMessage(SL0013, currentFileName, tokens[j + 1].line));
						i = t_find_next(tokens, j + 2, vector<string> {","});
						continue;
					}
					string type = tokens[j].value;
					string name = tokens[j + 1].value;
					Constructor param = Constructor();
					param.setName(string("param").append(to_string(k)));
					param.addProperty("type", type);
					param.addProperty("name", name);
					parameters.addProperty(param);
					k++;
					j = j + 2;
				}
				constructor.addProperty(parameters);

				i = t_find_next(tokens, i + 2, vector<string> {")"});

				if (tokens[i + 1].value != "{") {
					throwCompileMessage(CompileMessage(SL0021, currentFileName, tokens[i + 1].line));
					continue;
				}

				Constructor constructor_body = getFunctionBody(tokens, Scope(i + 2, findBraceClose(tokens, i + 2, 1) - 1));
				constructor.addProperty(constructor_body);

				class_constructors.push_back(constructor);
			}
		}

		return class_constructors;
	}
	
	vector<Constructor> SlentCompiler::getClassVariables(vector<Token> tokens, Scope scope) {
		vector<Constructor> class_variables;
		for (int i = scope.start; i <= scope.end; i++) {
			if ((tokens[i].value == "construct") || (tokens[i].value == "func")) {
				i = findBraceClose(tokens, t_find_next(tokens, i, vector<string> {"{"}), 0);
				continue;
			}

			if (tokens[i].value == "var") {
				Constructor variable = Constructor();
				variable.setName("variable_c");

				// get variable access_modifier
				if ((tokens[i + 1].value == "public") || (tokens[i + 1].value == "private")) {
					// var [access_modifier] [type] [var_name]
					variable.addProperty("access_modifier", tokens[i + 1].value);
					i = i + 2;
				}
				else {
					// var [type] [var_name];
					i = i + 1;
				}

				// get variable type
				if ((tokens[i].type != TokenType::KEYWORD) && (tokens[i].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i].line));
					continue;
				}
				variable.addProperty("type", tokens[i].value);

				// get variable name
				if (tokens[i + 1].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[i + 1].line));
					continue;
				}
				variable.addProperty("name", tokens[i + 1].value);

				if (tokens[i + 2].value != ";") {
					throwCompileMessage(CompileMessage(SL0017, currentFileName, tokens[i + 1].line));
					continue;
				}

				class_variables.push_back(variable);

				i = findNextSemicolon(tokens, i + 2);
			}
		}

		return class_variables;
	}

	vector<Constructor> SlentCompiler::getClassFunctions(vector<Token> tokens, Scope scope) {
		vector<Constructor> class_functions;
		for (int i = scope.start; i <= scope.end; i++) {
			if (tokens[i].value == "func") {
				if ((i + 1) > scope.end) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[i].line));
					continue;
				}

				if (tokens[i + 1].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0019, currentFileName, tokens[i + 1].line));
					continue;
				}

				Constructor function = Constructor();
				function.setName("function");
				function.addProperty("name", tokens[i + 1].value);

				if ((i + 2) > scope.end) {
					throwCompileMessage(CompileMessage(SL0010, currentFileName, tokens[i + 1].line));
					continue;
				}

				if (tokens[i + 2].value != "(") {
					throwCompileMessage(CompileMessage(SL0020, currentFileName, tokens[i + 2].line));
					continue;
				}
				i = i + 2;

				// get function parameters
				Constructor parameters = Constructor();
				parameters.setName("parameters");

				int k = 0;

				if (tokens[i + 1].value == ")") {
					function.addProperty("parameters", "");
					goto get_parameter_finished;
				}

				for (int j = i + 1; j < findBracketClose(tokens, i + 1, 1); j++) {
					if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
						throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[j].line));
						i = t_find_next(tokens, j, vector<string> {","});
						continue;
					}

					if ((j + 1) >= findBracketClose(tokens, i + 1, 1)) {
						break;
					}

					if (tokens[j + 1].type != TokenType::IDENTIFIER) {
						throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[j + 1].line));
						i = t_find_next(tokens, j + 1, vector<string> {","});
						continue;
					}

					if ((j + 2) >= findBracketClose(tokens, i + 1, 1)) {
						break;
					}

					if (tokens[j + 2].value != ",") {
						throwCompileMessage(CompileMessage(SL0013, currentFileName, tokens[j + 1].line));
						i = t_find_next(tokens, j + 2, vector<string> {","});
						continue;
					}

					string type = tokens[j].value;
					string name = tokens[j + 1].value;
					Constructor param = Constructor();
					param.setName(string("param").append(to_string(k)));
					param.addProperty("type", type);
					param.addProperty("name", name);
					parameters.addProperty(param);
					k++;
					j = j + 2;
				}

				function.addProperty(parameters);
			get_parameter_finished:

				i = t_find_next(tokens, i + 1, vector<string> {")"});

				if ((i + 2) > scope.end) {
					throwCompileMessage(CompileMessage(SL0021, currentFileName, tokens[i].line));
					continue;
				}

				if (tokens[i + 1].value != "{") {
					if (tokens[i + 1].value == "->") {
						if ((i + 3) > scope.end) {
							throwCompileMessage(CompileMessage(SL0020, currentFileName, tokens[i + 2].line));
							continue;
						}

						if ((tokens[i + 2].type != TokenType::KEYWORD) && (tokens[i + 2].type != TokenType::IDENTIFIER)) {
							throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i + 3].line));
							continue;
						}
						function.addProperty("return_type", tokens[i + 2].value);
						if (tokens[i + 3].value != "{") {
							throwCompileMessage(CompileMessage(SL0021, currentFileName, tokens[i + 3].line));
						}
						i = i + 3;
						goto return_type_done;
					}
					throwCompileMessage(CompileMessage(SL0021, currentFileName, tokens[i].line));
					continue;
				}
				// tokens[i + 1].value == "{" => true
				i = i + 1;

				function.addProperty("return_type", "");

			return_type_done:

				Constructor function_body = getFunctionBody(tokens, Scope(i + 1, findBraceClose(tokens, i + 2, 1) - 1));
				function.addProperty(function_body);

				class_functions.push_back(function);
			}
		}

		return class_functions;
	}
	
	Constructor SlentCompiler::getFunctionBody(vector<Token> tokens, Scope scope) {
		Constructor function_body = Constructor();
		function_body.setName("body");
		vector<vector<Token>> split = split_token(tokens, scope, ";");
		for (int i = 0; i < split.size(); i++) {
			vector<Token> line = split[i];

			if (line[0].value == ";") {
				// TODO: Message(Unnecessary semicolon)
				continue;
			}

			if ((line[0].value == "const") && (line[1].value == "var")) { // declear constant variable
				Constructor constant_variable_declear = Constructor();
				constant_variable_declear.setName("variable_decl_l");
				constant_variable_declear.addProperty("isConst", "1");

				if (line.size() < 3) {
					throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i].line));
					continue;
				}

				if ((line[2].value == "public") || (line[2].value == "private")) {
					throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[i + 1].line));
					continue;
				}

				// get variable type
				if ((line[i].type != TokenType::KEYWORD) && (line[i].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0011, currentFileName, line[i].line));
					continue;
				}
				constant_variable_declear.addProperty("type", line[i].value);

				// get variable name
				if (line[i + 1].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, line[i + 1].line));
					continue;
				}
				constant_variable_declear.addProperty("name", line[i + 1].value);

				if (line[i + 2].value == ";") {
					throwCompileMessage(CompileMessage(SL0024, currentFileName, line[i + 2].line));
				}
				else {
					auto result = getExpression(line, i + 1, 0);
					if (!get<bool>(result)) continue;
					Constructor variable_init = get<Constructor>(result);
					variable_init.setName("variable_init");
					constant_variable_declear.addProperty(variable_init);
					function_body.addProperty(constant_variable_declear);
				}

				i = findNextSemicolon(line, i + 2);
				continue;
			}
			else if (line[i].value == "var") {
				Constructor variable_declear = Constructor();
				variable_declear.setName("variable_decl_l");
				variable_declear.addProperty("isConst", "1");

				// variable access_modifier is not allowed when declearing local variable
				if ((line[i + 1].value == "public") || (line[i + 1].value == "private")) {
					throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[i + 1].line));
					continue;
				}
				else {
					// var [type] [var_name];
					i = i + 1;
				}

				// get variable type
				if ((line[i].type != TokenType::KEYWORD) && (line[i].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0011, currentFileName, line[i].line));
					continue;
				}
				variable_declear.addProperty("type", line[i].value);

				// get variable name
				if (line[i + 1].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, line[i + 1].line));
					continue;
				}
				variable_declear.addProperty("name", line[i + 1].value);

				if (line[i + 2].value == ";") {
					function_body.addProperty(variable_declear);
				}
				else {
					auto result = getExpression(line, i + 1, 0);
					if (!get<bool>(result)) continue;
					Constructor variable_init = get<Constructor>(result);
					variable_init.setName("variable_init");
					variable_declear.addProperty(variable_init);
					function_body.addProperty(variable_declear);
				}

				i = findNextSemicolon(line, i + 2);
				continue;
			}
			else if (line[i].value == "return") {
				if (!tokens_check_index(line, i + 1)) {
					throwCompileMessage(CompileMessage(SL0017, currentFileName, line[i].line));
					continue;
				}

				Constructor return_ = Constructor();
				return_.setName("return");

				if (line[i + 1].value == ";") {
					return_.addProperty("value", "");
					function_body.addProperty(return_);
					continue;
				}

				auto result = getExpression(line, i + 1, 0);
				if (!get<bool>(result)) continue;
				Constructor return_val_expression = get<Constructor>(result);
				return_val_expression.setName("value");
				return_.addProperty(return_val_expression);
				function_body.addProperty(return_);
				continue;
			}

			if (line.back().value == ";") {
				line.pop_back();
			}
			tuple<Constructor, bool> expression = getExpression(line, 0, 0);
			if (!get<bool>(expression)) continue;

			function_body.addProperty(get<Constructor>(expression));
			continue;
		}

		return function_body;
	}
	
	tuple<Constructor, bool> SlentCompiler::getExpression(vector<Token> line, int start_index, int depth) {
		Constructor expression = Constructor();
		expression.setName("expression");

		for (int i = start_index; i < ((depth == 0) ? line.size() : findBracketClose(line, start_index, 1)); i++) {
			function<tuple<Constructor, bool>(bool)> getReference;
			getReference = [line, &i, this, &getReference](bool origin) -> tuple<Constructor, bool> {
				if (line[i].type != TokenType::IDENTIFIER) return make_tuple(Constructor(), false);

				Constructor reference = Constructor();

				if (origin) {
					reference.setName("reference");
					auto result = getReference(false);
					Constructor reference_detail = get<Constructor>(result);
					reference.addProperty(reference_detail);
					return make_tuple(reference, true);
				}

				reference.setName(line[i].value);

				if (tokens_check_index(line, i + 1)) {
					if (line[i + 1].value == ".") {
						if (tokens_check_index(line, i + 2) && (line[i + 2].type == TokenType::IDENTIFIER)) {
							i = i + 2;
							auto result = getReference(false);
							if (!get<bool>(result)) {
								return make_tuple(Constructor(), false);
							}
							Constructor access_detail = get<Constructor>(result);
							Constructor member_access;
							member_access.setName("member_access");
							member_access.addProperty(access_detail);
							reference.addProperty(member_access);
						}
						else {
							throwCompileMessage(CompileMessage(SL0017, currentFileName, i + 1));
							return make_tuple(Constructor(), false);
						}
					}
					else {
						return make_tuple(reference, true);
					}
				}
				else {
					throwCompileMessage(CompileMessage(SL0017, currentFileName, i));
					return make_tuple(Constructor(), false);
				}

				return make_tuple(reference, true);
			};

			if (line[i].type == TokenType::IDENTIFIER) {
				if (line.size() <= (i + 1)) return make_tuple(Constructor(), false);
				
				// function call
				if (line[i + 1].value == "(") {
					Constructor function_call = Constructor();
					function_call.setName("function_call");
					function_call.addProperty("func_name", line[i].value);
					tuple<Constructor, bool> get_parameter = getExpression(line, i + 2, depth + 1);
					if (!get<bool>(get_parameter)) { // conversion of lower depth failed
						i = findBracketClose(line, i + 1, 0);
						continue;
					}
					if (findBracketClose(line, i + 1, 0) == -1) {
						throwCompileMessage(CompileMessage(SL0022, currentFileName, line[i + 1].line));
						return make_tuple(Constructor(), false);
					}
					Constructor parameter_constructor = get<Constructor>(get_parameter);
					parameter_constructor.setName("parameters");
					function_call.addProperty(parameter_constructor);
					expression.addProperty(function_call);
					i = findBracketClose(line, i + 1, 0);
					continue;
				}
				
				auto result = getReference(true);
				if (!get<bool>(result)) continue;
				expression.addProperty(get<Constructor>(result));
				continue;
			}

			if ((line[i].type == TokenType::CONSTANT) || line[i].type == TokenType::LITERAL) {
				auto result = getReference(true);
				if (!get<bool>(result)) continue;
				expression.addProperty(get<Constructor>(result));
				continue;
			}

			if (line[i].type == TokenType::OPERATOR) {
				const vector<string> assignment_operators = { "=", "+=", "-=", "*=", "/=", "%=" };
				const vector<string> relational_operators = { "==", "!=", "<", ">", "<=", ">=" };

				auto getOperation = [line, &i, this, &expression, &depth]() -> tuple<Constructor, bool> {
					Constructor operation = Constructor();
					operation.setName("operation");
					operation.addProperty("type", line[i].value);
					Constructor left = Constructor();
					left.setName("left");
					Constructor right = Constructor();
					right.setName("right");

					// left side of operator
					for (int j = 0; j < expression.getProperties().size(); j++) {
						left.addProperty(expression.getProperties()[j]);
					}

					// right side of operator
					tuple<Constructor, bool> right_expression_result = getExpression(line, i + 1, depth);
					if (!get<bool>(right_expression_result)) {
						return make_tuple(Constructor(), false);
					}
					for (int j = 0; j < get<Constructor>(right_expression_result).getProperties().size(); j++) {
						right.addProperty(get<Constructor>(right_expression_result).getProperties()[j]);
					}

					operation.addProperty(left);
					operation.addProperty(right);
					return make_tuple(operation, true);
				};

				if (find(assignment_operators.begin(), assignment_operators.end(), line[i].value) != assignment_operators.end()) {
					if (depth != 0) {
						throwCompileMessage(CompileMessage(SL0023, currentFileName, line[i].line));
						return make_tuple(Constructor(), false);
					}

					tuple<Constructor, bool> getOperation_result = getOperation();
					if (!get<bool>(getOperation_result)) {
						return make_tuple(Constructor(), false);
					}
					expression.addProperty(get<Constructor>(getOperation_result));
					i = findBracketClose(line, i, depth);
					continue;
				}
				if (find(relational_operators.begin(), relational_operators.end(), line[i].value) != relational_operators.end()) {
					tuple<Constructor, bool> getOperation_result = getOperation();
					if (!get<bool>(getOperation_result)) {
						return make_tuple(Constructor(), false);
					}
					expression.addProperty(get<Constructor>(getOperation_result));
					i = findBracketClose(line, i, depth);
					continue;
				}
			}
		}

		return make_tuple(expression, true);
	}

	bool SlentCompiler::tokens_check_index(vector<Token> tokens, int index) {
		return tokens.size() >= (index + 1);
	}

	vector<vector<Token>> SlentCompiler::split_token(vector<Token> tokens, Scope scope, string delimiter) {
		vector<vector<Token>> split;
		vector<Token> k;
		for (int i = scope.start; i <= scope.end; i++) {
			k.push_back(tokens[i]);
			if (tokens[i].value == delimiter) {
				split.push_back(k);
				k.clear();
			}
		}
		return split;
	}

	// check if the type is available to use for variable type or function return type
	bool SlentCompiler::check_type(string type) {
		// check implemented types
		if ((type == "int") || (type == "string") || (type == "bool") || (type == "float") || (type == "double") || (type == "char") || (type == "short")) {
			return true;
		}

		// TODO: check type with decleared class

		return false;
	}

	int SlentCompiler::findBraceClose(vector<Token> tokens, int cursor, int current_brace) {
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

	int SlentCompiler::findBracketClose(vector<Token> tokens, int cursor, int current_bracket) {
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

		return -1;
	}

	int SlentCompiler::findNextSemicolon(vector<Token> tokens, int cursor) {
		for (int i = cursor; i < tokens.size(); i++) {
			if (tokens[i].value == ";") {
				return i;
			}
		}
	}

	void SlentCompiler::throwCompileMessage(CompileMessage compileMessage) {
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
		string str = colorString(string("[").append(type).append("] ")
			.append(compileMessage.message)
			.append("(").append(compileMessage.file_name)
			.append(":line ").append(to_string(compileMessage.line_index + 1))
			.append(")"), color);
		cout << str << endl;
	}

	string SlentCompiler::bytecode(Constructor root) {
		return "";
	}

	void SlentCompiler::optimize() {

	}

	void SlentCompiler::compile_file(string file_name, string code) {
		currentFileName = file_name;

#ifdef _DEBUG
		try {
			cout << "source code:" << endl << code << endl << endl;
			string preprocessed_code = preprocess(code);
			cout << "preprocessed_code:" << endl << preprocessed_code << endl << endl;
			regex commentRegex("//.*");
			code = regex_replace(preprocessed_code, commentRegex, "");

			vector<Token> tokens = lexer(code);


			// print tokens
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

			cout << endl;
#endif
			Constructor root = parser(tokens); // AST
#ifdef _DEBUG
			cout << "AST:" << endl << root.toPrettyString() << endl;
#endif
		}
		catch (const invalid_argument& e) {
			cerr << "! Compiler internal error (code: SC0000)" << endl << e.what() << endl;
		}

		currentFileName = "";
	}

	void SlentCompiler::AddFile(string file_name, string code) {
		code_files.push_back(make_tuple(file_name, code));
	}

	void SlentCompiler::Compile() {
		for (int i = 0; i < code_files.size(); i++) {
			compile_file(get<0>(code_files[i]), get<1>(code_files[i]));
		}
	}

	typedef int address;

	address MemoryManager::find_free_address() {
		for (address i = 0; i < heap_size; i++) {
			if (heap_memory[i] == nullptr) return i;
		}
	}

	void MemoryManager::set_stack_size(int size) {
		this->stack_size = size;
	}

	void MemoryManager::set_heap_size(int size) {
		this->heap_size = size;
	}

	void MemoryManager::start() {
		heap_memory = (void**)malloc(sizeof(void*) * heap_size);
		for (int i = 0; i < heap_size; i++) {
			heap_memory[i] = nullptr;
		}
		heap_usage = (bool*)calloc(heap_size, sizeof(bool));
	}

	template <typename T>
	int MemoryManager::allocate_heap() {
		T* ptr = (T*)malloc(sizeof(T));

		address free_address = find_free_address();
		heap_memory[free_address] = (void*)ptr;
		heap_usage[free_address] = true;
		return free_address;
	}

	template <typename T>
	void MemoryManager::write_heap(int address, T value) {
		if (!heap_usage[address]) return;
		*((T*)heap_memory[address]) = value;
	}

	void MemoryManager::free_heap(int address) {
		free(heap_memory[address]);
		heap_memory[address] = nullptr;
	}

	template <typename T>
	tuple<T, bool> MemoryManager::read_heap(int address) {
		if (!heap_usage[address]) return make_tuple(T(), false);
		if (heap_memory[address] == nullptr) return make_tuple(T(), false);
		return make_tuple(*((T*)heap_memory[address]), true);
	}

	void SlentVirtualMachine::set_stack_size(int size) {
		memory_manager->set_stack_size(size);
	}

	void SlentVirtualMachine::set_heap_size(int size) {
		memory_manager->set_heap_size(size);
	}

	void SlentVirtualMachine::Run(string bytecode) {
		memory_manager->start();
	}
}