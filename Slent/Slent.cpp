#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <tuple>
#include <sstream>
#include <functional>
#include <fstream>

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
 *    _____ __           __
 *   / ___// /__  ____  / /_
 *   \__ \/ / _ \/ __ \/ __/
 *  ___/ / /  __/ / / / /_
 * /____/_/\___/_/ /_/\__/
 *
 */


#define vec_check_index(vec, index) (vec.size() >= (index + 1))

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

vector<Token> SlentCompiler::getPreprocessorTokens(string code) {
	vector<Token> tokens;

	regex tokenRegex(R"([a-zA-Z_][a-zA-Z0-9_]*!|->|==|=|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|\+|\-|\*|\/|<|>|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"|\(|\)|\{|\}|\[|\]|::|:|;|<|>|\.|,|~|#)");
	// \+|-|\*|\/|<=|>=|<|>|==|!=|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"
	smatch match;

	vector<string> code_split = split(code, '\n');
	for (int i = 0; i < code_split.size(); i++) {
		while (regex_search(code_split[i], match, tokenRegex)) {
			string matched = match.str();
			if (regex_match(matched, regex("\"[^\"]*\""))) {
				tokens.push_back(Token(TokenType::LITERAL, matched, i));
			}
			else if (find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
				tokens.push_back(Token(TokenType::KEYWORD, matched, i));
			}
			else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*!"))) {
				tokens.push_back(Token(TokenType::MACRO, matched, i));
			}
			else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
				tokens.push_back(Token(TokenType::IDENTIFIER, matched, i));
			}
			else if (regex_match(matched, regex("[0-9]+(\\.[0-9]+)?"))) {
				tokens.push_back(Token(TokenType::CONSTANT, matched, i));
			}
			else if (regex_match(matched, regex(R"(==|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|=|\+|\-|\*|\/|<|>|\|\||&&|!)"))) {
				tokens.push_back(Token(TokenType::OPERATOR, matched, i));
			}
			else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|::|:|;|<|>|\.|\,|~|#)"))) {
				tokens.push_back(Token(TokenType::SPECIAL_SYMBOL, matched, i));
			}
			else {
				throwCompileMessage(CompileMessage(SL0000E(matched), currentFileName, i));
			}
			code_split[i] = match.suffix().str();
		}
	}

	return tokens;
}

string SlentCompiler::preprocess(Constructor* module_tree, string code, vector<Macro> macros) {
	vector<Token> tokens = getPreprocessorTokens(code);
	vector<string> imports = getImports(module_tree, tokens);
	string processed_code = runMacros(code, macros);

	return processed_code;
}

vector<string> SlentCompiler::getImports(Constructor* module_tree, vector<Token> tokens) {
	vector<string> imports;

	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].value == "import") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0006E, currentFileName, tokens[i + 5].line));
				break;
			}

			string import_temp = "";

			Constructor* current_depth_module = Constructor::copy(module_tree);
			int j = i + 1;
			while (true) {
				if (tokens[j].type == TokenType::IDENTIFIER) {
					if (!current_depth_module->propertyExist(tokens[j].value)) {
						throwCompileMessage(CompileMessage(SL0033E, currentFileName, tokens[j].line));
						goto err_1;
					}
					current_depth_module = current_depth_module->getProperty(tokens[j].value);
					import_temp.append(tokens[j].value.append("::"));
				}
				else {
					throwCompileMessage(CompileMessage(SL0033E, currentFileName, tokens[j].line));
					goto err_1;
				}
				if (!vec_check_index(tokens, j + 1)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[j + 1].line));
					goto err_1;
				}
				if (tokens[j + 1].value != "::") {
					if (tokens[j + 1].value != ";") {
						throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[j + 1].line));
						goto err_1;
					}
					j = j + 1;
					break;
				}

				j = j + 2;
			}
			goto nerr_1;

		err_1:
			continue;
		nerr_1:

			imports.push_back(import_temp);
			i = j;
			continue;
		}
	}

	return imports;
}

vector<Macro> SlentCompiler::getMacros(Constructor* module_tree, vector<string> codes) {
	vector<Macro> macros;
	for (auto& code : codes) {
		vector<string> lines = split(code, '\n');
		vector<Token> tokens = getPreprocessorTokens(code);

		for (int i = 0; i < tokens.size(); i++) {
			if ((tokens[i].value == "$") && (!vec_check_index(tokens, i + 1))) {
				throwCompileMessage(CompileMessage(SL0001E, currentFileName, tokens[i + 1].line));
				break;
			}

			if (!vec_check_index(tokens, i + 1)) break;

			if ((tokens[i].value == "$") && (tokens[i + 1].value == "macro_def")) {
				if (tokens[i - 1].line == tokens[i].line) {
					throwCompileMessage(CompileMessage(SL0001E, currentFileName, tokens[i].line));
					continue;
				}

				Macro macro = Macro();
				macro.name = tokens[i + 1].value;

				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0002E, currentFileName, tokens[i].line));
					break;
				}
				if ((tokens[i + 1].line != tokens[i + 2].line) || (tokens[i + 2].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0002E, currentFileName, tokens[i].line));
					continue;
				}


				if (!vec_check_index(tokens, i + 5)) {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 2].line));
					break;
				}
				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 2].line));
					continue;
				}
				if (tokens[i + 4].value != "$") {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (tokens[i + 5].value == "(") {
					i = i + 5;
				}
				else if (tokens[i + 5].value == "macro_module") {
					if (!vec_check_index(tokens, i + 6)) {
						throwCompileMessage(CompileMessage(SL0006E, currentFileName, tokens[i + 5].line));
						break;
					}

					Constructor* current_depth_module = Constructor::copy(module_tree);
					int j = i + 6;
					while (true) {
						if (tokens[j].type == TokenType::IDENTIFIER) {
							if (!current_depth_module->propertyExist(tokens[j].value)) {
								throwCompileMessage(CompileMessage(SL0033E, currentFileName, tokens[j].line));
								goto err_2;
							}
							current_depth_module = current_depth_module->getProperty(tokens[j].value);
							macro.macro_module.append(tokens[j].value.append("::"));
						}
						else {
							throwCompileMessage(CompileMessage(SL0033E, currentFileName, tokens[j].line));
							goto err_2;
						}
						if (!vec_check_index(tokens, j + 1)) {
							j = j + 1;
							break;
						}
						if (tokens[j + 1].value != "::") {
							if (tokens[j].line == tokens[j + 1].line) {
								throwCompileMessage(CompileMessage(SL0033E, currentFileName, tokens[j + 1].line));
								goto err_2;
							}
							j = j + 1;
							break;
						}

						j = j + 2;
					}
					goto nerr_2;

				err_2:
					continue;
				nerr_2:

					if (!vec_check_index(tokens, j + 1)) {
						throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 2].line));
						break;
					}

					if (tokens[j].value != "$") {
						throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 2].line));
						continue;
					}

					if (tokens[j + 1].value == "(") {
						i = j + 1;
					}
					else {
						throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 7].line));
						continue;
					}
				}
				else {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i + 4].line));
					continue;
				}

				for (int j = i + 1; j < findBracketClose(tokens, i + 4, 1); j++) {
					if (tokens[j].value == "~") {
						if (!vec_check_index(tokens, j + 1)) {
							throwCompileMessage(CompileMessage(SL0026E, currentFileName, tokens[j].line));
							break;
						}

						if (tokens[j + 1].type == TokenType::IDENTIFIER) {
							if (tokens[j + 1].value == "null") {
								throwCompileMessage(CompileMessage(SL0005E, currentFileName, tokens[j + 1].line));
							}
							else {
								macro.parameters.push_back(tokens[j + 1].value);
							}

							if (!vec_check_index(tokens, j + 2)) {
								throwCompileMessage(CompileMessage(SL0027E, currentFileName, tokens[j].line));
								break;
							}

							if (tokens[j + 2].value == ",") {
								j = j + 2;
								continue;
							}
							else if (tokens[j + 2].value == ")") {
								break;
							}
							else {
								throwCompileMessage(CompileMessage(SL0014E, currentFileName, tokens[j].line));
								j = t_find_next(tokens, j + 2, vector<string> {","});
								continue;
							}
							continue;
						}
						else if (tokens[j + 1].value == ")") {
							macro.parameters.push_back("~");
							break;
						}
						else {
							throwCompileMessage(CompileMessage(SL0005E, currentFileName, tokens[j].line));

							if (!vec_check_index(tokens, j + 2)) {
								throwCompileMessage(CompileMessage(SL0027E, currentFileName, tokens[j + 1].line));
								break;
							}

							if (tokens[j + 2].value == ",") {
								j = j + 2;
								continue;
							}
							else {
								throwCompileMessage(CompileMessage(SL0014E, currentFileName, tokens[j + 1].line));
								j = t_find_next(tokens, j + 2, vector<string> {","});
								continue;
							}
						}
					}
					else if (tokens[j].type == TokenType::IDENTIFIER) {
						macro.parameters.push_back(tokens[j].value);

						if (!vec_check_index(tokens, j + 1)) {
							throwCompileMessage(CompileMessage(SL0027E, currentFileName, tokens[j].line));
							break;
						}

						if (tokens[j + 1].value == ",") {
							j = j + 1;
							continue;
						}
						else if (tokens[j + 1].value == ")") {
							break;
						}
						else {
							throwCompileMessage(CompileMessage(SL0014E, currentFileName, tokens[j].line));
							j = t_find_next(tokens, j + 2, vector<string> {","});
							continue;
						}
						continue;
					}
					else {
						throwCompileMessage(CompileMessage(SL0005E, currentFileName, tokens[j].line));
					}
				}
				i = findBracketClose(tokens, i + 3, 1) + 1;

				if (!vec_check_index(tokens, i)) {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i - 1].line));
					break;
				}

				if (tokens[i].value != "=>") {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i - 1].line));
					continue;
				}
				if (tokens[i + 1].value != "{") {
					throwCompileMessage(CompileMessage(SL0004E, currentFileName, tokens[i].line));
					continue;
				}

				int start_line = tokens[i + 2].line;
				int end_line = tokens[findBraceClose(tokens, i + 1, 1)].line;

				int start_brace_index = lines[start_line].find("{", lines[start_line].find("=>"));

				if (start_line == tokens[i + 1].line) {

					macro.body += lines[start_line].substr(start_brace_index);
				}
				else {
					macro.body += lines[start_line];
				}

				for (int l = start_line + 1; l < end_line; l++) {
					macro.body += lines[l];
				}

				if (end_line == tokens[findBraceClose(tokens, i + 1, 1) + 1].line) {
					int brace_count = 1;
					for (int l = start_line; l < lines.size(); l++) {
						for (int j = (l == start_line) ? (start_brace_index + 1) : 0; j < lines[l].size(); j++) {
							if (lines[l][j] == '{') brace_count++;
							else if (lines[l][j] == '}') brace_count--;

							if (brace_count == 0) {
								if (l != end_line) {
									cerr << "! Compiler internal error (code: SC0001)" << endl;
									exit(1);
								}
								macro.body += lines[end_line].substr(0, j);
								goto done;
							}
						}
					}
				}
				else {
					macro.body += lines[end_line];
				}

			done:

				macros.push_back(macro);
				continue;
			}
		}
	}

	return macros;
}

string SlentCompiler::runMacros(string code, vector<Macro> macros) {
	string m_code = code;
	bool macro_exist = false;
	do {
		for (auto& macro : macros) {
			int last_index = 0;
			while (true) {
				int index = m_code.find(macro.name + "!", last_index);

				if (index == string::npos) break;

				last_index = index + (macro.name + "!").size();

				auto findMacroEnd = [&index, &macro, &m_code]() -> int {
					bool bracket_started = false;
					int brackets = 0;
					for (int i = index + (macro.name + "!").size(); i < m_code.size(); i++) {
						if (m_code[i] == '(') brackets++;
						if (bracket_started && (brackets == 0)) return i;
					}
				};

				auto getLineNum = [](string str, int index) -> int {
					int line_num = 0;
					for (int i = 0; i <= index; i++) {
						if (str[i] == '\n') {
							line_num++;
						}
					}
					return line_num;
				};

				int state = -1;
				/*
				-1: haven't received anything
				0: receiving param
				1: finished receiving param and waiting for ','
				*/
				string param_temp = "";
				vector<string> params_val;
				for (int i = m_code.find('(', index) + 1; i < findMacroEnd(); i++) {
					if (m_code[i] == ' ') {
						if (state == 0) state = 1;
					}
					else if (m_code[i] == ',') {
						if ((state == 0) || (state == 1)) {
							params_val.push_back(param_temp);
							param_temp = "";
							state = -1;
						}
						else {
							throwCompileMessage(CompileMessage(SL0005E, currentFileName, getLineNum(m_code, i)));
							m_code.replace(index, findMacroEnd() - index + 1, "");
							goto err;
						}
					}
					else {
						if ((state == -1) || (state == 0)) {
							param_temp += m_code[i];
							state = 0;
						}
						else {
							throwCompileMessage(CompileMessage(SL0023E, currentFileName, getLineNum(m_code, i)));
							m_code.replace(index, findMacroEnd() - index + 1, "");
							goto err;
						}
					}
				}

				m_code.replace(index, findMacroEnd() - index + 1, runMacro(macro, params_val));
				macro_exist = true;

				goto nerr;
			err:
				continue;
			nerr:;
			}
		}
	} while (macro_exist);

	return m_code;
}

string SlentCompiler::runMacro(Macro macro, vector<string> params_val) {
	string result = macro.body;
	for (int i = 0; i < macro.parameters.size(); i++) {
		string target = string("#").append(macro.parameters[i]).append("#");
		result.replace(macro.body.find(target), target.size(), params_val[i]);
	}
	return result;
}

Constructor* SlentCompiler::getModuleTree(std::string code) {
	Constructor* root = new Constructor();
	root->setName("root");
	vector<Token> tokens = getPreprocessorTokens(code);
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].value == "export") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0028E, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].value != "module") {
				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0029E, currentFileName, tokens[i + 1].line));
					break;
				}

				if (tokens[i + 2].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 2].line));
					break;
				}

				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 4)) {
					throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (findBraceClose(tokens, i + 4, 1) == -1) {
					throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[tokens.size() - 1].line));
					continue;
				}

				Constructor* sub_modules = getSubModuleTree(tokens, Scope(i + 4, findBraceClose(tokens, i + 4, 1) - 1));
				sub_modules->setName(tokens[i + 2].value);
				root->addProperty(sub_modules);
				root->addProperty("export", "1");
			}
		}
		else if (tokens[i].value == "module") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 2].value != "{") {
				throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 3)) {
				throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[i + 2].line));
				continue;
			}

			if (findBraceClose(tokens, i + 3, 1) == -1) {
				throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[tokens.size() - 1].line));
				continue;
			}

			Constructor* sub_modules = getSubModuleTree(tokens, Scope(i + 3, findBraceClose(tokens, i + 3, 1) - 1));
			sub_modules->setName(tokens[i + 2].value);
			root->addProperty(sub_modules);
			root->addProperty("export", "0");
		}
	}

	return root;
}

Constructor* SlentCompiler::getSubModuleTree(vector<Token> tokens, Scope scope) {
	Constructor* root = new Constructor();
	root->setName("sub_root");
	for (int i = scope.start; i < scope.end; i++) {
		if (tokens[i].value == "export") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0028E, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].value != "module") {
				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0029E, currentFileName, tokens[i + 1].line));
					break;
				}

				if (tokens[i + 2].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 2].line));
					break;
				}

				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 4)) {
					throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (findBraceClose(tokens, i + 4, 1) == -1) {
					throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[tokens.size() - 1].line));
					continue;
				}

				Constructor* sub_modules = getSubModuleTree(tokens, Scope(i + 4, findBraceClose(tokens, i + 4, 1) - 1));
				sub_modules->setName(tokens[i + 2].value);
				root->addProperty(sub_modules);
				root->addProperty("export", "1");
			}
		}
		else if (tokens[i].value == "module") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 2].value != "{") {
				throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 3)) {
				throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[i + 2].line));
				continue;
			}

			if (findBraceClose(tokens, i + 3, 1) == -1) {
				throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[tokens.size() - 1].line));
				continue;
			}

			Constructor* sub_modules = getSubModuleTree(tokens, Scope(i + 3, findBraceClose(tokens, i + 3, 1) - 1));
			sub_modules->setName(tokens[i + 2].value);
			root->addProperty(sub_modules);
			root->addProperty("export", "0");
		}
	}

	return root;
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
			if (regex_match(matched, regex("\"[^\"]*\""))) {
				tokens.push_back(Token(TokenType::LITERAL, matched, i));
			}
			else if (find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
				tokens.push_back(Token(TokenType::KEYWORD, matched, i));
			}
			else if (regex_match(matched, regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
				tokens.push_back(Token(TokenType::IDENTIFIER, matched, i));
			}
			else if (regex_match(matched, regex("[0-9]+(\\.[0-9]+)?"))) {
				tokens.push_back(Token(TokenType::CONSTANT, matched, i));
			}
			else if (regex_match(matched, regex(R"(==|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|=|\+|\-|\*|\/|<|>|\|\||&&|!)"))) {
				tokens.push_back(Token(TokenType::OPERATOR, matched, i));
			}
			else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|:|;|<|>|\.|\,)"))) {
				tokens.push_back(Token(TokenType::SPECIAL_SYMBOL, matched, i));
			}
			else {
				throwCompileMessage(CompileMessage(SL0000E(matched), currentFileName, i));
			}
			code_split[i] = match.suffix().str();
		}
	}

	return tokens;
}

Constructor* SlentCompiler::parser(vector<Token> tokens) {
	Constructor* root = new Constructor();
	root->setName("root");

	vector<string> currentModule;

	vector<Constructor*> current_module_body;
	current_module_body.push_back(root);

	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].value == "import") {
			if (currentModule.size() != 0) {
				throwCompileMessage(CompileMessage(SL0041E, currentFileName, tokens[i].line));
				i = findNextSemicolon(tokens, i);
				continue;
			}

			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0042E, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0042E, currentFileName, tokens[i + 1].line));
				i = findNextSemicolon(tokens, i + 1);
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 2].value != ";") {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 1].line));
				i = findNextSemicolon(tokens, i + 2);
				continue;
			}

			Constructor* import = new Constructor();
			import->setName("import");
			import->setValue(tokens[i + 1].value);
			root->addProperty(import);
			i = i + 2;
		}
		else if (tokens[i].value == "export") {
			Constructor* export_ = new Constructor();

			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0028E, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].value != "module") {
				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0029E, currentFileName, tokens[i + 1].line));
					break;
				}

				if (tokens[i + 2].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 2].line));
					break;
				}

				if (tokens[i + 3].value == "{") {
					if (!vec_check_index(tokens, i + 4)) {
						throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[i + 3].line));
						continue;
					}

					if (findBraceClose(tokens, i + 4, 1) == -1) {
						throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[tokens.size() - 1].line));
						i = i + 3;
						continue;
					}

					export_->setName("export");
					string moduleTree = "";
					for (auto& module_ : currentModule) {
						if (moduleTree != "") moduleTree.append("/");
						moduleTree.append(module_);
					}
					export_->setValue(moduleTree.append("/").append(tokens[i + 2].value));
					i = i;
				}
				else if (tokens[i + 3].value == ";") {
					export_->setName("export");
					string moduleTree = "";
					for (auto& module_ : currentModule) {
						if (moduleTree != "") moduleTree.append("/");
						moduleTree.append(module_);
					}
					export_->setValue(moduleTree.append("/").append(tokens[i + 2].value));
					i = i + 3;
				}
				else {
					throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 2].line));
					i = i + 2;
					continue;
				}
			}
			else {
				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 1].line));
					break;
				}

				if (tokens[i + 2].value != ";") {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 2].line));
					break;
				}

				export_->setName("export");
				string moduleTree = "";
				for (auto& module_ : currentModule) {
					if (moduleTree != "") moduleTree.append("/");
					moduleTree.append(module_);
				}
				moduleTree.append("/").append(tokens[i + 1].value);
				export_->setValue(moduleTree);
			}
		}
		else if (tokens[i].value == "module") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0030E, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 2].value != "{") {
				throwCompileMessage(CompileMessage(SL0031E, currentFileName, tokens[i + 1].line));
				i = i + 1;
				continue;
			}

			if (!vec_check_index(tokens, i + 3)) {
				throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[i + 2].line));
				continue;
			}

			if (findBraceClose(tokens, i + 3, 1) == -1) {
				throwCompileMessage(CompileMessage(SL0032E, currentFileName, tokens[tokens.size() - 1].line));
				i = i + 2;
				continue;
			}

			currentModule.push_back(tokens[i + 1].value);

			Constructor* new_module = new Constructor();
			new_module->setName("module");
			Constructor* new_module_body = new Constructor();
			new_module_body->setName("body");
			new_module->addProperty("name", tokens[i + 1].value);
			new_module->addProperty(new_module_body);

			root->addProperty(new_module);
			current_module_body.push_back(new_module_body);

			i = i + 2;
		}
		else if (tokens[i].value == "dynamic") {
			tuple<Constructor*, bool> getExternalFunction_result = getExternalFunction(tokens, i, true);
			if (!get<bool>(getExternalFunction_result)) {
				i = findNextSemicolon(tokens, i);
				continue;
			}
			Constructor* external_function = get<Constructor*>(getExternalFunction_result);
			current_module_body.back()->addProperty(external_function);
			i = findNextSemicolon(tokens, i);
			continue;
		}
		else if (tokens[i].value == "extern") {
			tuple<Constructor*, bool> getExternalFunction_result = getExternalFunction(tokens, i, false);
			if (!get<bool>(getExternalFunction_result)) {
				i = findNextSemicolon(tokens, i);
				continue;
			}
			Constructor* external_function = get<Constructor*>(getExternalFunction_result);
			current_module_body.back()->addProperty(external_function);
			i = findNextSemicolon(tokens, i);
			continue;
		}
		else if (tokens[i].value == "func") {
			int functionBodyEnd = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);

			if (vec_check_index(tokens, functionBodyEnd + 1)) {
				if (tokens[functionBodyEnd + 1].value == ":") {
					throwCompileMessage(CompileMessage(SL0040E, currentFileName, tokens[functionBodyEnd + 2].line));
					if (vec_check_index(tokens, functionBodyEnd + 2)) {
						if ((tokens[functionBodyEnd + 2].value == "public")
							|| (tokens[functionBodyEnd + 2].value == "protected")
							|| (tokens[functionBodyEnd + 2].value == "private")) {
							i = functionBodyEnd + 2;
						}
					}
					else {
						i = functionBodyEnd + 1;
					}
					continue;
				}
			}

			tuple<Constructor*, bool> getFunction_result = getFunction(tokens, Scope(i, functionBodyEnd), true);
			if (!get<bool>(getFunction_result)) {
				i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
				continue;
			}
			Constructor* function = get<Constructor*>(getFunction_result);
			function->setName("function_g");
			function->addProperty("access_modifier", "");
			current_module_body.back()->addProperty(function);
			i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
			continue;
		}
		else if (tokens[i].value == "class") {
			tuple<Constructor*, int, bool> getClass_result = getClass(tokens, i);
			i = get<int>(getClass_result);
			if (!get<bool>(getClass_result)) {
				i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
				continue;
			}
			Constructor* class_define = get<Constructor*>(getClass_result);
			current_module_body.back()->addProperty(class_define);
			continue;
		}
		else if (tokens[i].value == "}") {
			currentModule.erase(currentModule.end() - 1);
			current_module_body.erase(current_module_body.end() - 1);
		}
		else {
			cout << tokens[i].value << endl;
			throwCompileMessage(CompileMessage(SL0035E, currentFileName, tokens[i].line));
			continue;
		}
	}

	return root;
}

tuple<Constructor*, bool> SlentCompiler::getFunction(vector<Token> tokens, Scope scope, bool includeBody) {
	int i = scope.start;
	if (tokens[i].value == "func") {
		if ((i + 1) > scope.end) {
			throwCompileMessage(CompileMessage(SL0019E, currentFileName, tokens[i].line));
			return make_tuple(new Constructor(), false);
		}

		if (tokens[i + 1].type != TokenType::IDENTIFIER) {
			throwCompileMessage(CompileMessage(SL0020E, currentFileName, tokens[i + 1].line));
			return make_tuple(new Constructor(), false);
		}

		Constructor* function = new Constructor();
		function->setName("function");
		function->addProperty("name", tokens[i + 1].value);

		if ((i + 2) > scope.end) {
			throwCompileMessage(CompileMessage(SL0011E, currentFileName, tokens[i + 1].line));
			return make_tuple(new Constructor(), false);
		}

		if (tokens[i + 2].value != "(") {
			throwCompileMessage(CompileMessage(SL0011E, currentFileName, tokens[i + 2].line));
			return make_tuple(new Constructor(), false);
		}
		i = i + 2;

		// get function parameters
		Constructor* parameters = new Constructor();
		parameters->setName("parameters");

		int k = 0;

		bool error = false;

		if (tokens[i + 1].value == ")") {
			function->addProperty("parameters", "");
			goto get_parameter_finished;
		}

		for (int j = i + 1; j < findBracketClose(tokens, i + 1, 1); j++) {
			if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, tokens[j].line));
				j = t_find_next(tokens, j, vector<string> {","});
				error = true;
				continue;
			}

			if ((j + 1) >= findBracketClose(tokens, i + 1, 1)) {
				break;
			}

			if (tokens[j + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, tokens[j + 1].line));
				i = t_find_next(tokens, j + 1, vector<string> {","});
				continue;
			}

			if ((j + 2) >= findBracketClose(tokens, i + 1, 1)) {
				break;
			}

			if (tokens[j + 2].value != ",") {
				throwCompileMessage(CompileMessage(SL0014E, currentFileName, tokens[j + 1].line));
				i = t_find_next(tokens, j + 2, vector<string> {","});
				continue;
			}

			string type = tokens[j].value;
			string name = tokens[j + 1].value;
			Constructor* param = new Constructor();
			param->setName(string("param").append(to_string(k)));
			param->addProperty("type", type);
			param->addProperty("name", name);
			parameters->addProperty(param);
			k++;
			j = j + 2;
		}

		if (error) return make_tuple(new Constructor(), false);

		function->addProperty(parameters);
	get_parameter_finished:

		i = t_find_next(tokens, i + 1, vector<string> {")"});

		if ((i + 2) > scope.end) {
			throwCompileMessage(CompileMessage(SL0022E, currentFileName, tokens[i].line));
			return make_tuple(new Constructor(), false);
		}

		if (tokens[i + 1].value != "{") {
			if (tokens[i + 1].value == "->") {
				if ((i + 3) > scope.end) {
					throwCompileMessage(CompileMessage(SL0021E, currentFileName, tokens[i + 2].line));
					return make_tuple(new Constructor(), false);
				}

				if ((tokens[i + 2].type != TokenType::KEYWORD) && (tokens[i + 2].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0012E, currentFileName, tokens[i + 3].line));
					return make_tuple(new Constructor(), false);
				}
				function->addProperty("return_type", tokens[i + 2].value);
				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0022E, currentFileName, tokens[i + 3].line));
				}
				i = i + 3;
				goto return_type_done;
			}
			throwCompileMessage(CompileMessage(SL0022E, currentFileName, tokens[i].line));
			return make_tuple(new Constructor(), false);
		}
		// tokens[i + 1].value == "{" => true
		i = i + 1;

		function->addProperty("return_type", "");

	return_type_done:

		if (includeBody) {
			Constructor* function_body = getFunctionBody(tokens, Scope(i + 1, findBraceClose(tokens, i + 2, 1) - 1));
			function->addProperty(function_body);
		}

		return make_tuple(function, true);
	}
}

tuple<Constructor*, int, bool> SlentCompiler::getClass(vector<Token> tokens, int cursor) {
	Constructor* class_define = new Constructor();
	class_define->setName("class");
	if (!vec_check_index(tokens, cursor + 1)) {
		throwCompileMessage(CompileMessage(SL0008E, currentFileName, tokens[cursor].line));
	}
	if (tokens[cursor + 1].type != TokenType::IDENTIFIER) {
		throwCompileMessage(CompileMessage(SL0009E, currentFileName, tokens[cursor + 1].line));
		return make_tuple(class_define, cursor, false);
	}
	class_define->addProperty("name", tokens[cursor + 1].value);
	if (tokens[cursor + 2].value != "{") {
		throwCompileMessage(CompileMessage(SL0010E, currentFileName, tokens[cursor + 1].line));
		return make_tuple(class_define, cursor + 1, false);
	}
	Constructor* members = getClassMembers(tokens, Scope(cursor + 3, findBraceClose(tokens, cursor + 3, 1) - 1));
	class_define->addProperty(members);

	return make_tuple(class_define, findBraceClose(tokens, cursor + 3, 1), true);
}

Constructor* SlentCompiler::getClassMembers(vector<Token> tokens, Scope scope) {
	Constructor* classMembers = new Constructor();
	classMembers->setName("members");
	vector<Constructor*> class_constructors = getClassConstructors(tokens, scope);
	for (int i = 0; i < class_constructors.size(); i++) {
		classMembers->addProperty(class_constructors[i]);
	}
	vector<Constructor*> class_variables = getClassVariables(tokens, scope);
	for (int i = 0; i < class_variables.size(); i++) {
		classMembers->addProperty(class_variables[i]);
	}
	vector<Constructor*> class_functions = getClassFunctions(tokens, scope, true);
	for (int i = 0; i < class_functions.size(); i++) {
		classMembers->addProperty(class_functions[i]);
	}

	return classMembers;
}

vector<Constructor*> SlentCompiler::getClassConstructors(vector<Token> tokens, Scope scope) {
	vector<Constructor*> class_constructors;
	for (int i = scope.start; i <= scope.end; i++) {
		if (tokens[i].value == "construct") {
			Constructor* constructor = new Constructor();
			constructor->setName("constructor");

			if (tokens[i + 1].value != "(") {
				throwCompileMessage(CompileMessage(SL0011E, currentFileName, tokens[i + 4].line));
				continue;
			}

			Constructor* parameters = new Constructor();
			parameters->setName("parameters");
			int k = 0;
			for (int j = i + 2; j < findBracketClose(tokens, i + 2, 1); j++) {
				if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0012E, currentFileName, tokens[j].line));
					i = t_find_next(tokens, j, vector<string> {","});
					continue;
				}
				if (tokens[j + 1].type != TokenType::IDENTIFIER) {
					if ((j + 1) >= findBracketClose(tokens, i + 2, 1)) {
						break;
					}
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, tokens[j + 1].line));
					i = t_find_next(tokens, j + 1, vector<string> {","});
					continue;
				}
				if (tokens[j + 2].value != ",") {
					if ((j + 2) >= findBracketClose(tokens, i + 2, 1)) {
						break;
					}
					throwCompileMessage(CompileMessage(SL0014E, currentFileName, tokens[j + 1].line));
					i = t_find_next(tokens, j + 2, vector<string> {","});
					continue;
				}
				string type = tokens[j].value;
				string name = tokens[j + 1].value;
				Constructor* param = new Constructor();
				param->setName(string("param").append(to_string(k)));
				param->addProperty("type", type);
				param->addProperty("name", name);
				parameters->addProperty(param);
				k++;
				j = j + 2;
			}
			constructor->addProperty(parameters);

			i = t_find_next(tokens, i + 2, vector<string> {")"});

			if (tokens[i + 1].value != "{") {
				throwCompileMessage(CompileMessage(SL0022E, currentFileName, tokens[i + 1].line));
				continue;
			}

			Constructor* constructor_body = getFunctionBody(tokens, Scope(i + 2, findBraceClose(tokens, i + 2, 1) - 1));
			constructor->addProperty(constructor_body);

			class_constructors.push_back(constructor);
		}
	}

	return class_constructors;
}

vector<Constructor*> SlentCompiler::getClassVariables(vector<Token> tokens, Scope scope) {
	vector<Constructor*> class_variables;
	for (int i = scope.start; i <= scope.end; i++) {
		if ((tokens[i].value == "construct") || (tokens[i].value == "func")) {
			i = findBraceClose(tokens, t_find_next(tokens, i, vector<string> {"{"}), 0);
			continue;
		}

		if (tokens[i].value == "var") {
			Constructor* variable = new Constructor();
			variable->setName("variable_c");

			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0016E, currentFileName, tokens[i].line));
				continue;
			}

			// get type
			if ((tokens[i + 1].type != TokenType::KEYWORD) && (tokens[i + 1].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, tokens[i + 1].line));
				i = findNextSemicolon(tokens, i + 1);
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, tokens[i + 1].line));
				continue;
			}

			int k;

			// get name
			if (tokens[i + 2].type == TokenType::IDENTIFIER) {
				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 2].line));
					continue;
				}

				int reference_offset = 0;

				if (tokens[i + 3].value == "&") {
					variable->addProperty("type", tokens[i + 1].value + "&");
					reference_offset = 1;
				}
				else {
					variable->addProperty("type", tokens[i + 1].value);
				}

				variable->addProperty("nullable", "0");
				variable->addProperty("name", tokens[i + 2].value);

				int access_modifier_offset = 0;

				// get access_modifier
				if (tokens[i + 3 + reference_offset].value == ":") {
					if (!vec_check_index(tokens, i + 4 + reference_offset)) {
						throwCompileMessage(CompileMessage(SL0036E, currentFileName, tokens[i + 3 + reference_offset].line));
						continue;
					}

					if (tokens[i + 4 + reference_offset].value == "public") {
						variable->addProperty("access_modifier", "public");
						access_modifier_offset = 2;
					}
					else if (tokens[i + 4 + reference_offset].value == "protected") {
						variable->addProperty("access_modifier", "protected");
						access_modifier_offset = 2;
					}
					else if (tokens[i + 4 + reference_offset].value == "private") {
						variable->addProperty("access_modifier", "private");
						access_modifier_offset = 2;
					}
					else {
						throwCompileMessage(CompileMessage(SL0036E, currentFileName, tokens[i + 4 + reference_offset].line));
						i = findNextSemicolon(tokens, i + 4 + reference_offset);
						continue;
					}
				}

				k = i + 2 + reference_offset + access_modifier_offset;
			}
			else if (tokens[i + 2].value == "?") {
				variable->addProperty("nullable", "1");

				if (!vec_check_index(tokens, 3)) {
					throwCompileMessage(CompileMessage(SL0016E, currentFileName, tokens[i + 2].line));
					continue;
				}

				int reference_offset = 0;

				if (tokens[i + 3].value == "&") {
					variable->addProperty("type", tokens[i + 1].value + "&");
					reference_offset = 1;
				}
				else {
					variable->addProperty("type", tokens[i + 1].value);
				}

				if (tokens[i + 3 + reference_offset].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, tokens[i + 3 + reference_offset].line));
					i = findNextSemicolon(tokens, i + 3 + reference_offset);
					continue;
				}
				variable->addProperty("name", tokens[i + 3 + reference_offset].value);

				int access_modifier_offset = 0;

				if (!vec_check_index(tokens, i + 4 + reference_offset)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 3 + reference_offset].line));
					continue;
				}

				if (tokens[i + 4 + reference_offset].value == ":") {
					if (!vec_check_index(tokens, i + 5 + reference_offset)) {
						throwCompileMessage(CompileMessage(SL0036E, currentFileName, tokens[i + 4 + reference_offset].line));
						continue;
					}

					if (tokens[i + 5 + reference_offset].value == "public") {
						variable->addProperty("access_modifier", "public");
						access_modifier_offset = 2;
					}
					else if (tokens[i + 5 + reference_offset].value == "protected") {
						variable->addProperty("access_modifier", "protected");
						access_modifier_offset = 2;
					}
					else if (tokens[i + 5 + reference_offset].value == "private") {
						variable->addProperty("access_modifier", "private");
						access_modifier_offset = 2;
					}
					else {
						throwCompileMessage(CompileMessage(SL0036E, currentFileName, tokens[i + 5 + reference_offset].line));
						i = findNextSemicolon(tokens, i + 5 + reference_offset);
						continue;
					}
				}

				k = i + 3 + reference_offset + access_modifier_offset;
			}
			else if (tokens[i + 2].value == "&") {
				variable->addProperty("nullable", "0");
				variable->addProperty("type", tokens[i + 1].value + "&");

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0016E, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (tokens[i + 3].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, tokens[i + 3].line));
					i = findNextSemicolon(tokens, i + 3);
					continue;
				}
				variable->addProperty("name", tokens[i + 3].value);

				int access_modifier_offset = 0;

				if (!vec_check_index(tokens, i + 4)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (tokens[i + 4].value == ":") {
					if (!vec_check_index(tokens, i + 5)) {
						throwCompileMessage(CompileMessage(SL0036E, currentFileName, tokens[i + 4].line));
					}

					if (tokens[i + 5].value == "public") {
						variable->addProperty("access_modifier", "public");
						access_modifier_offset = 2;
					}
					else if(tokens[i + 5].value == "protected") {
						variable->addProperty("access_modifier", "protected");
						access_modifier_offset = 2;
					}
					else if (tokens[i + 5].value == "private") {
						variable->addProperty("access_modifier", "private");
						access_modifier_offset = 2;
					}
				}
				
				k = i + 3 + access_modifier_offset;
			}
			else if (tokens[i + 2].value == "=") {
				variable->addProperty("type", "auto");
				variable->addProperty("name", tokens[i + 1].value);

				k = 1;
			}
			else if (tokens[i + 2].value == ";") {
				variable->addProperty("name", tokens[i + 1].value);
				throwCompileMessage(CompileMessage(SL0034E, currentFileName, tokens[i + 2].line));
				i = i + 2;
				continue;
			}
			else {
				cout << tokens[i].value << endl << tokens[i+1].value << endl << tokens[i+2].value << endl;
				throwCompileMessage(CompileMessage(SL0013E, currentFileName, tokens[i + 2].line));
				i = findNextSemicolon(tokens, i + 2);
				continue;
			}

			if (!vec_check_index(tokens, k + 1)) {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[k].line));
				continue;
			}

			if (tokens[k + 1].value == ";") {
				class_variables.push_back(variable);
				i = k + 1;
			}
			else if (tokens[k + 1].value == "=") {
				auto result = getExpression(tokens, i + 2, 0);
				if (!get<bool>(result)) continue;
				Constructor* variable_init = get<Constructor*>(result);
				variable_init->setName("variable_init");
				variable->addProperty(variable_init);
				class_variables.push_back(variable);
				i = findNextSemicolon(tokens, k + 1);
			}
			else {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, tokens[i].line));
				i = findNextSemicolon(tokens, k + 1);
				continue;
			}
		}
	}

	return class_variables;
}

vector<Constructor*> SlentCompiler::getClassFunctions(vector<Token> tokens, Scope scope, bool includeBody) {
	vector<Constructor*> class_functions;
	for (int i = scope.start; i <= scope.end; i++) {
		if (tokens[i].value == "dynamic") {
			tuple<Constructor*, bool> getExternalFunction_result = getExternalFunction(tokens, i, true);
			if (!get<bool>(getExternalFunction_result)) {
				i = findNextSemicolon(tokens, i);
				continue;
			}
			Constructor* external_function = get<Constructor*>(getExternalFunction_result);
			class_functions.push_back(external_function);
			i = findNextSemicolon(tokens, i);
			continue;
		}
		else if (tokens[i].value == "extern") {
			tuple<Constructor*, bool> getExternalFunction_result = getExternalFunction(tokens, i, false);
			if (!get<bool>(getExternalFunction_result)) {
				i = findNextSemicolon(tokens, i);
				continue;
			}
			Constructor* external_function = get<Constructor*>(getExternalFunction_result);
			class_functions.push_back(external_function);
			i = findNextSemicolon(tokens, i);
			continue;
		}
		else if (tokens[i].value == "func") {
			int functionBodyEnd = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
			tuple<Constructor*, bool> getFunction_result = getFunction(tokens, Scope(i, functionBodyEnd), true);
			if (get<bool>(getFunction_result)) {
				Constructor* function = get<Constructor*>(getFunction_result);
				function->setName("function_c");
				
				if (!vec_check_index(tokens, functionBodyEnd + 1)) {
					if (tokens[functionBodyEnd + 1].value == ":") {
						if (!vec_check_index(tokens, functionBodyEnd + 2)) {
							throwCompileMessage(CompileMessage(SL0036E, currentFileName, tokens[functionBodyEnd + 1].line));
							continue;
						}
						
						if (tokens[functionBodyEnd + 2].value == "public") {
							function->addProperty("access_modifier", "public");
						}
						else if (tokens[functionBodyEnd + 2].value == "protected") {
							function->addProperty("access_modifier", "protected");
						}
						else if (tokens[functionBodyEnd + 2].value == "private") {
							function->addProperty("access_modifier", "private");
						}
					}
				}

				class_functions.push_back(function);
			}
		}
	}

	return class_functions;
}

Constructor* SlentCompiler::getFunctionBody(vector<Token> tokens, Scope scope) {
	Constructor* function_body = new Constructor();
	function_body->setName("body");
	vector<vector<Token>> split = split_token(tokens, scope, ";");
	for (int i = 0; i < split.size(); i++) {
		vector<Token> line = split[i];

		if (line[0].value == ";") {
			continue;
		}

		if (line[0].value == "val") { // declear value variable
			Constructor* constant_variable_declear = new Constructor();
			constant_variable_declear->setName("variable_decl_l");
			constant_variable_declear->addProperty("isVal", "1");

			if (!vec_check_index(line, 1)) {
				throwCompileMessage(CompileMessage(SL0016E, currentFileName, line[0].line));
				continue;
			}

			// get type
			if ((line[1].type != TokenType::KEYWORD) && (line[1].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, line[1].line));
				continue;
			}

			if (!vec_check_index(line, 2)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, line[1].line));
				continue;
			}

			int k;

			// get name
			if (line[2].type == TokenType::IDENTIFIER) {
				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[2].line));
					continue;
				}

				int reference_offset = 0;

				if (line[3].value == "&") {
					constant_variable_declear->addProperty("type", tokens[i + 1].value + "&");
					reference_offset = 1;
				}
				else {
					constant_variable_declear->addProperty("type", line[1].value);
				}

				constant_variable_declear->addProperty("nullable", "0");
				constant_variable_declear->addProperty("name", line[2].value);

				if (!vec_check_index(line, 3 + reference_offset)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[2 + reference_offset].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[3 + reference_offset].value == ":") {
					throwCompileMessage(CompileMessage(SL0015E, currentFileName, line[3 + reference_offset].line));
					continue;
				}

				k = 2 + reference_offset;
			}
			else if (line[2].value == "?") {
				constant_variable_declear->addProperty("nullable", "1");

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0016E, currentFileName, line[2].line));
					continue;
				}

				int reference_offset = 0;

				if (line[3].value == "&") {
					constant_variable_declear->addProperty("type", line[1].value + "&");
					reference_offset = 1;
				}
				else {
					constant_variable_declear->addProperty("type", line[1].value);
				}

				if (line[3 + reference_offset].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, line[3 + reference_offset].line));
					continue;
				}
				constant_variable_declear->addProperty("name", line[3 + reference_offset].value);

				if (!vec_check_index(line, 4 + reference_offset)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[3 + reference_offset].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[4 + reference_offset].value == ":") {
					throwCompileMessage(CompileMessage(SL0015E, currentFileName, line[4 + reference_offset].line));
					continue;
				}

				k = 3 + reference_offset;
			}
			else if (line[2].value == "&") {
				constant_variable_declear->addProperty("nullable", "0");
				constant_variable_declear->addProperty("type", line[1].value + "&");

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0016E, currentFileName, line[2].line));
					continue;
				}

				if (line[3].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, tokens[3].line));
					continue;
				}
				constant_variable_declear->addProperty("name", line[3].value);

				if (!vec_check_index(line, 4)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[2].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[4].value == ":") {
					throwCompileMessage(CompileMessage(SL0015E, currentFileName, line[4].line));
					continue;
				}

				k = 3;
			}
			else if (line[2].value == "=") {
				constant_variable_declear->addProperty("type", "auto");
				constant_variable_declear->addProperty("name", line[1].value);

				k = 1;
			}
			else if (line[2].value == ";") {
				constant_variable_declear->addProperty("name", line[1].value);
				throwCompileMessage(CompileMessage(SL0034E, currentFileName, line[2].line));
				throwCompileMessage(CompileMessage(SL0025E, currentFileName, line[2].line));
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0013E, currentFileName, line[2].line));
				continue;
			}

			if (!vec_check_index(line, k + 1)) {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[k].line));
				continue;
			}

			if (line[k + 1].value == "=") {
				auto result = getExpression(line, k + 2, 0);
				if (!get<bool>(result)) continue;
				Constructor* variable_init = get<Constructor*>(result);
				variable_init->setName("variable_init");
				constant_variable_declear->addProperty(variable_init);
				function_body->addProperty(constant_variable_declear);
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[k + 1].line));
				continue;
			}
		}
		else if (line[0].value == "var") { // declear variable
			Constructor* variable_declear = new Constructor();
			variable_declear->setName("variable_decl_l");
			variable_declear->addProperty("isVal", "0");

			if (!vec_check_index(line, 1)) {
				throwCompileMessage(CompileMessage(SL0016E, currentFileName, line[0].line));
				continue;
			}

			// get type
			if ((line[1].type != TokenType::KEYWORD) && (line[1].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, line[i].line));
				continue;
			}

			if (!vec_check_index(line, 2)) {
				throwCompileMessage(CompileMessage(SL0012E, currentFileName, line[1].line));
				continue;
			}

			int k;

			// get name
			if (line[2].type == TokenType::IDENTIFIER) {
				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[2].line));
					continue;
				}

				int reference_offset = 0;

				if (line[3].value == "&") {
					variable_declear->addProperty("type", tokens[i + 1].value + "&");
					reference_offset = 1;
				}
				else {
					variable_declear->addProperty("type", line[1].value);
				}

				variable_declear->addProperty("nullable", "0");
				variable_declear->addProperty("name", line[2].value);

				if (!vec_check_index(line, 3 + reference_offset)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[2 + reference_offset].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[3 + reference_offset].value == ":") {
					throwCompileMessage(CompileMessage(SL0015E, currentFileName, line[3 + reference_offset].line));
					continue;
				}

				k = 2 + reference_offset;
			}
			else if (line[2].value == "?") {
				variable_declear->addProperty("nullable", "1");

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0016E, currentFileName, line[2].line));
					continue;
				}

				int reference_offset = 0;

				if (line[3].value == "&") {
					variable_declear->addProperty("type", line[1].value + "&");
					reference_offset = 1;
				}
				else {
					variable_declear->addProperty("type", line[1].value);
				}

				if (line[3 + reference_offset].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, line[3 + reference_offset].line));
					continue;
				}
				variable_declear->addProperty("name", line[3 + reference_offset].value);

				if (!vec_check_index(line, 4 + reference_offset)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[3 + reference_offset].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[4 + reference_offset].value == ":") {
					throwCompileMessage(CompileMessage(SL0015E, currentFileName, line[4 + reference_offset].line));
					continue;
				}

				k = 3 + reference_offset;
			}
			else if (line[2].value == "&") {
				variable_declear->addProperty("nullable", "0");
				variable_declear->addProperty("type", line[1].value + "&");

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0016E, currentFileName, line[2].line));
					continue;
				}

				if (line[3].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013E, currentFileName, tokens[3].line));
					continue;
				}
				variable_declear->addProperty("name", line[3].value);

				if (!vec_check_index(line, 4)) {
					throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[2].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[4].value == ":") {
					throwCompileMessage(CompileMessage(SL0015E, currentFileName, line[4].line));
					continue;
				}

				k = 3;
			}
			else if (line[2].value == "=") {
				variable_declear->addProperty("type", "auto");
				variable_declear->addProperty("name", line[1].value);

				k = 1;
			}
			else if (line[2].value == ";") {
				variable_declear->addProperty("name", line[1].value);
				throwCompileMessage(CompileMessage(SL0034E, currentFileName, line[2].line));
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0013E, currentFileName, line[2].line));
				continue;
			}

			if (!vec_check_index(line, k + 1)) {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[k].line));
				continue;
			}

			if (line[k + 1].value == ";") {
				function_body->addProperty(variable_declear);
				continue;
			}
			else if (line[k + 1].value == "=") {
				auto result = getExpression(line, k + 2, 0);
				if (!get<bool>(result)) continue;
				Constructor* variable_init = get<Constructor*>(result);
				variable_init->setName("variable_init");
				variable_declear->addProperty(variable_init);
				function_body->addProperty(variable_declear);
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[k + 1].line));
				continue;
			}
		}
		else if (line[0].value == "return") {
			if (!vec_check_index(line, 1)) {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, line[0].line));
				continue;
			}

			Constructor* return_ = new Constructor();
			return_->setName("return");

			if (line[1].value == ";") {
				return_->addProperty("value", "");
				function_body->addProperty(return_);
				continue;
			}

			auto result = getExpression(line, 1, 0);
			if (!get<bool>(result)) continue;
			Constructor* return_val_expression = get<Constructor*>(result);
			return_val_expression->setName("value");
			return_->addProperty(return_val_expression);
			function_body->addProperty(return_);
			continue;
		}

		if (line.back().value != ";") {
			tuple<Constructor*, bool> expression = getExpression(line, 0, 0);
			if (!get<bool>(expression)) continue;

			function_body->addProperty(get<Constructor*>(expression));
			continue;
		}

		continue;
	}

	return function_body;
}

tuple<Constructor*, bool> SlentCompiler::getExpression(vector<Token> line, int start_index, int depth) {
	Constructor* expression = new Constructor();
	expression->setName("expression");

	for (int i = start_index; i < ((depth == 0) ? line.size() : findBracketClose(line, start_index, 1)); i++) {
		function<tuple<Constructor*, bool>(bool)> getReference;
		getReference = [line, &i, this, &getReference](bool origin) -> tuple<Constructor*, bool> {
			if (line[i].type != TokenType::IDENTIFIER) return make_tuple(new Constructor(), false);

			Constructor* reference = new Constructor();

			if (origin) {
				reference->setName("reference");
				auto result = getReference(false);
				Constructor* reference_detail = get<Constructor*>(result);
				reference->addProperty(reference_detail);
				return make_tuple(reference, true);
			}

			reference->setName(line[i].value);

			if (vec_check_index(line, i + 1)) {
				if (line[i + 1].value == ".") {
					if (vec_check_index(line, i + 2) && (line[i + 2].type == TokenType::IDENTIFIER)) {
						i = i + 2;
						auto result = getReference(false);
						if (!get<bool>(result)) {
							return make_tuple(new Constructor(), false);
						}
						Constructor* access_detail = get<Constructor*>(result);
						Constructor* member_access = new Constructor();
						member_access->setName("member_access");
						member_access->addProperty(access_detail);
						reference->addProperty(member_access);
					}
					else {
						throwCompileMessage(CompileMessage(SL0018E, currentFileName, i + 1));
						return make_tuple(new Constructor(), false);
					}
				}
				else {
					return make_tuple(reference, true);
				}
			}
			else {
				throwCompileMessage(CompileMessage(SL0018E, currentFileName, i));
				return make_tuple(new Constructor(), false);
			}

			return make_tuple(reference, true);
		};

		if (line[i].type == TokenType::IDENTIFIER) {
			if (line.size() <= (i + 1)) return make_tuple(new Constructor(), false);

			// function call
			if (line[i + 1].value == "(") {
				Constructor* function_call = new Constructor();
				function_call->setName("function_call");
				function_call->addProperty("func_name", line[i].value);
				tuple<Constructor*, bool> get_parameter = getExpression(line, i + 2, depth + 1);
				if (!get<bool>(get_parameter)) { // conversion of lower depth failed
					i = findBracketClose(line, i + 1, 0);
					continue;
				}
				if (findBracketClose(line, i + 1, 0) == -1) {
					throwCompileMessage(CompileMessage(SL0027E, currentFileName, line[i + 1].line));
					return make_tuple(new Constructor(), false);
				}
				Constructor* parameter_constructor = get<Constructor*>(get_parameter);
				parameter_constructor->setName("parameters");
				function_call->addProperty(parameter_constructor);
				expression->addProperty(function_call);
				i = findBracketClose(line, i + 1, 0);
				continue;
			}

			auto result = getReference(true);
			if (!get<bool>(result)) continue;
			expression->addProperty(get<Constructor*>(result));
			continue;
		}

		if ((line[i].type == TokenType::CONSTANT) || line[i].type == TokenType::LITERAL) {
			auto result = getReference(true);
			if (!get<bool>(result)) continue;
			expression->addProperty(get<Constructor*>(result));
			continue;
		}

		if (line[i].type == TokenType::OPERATOR) {
			const vector<string> assignment_operators = { "=", "+=", "-=", "*=", "/=", "%=" };
			const vector<string> relational_operators = { "==", "!=", "<", ">", "<=", ">=" };

			auto getOperation = [line, &i, this, &expression, &depth]() -> tuple<Constructor*, bool> {
				Constructor* operation = new Constructor();
				operation->setName("operation");
				operation->addProperty("type", line[i].value);
				Constructor* left = new Constructor();
				left->setName("left");
				Constructor* right = new Constructor();
				right->setName("right");

				// left side of operator
				for (int j = 0; j < expression->getProperties().size(); j++) {
					left->addProperty(expression->getProperties()[j]);
				}

				// right side of operator
				tuple<Constructor*, bool> right_expression_result = getExpression(line, i + 1, depth);
				if (!get<bool>(right_expression_result)) {
					return make_tuple(new Constructor(), false);
				}
				for (int j = 0; j < get<Constructor*>(right_expression_result)->getProperties().size(); j++) {
					right->addProperty(get<Constructor*>(right_expression_result)->getProperties()[j]);
				}

				operation->addProperty(left);
				operation->addProperty(right);
				return make_tuple(operation, true);
			};

			if (find(assignment_operators.begin(), assignment_operators.end(), line[i].value) != assignment_operators.end()) {
				if (depth != 0) {
					throwCompileMessage(CompileMessage(SL0024E, currentFileName, line[i].line));
					return make_tuple(new Constructor(), false);
				}

				tuple<Constructor*, bool> getOperation_result = getOperation();
				if (!get<bool>(getOperation_result)) {
					return make_tuple(new Constructor(), false);
				}
				expression->addProperty(get<Constructor*>(getOperation_result));
				i = findBracketClose(line, i, depth);
				continue;
			}
			if (find(relational_operators.begin(), relational_operators.end(), line[i].value) != relational_operators.end()) {
				tuple<Constructor*, bool> getOperation_result = getOperation();
				if (!get<bool>(getOperation_result)) {
					return make_tuple(new Constructor(), false);
				}
				expression->addProperty(get<Constructor*>(getOperation_result));
				i = findBracketClose(line, i, depth);
				continue;
			}
		}
	}

	return make_tuple(expression, true);
}

tuple<Constructor*, bool> SlentCompiler::getExternalFunction(vector<Token> tokens, int cursor, bool isDynamic) {
	if (isDynamic) {
		if (tokens[cursor].value == "dynamic") {
			if (!vec_check_index(tokens, cursor + 1)) {
				throwCompileMessage(CompileMessage(SL0037E, currentFileName, tokens[cursor].line));
				return make_tuple(new Constructor(), false);
			}

			if (tokens[cursor + 1].value == "extern") {
				if (!vec_check_index(tokens, cursor + 2)) {
					throwCompileMessage(CompileMessage(SL0038E, currentFileName, tokens[cursor + 1].line));
					throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 1].line));
					return make_tuple(new Constructor(), false);
				}

				if (tokens[cursor + 2].type == TokenType::LITERAL) {
					if (!vec_check_index(tokens, cursor + 3)) {
						throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 3].line));
						return make_tuple(new Constructor(), false);
					}

					if (tokens[cursor + 3].value == "func") {
						tuple<Constructor*, bool> externalFunction_result = getFunction(tokens, Scope(cursor + 3, findNextSemicolon(tokens, cursor + 3)), false);
						Constructor* external_function = get<Constructor*>(externalFunction_result);
						external_function->addProperty("extern", tokens[cursor + 2].value);
						return make_tuple(external_function, true);
					}
					else {
						throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 3].line));
						return make_tuple(new Constructor(), false);
					}
				}
				else if (tokens[cursor + 2].value == "func") {
					tuple<Constructor*, bool> externalFunction_result = getFunction(tokens, Scope(cursor + 2, findNextSemicolon(tokens, cursor + 2)), false);
					Constructor* external_function = get<Constructor*>(externalFunction_result);
					external_function->addProperty("extern", "");
					return make_tuple(external_function, true);
				}
				else {
					throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 2].line));
					return make_tuple(new Constructor(), false);
				}
			}
			else {
				throwCompileMessage(CompileMessage(SL0037E, currentFileName, tokens[cursor + 1].line));
				return make_tuple(new Constructor(), false);
			}
		}
	}
	else {
		if (tokens[cursor].value == "extern") {
			if (!vec_check_index(tokens, cursor + 1)) {
				throwCompileMessage(CompileMessage(SL0038E, currentFileName, tokens[cursor].line));
				throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor].line));
				return make_tuple(new Constructor(), false);
			}

			if (tokens[cursor + 1].type == TokenType::LITERAL) {
				if (!vec_check_index(tokens, cursor + 2)) {
					throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 2].line));
					return make_tuple(new Constructor(), false);
				}

				if (tokens[cursor + 2].value == "func") {
					tuple<Constructor*, bool> externalFunction_result = getFunction(tokens, Scope(cursor + 2, findNextSemicolon(tokens, cursor + 2)), false);
					Constructor* external_function = get<Constructor*>(externalFunction_result);
					external_function->addProperty("extern", tokens[cursor + 1].value);
					return make_tuple(external_function, true);
				}
				else {
					throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 2].line));
					return make_tuple(new Constructor(), false);
				}
			}
			else if (tokens[cursor + 1].value == "func") {
				throwCompileMessage(CompileMessage(SL0038E, currentFileName, tokens[cursor + 1].line));
				return make_tuple(new Constructor(), false);
			}
			else {
				throwCompileMessage(CompileMessage(SL0039E, currentFileName, tokens[cursor + 1].line));
				return make_tuple(new Constructor(), false);
			}
		}
	}
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
	int braces = current_brace;
	for (int i = cursor; i < tokens.size(); i++) {
		if (tokens[i].value == "{") {
			braces++;
		}
		else if (tokens[i].value == "}") {
			braces--;
		}

		if (braces == 0) {
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

string SlentCompiler::bytecode(Constructor* ast) {
	vector<string> commands;



	string bytecode = "";
	for (auto& command : commands) {
		bytecode += command;
	}
	return bytecode;
}

void SlentCompiler::optimize() {

}

void SlentCompiler::throwCompileMessage(CompileMessage compileMessage) {
	string type;
	int color;
	switch (compileMessage.type) {
		case MessageType::ERROR:
			type = "Error";
			color = RED;
			_error = true;
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

/*
void SlentCompiler::compile_file(string file_name, string code) {
	currentFileName = file_name;

#ifdef _DEBUG
	try {
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
*/

void SlentCompiler::AddFile(string file_name, string code) {
	code_files.push_back(make_tuple(file_name, code));
}

void SlentCompiler::Compile() {
	_error = false;

	vector<string> no_comment_codes;

	Constructor* module_tree = new Constructor();
	bool first_module = true;

	for (auto& [filename, code] : code_files) {
		currentFileName = filename;

		cout << colorString(string("[Compile] ").append(filename), CYAN_LIGHT) << endl;

		if (compilerSetting.trace_compile_logs) {
			cout << "[ source code ]" << endl;
			vector<string> split_code = split(code, '\n');
			int maxNumLength = to_string(split_code.size()).length();
			for (int i = 1; i <= split_code.size(); i++) {
				int numLength = to_string(i).length();
				int paddingLength = maxNumLength - numLength;
				string padding = "";
				for (int k = 0; k < paddingLength; k++) {
					padding.append(" ");
				}
				cout << padding << i << "| " << split_code[i - 1] << endl;
			}
			cout << endl;
		}

		regex commentRegex("//.*");
		string no_comment_code = regex_replace(code, commentRegex, "");
		no_comment_codes.push_back(no_comment_code);

		Constructor* module_tree_file = getModuleTree(code);
		if (first_module) {
			module_tree = Constructor::copy(module_tree_file);
			first_module = false;
		}
		else {
			tuple<Constructor*, bool> module_tree_merge_result = Constructor::merge(module_tree, module_tree_file);
			Constructor* module_tree = get<Constructor*>(module_tree_merge_result);
		}

		if (compilerSetting.trace_compile_logs) {
			cout << "[ module_tree ]" << endl << module_tree->toPrettyString() << endl << endl;
		}
	}

	vector<Macro> macros = getMacros(module_tree, no_comment_codes);

	if (compilerSetting.trace_compile_logs) {
		cout << "[ macros ]" << endl;
		for (auto& macro : macros) {
			cout << macro.toString() << endl;
		}
		cout << endl;
	}

	for (int i = 0; i < no_comment_codes.size(); i++) {
		currentFileName = get<0>(code_files[i]);
		string preprocessed_code = preprocess(module_tree, no_comment_codes[i], macros);
		if (compilerSetting.trace_compile_logs) {
			cout << "[ preprocessed_code ]" << endl;
			vector<string> split_preprocessed_code = split(preprocessed_code, '\n');
			int maxNumLength = to_string(split_preprocessed_code.size()).length();
			for (int j = 1; j <= split_preprocessed_code.size(); j++) {
				int numLength = to_string(j).length();
				int paddingLength = maxNumLength - numLength;
				string padding = "";
				for (int k = 0; k < paddingLength; k++) {
					padding.append(" ");
				}
				cout << padding << j << "| " << split_preprocessed_code[j - 1] << endl;
			}
			cout << endl;
		}

		vector<Token> tokens = lexer(preprocessed_code);

		if (compilerSetting.trace_compile_logs) {
			cout << "[ tokens ]" << endl;

			int maxNumLength = to_string(tokens.size()).length();
			int index = 1;
			// print tokens
			for (const auto& token : tokens) {
				int numLength = to_string(index).length();
				int paddingLength = maxNumLength - numLength;
				string padding = "";
				for (int k = 0; k < paddingLength; k++) {
					padding.append(" ");
				}
				cout << padding << index << "| ";
				index++;
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
		}

		Constructor* AST = parser(tokens);
		
		if (compilerSetting.trace_compile_logs) {
			cout << "[ AST ]" << endl;
			cout << AST->toPrettyString() << endl;
		}
	}
}

bool SlentCompiler::compileError() {
	return _error;
}