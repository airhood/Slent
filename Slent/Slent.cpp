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
using namespace Slent;

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

string Slent::colorString(string str, int color) {
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
			if (find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
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
			else if (regex_match(matched, regex("\"[^\"]*\""))) {
				tokens.push_back(Token(TokenType::LITERAL, matched, i));
			}
			else if (regex_match(matched, regex(R"(==|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|=|\+|\-|\*|\/|<|>|\|\||&&|!)"))) {
				tokens.push_back(Token(TokenType::OPERATOR, matched, i));
			}
			else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|::|:|;|<|>|\.|\,|~|#)"))) {
				tokens.push_back(Token(TokenType::SPECIAL_SYMBOL, matched, i));
			}
			else {
				throwCompileMessage(CompileMessage(SL0000(matched), currentFileName, i));
			}
			code_split[i] = match.suffix().str();
		}
	}

	return tokens;
}

string SlentCompiler::preprocess(Constructor module_tree, string code, vector<Macro> macros) {
	vector<Token> tokens = getPreprocessorTokens(code);
	vector<string> imports = getImports(module_tree, tokens);
	string processed_code = runMacros(code, macros);

	return processed_code;
}

vector<string> SlentCompiler::getImports(Constructor module_tree, vector<Token> tokens) {
	vector<string> imports;

	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].value == "import") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0006, currentFileName, tokens[i + 5].line));
				break;
			}

			string import_temp = "";

			Constructor current_depth_module = module_tree;
			int j = i + 1;
			while (true) {
				if (tokens[j].type == TokenType::IDENTIFIER) {
					if (!current_depth_module.propertyExist(tokens[j].value)) {
						throwCompileMessage(CompileMessage(SL0033, currentFileName, tokens[j].line));
						goto err_1;
					}
					current_depth_module = current_depth_module.getProperty(tokens[j].value);
					import_temp.append(tokens[j].value.append("::"));
				}
				else {
					throwCompileMessage(CompileMessage(SL0033, currentFileName, tokens[j].line));
					goto err_1;
				}
				if (!vec_check_index(tokens, j + 1)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[j + 1].line));
					goto err_1;
				}
				if (tokens[j + 1].value != "::") {
					if (tokens[j + 1].value != ";") {
						throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[j + 1].line));
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

vector<Macro> SlentCompiler::getMacros(Constructor module_tree, vector<string> codes) {
	vector<Macro> macros;
	for (auto& code : codes) {
		vector<string> lines = split(code, '\n');
		vector<Token> tokens = getPreprocessorTokens(code);

		for (int i = 0; i < tokens.size(); i++) {
			if ((tokens[i].value == "$") && (!vec_check_index(tokens, i + 1))) {
				throwCompileMessage(CompileMessage(SL0001, currentFileName, tokens[i + 1].line));
				break;
			}

			if (!vec_check_index(tokens, i + 1)) break;

			if ((tokens[i].value == "$") && (tokens[i + 1].value == "macro_def")) {
				if (tokens[i - 1].line == tokens[i].line) {
					throwCompileMessage(CompileMessage(SL0001, currentFileName, tokens[i].line));
					continue;
				}

				Macro macro = Macro();
				macro.name = tokens[i + 1].value;

				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0002, currentFileName, tokens[i].line));
					break;
				}
				if ((tokens[i + 1].line != tokens[i + 2].line) || (tokens[i + 2].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0002, currentFileName, tokens[i].line));
					continue;
				}


				if (!vec_check_index(tokens, i + 5)) {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 2].line));
					break;
				}
				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 2].line));
					continue;
				}
				if (tokens[i + 4].value != "$") {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (tokens[i + 5].value == "(") {
					i = i + 5;
				}
				else if (tokens[i + 5].value == "macro_module") {
					if (!vec_check_index(tokens, i + 6)) {
						throwCompileMessage(CompileMessage(SL0006, currentFileName, tokens[i + 5].line));
						break;
					}

					Constructor current_depth_module = module_tree;
					int j = i + 6;
					while (true) {
						if (tokens[j].type == TokenType::IDENTIFIER) {
							if (!current_depth_module.propertyExist(tokens[j].value)) {
								throwCompileMessage(CompileMessage(SL0033, currentFileName, tokens[j].line));
								goto err_2;
							}
							current_depth_module = current_depth_module.getProperty(tokens[j].value);
							macro.macro_module.append(tokens[j].value.append("::"));
						}
						else {
							throwCompileMessage(CompileMessage(SL0033, currentFileName, tokens[j].line));
							goto err_2;
						}
						if (!vec_check_index(tokens, j + 1)) {
							j = j + 1;
							break;
						}
						if (tokens[j + 1].value != "::") {
							if (tokens[j].line == tokens[j + 1].line) {
								throwCompileMessage(CompileMessage(SL0033, currentFileName, tokens[j + 1].line));
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
						throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 2].line));
						break;
					}

					if (tokens[j].value != "$") {
						throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 2].line));
						continue;
					}

					if (tokens[j + 1].value == "(") {
						i = j + 1;
					}
					else {
						throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 7].line));
						continue;
					}
				}
				else {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i + 4].line));
					continue;
				}

				for (int j = i + 1; j < findBracketClose(tokens, i + 4, 1); j++) {
					if (tokens[j].value == "~") {
						if (!vec_check_index(tokens, j + 1)) {
							throwCompileMessage(CompileMessage(SL0026, currentFileName, tokens[j].line));
							break;
						}

						if (tokens[j + 1].type == TokenType::IDENTIFIER) {
							if (tokens[j + 1].value == "null") {
								throwCompileMessage(CompileMessage(SL0005, currentFileName, tokens[j + 1].line));
							}
							else {
								macro.parameters.push_back(tokens[j + 1].value);
							}

							if (!vec_check_index(tokens, j + 2)) {
								throwCompileMessage(CompileMessage(SL0027, currentFileName, tokens[j].line));
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
								throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[j].line));
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
							throwCompileMessage(CompileMessage(SL0005, currentFileName, tokens[j].line));

							if (!vec_check_index(tokens, j + 2)) {
								throwCompileMessage(CompileMessage(SL0027, currentFileName, tokens[j + 1].line));
								break;
							}

							if (tokens[j + 2].value == ",") {
								j = j + 2;
								continue;
							}
							else {
								throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[j + 1].line));
								j = t_find_next(tokens, j + 2, vector<string> {","});
								continue;
							}
						}
					}
					else if (tokens[j].type == TokenType::IDENTIFIER) {
						macro.parameters.push_back(tokens[j].value);

						if (!vec_check_index(tokens, j + 1)) {
							throwCompileMessage(CompileMessage(SL0027, currentFileName, tokens[j].line));
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
							throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[j].line));
							j = t_find_next(tokens, j + 2, vector<string> {","});
							continue;
						}
						continue;
					}
					else {
						throwCompileMessage(CompileMessage(SL0005, currentFileName, tokens[j].line));
					}
				}
				i = findBracketClose(tokens, i + 3, 1) + 1;

				if (!vec_check_index(tokens, i)) {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i - 1].line));
					break;
				}

				if (tokens[i].value != "=>") {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i - 1].line));
					continue;
				}
				if (tokens[i + 1].value != "{") {
					throwCompileMessage(CompileMessage(SL0004, currentFileName, tokens[i].line));
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
							throwCompileMessage(CompileMessage(SL0005, currentFileName, getLineNum(m_code, i)));
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
							throwCompileMessage(CompileMessage(SL0023, currentFileName, getLineNum(m_code, i)));
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

Constructor SlentCompiler::getModuleTree(std::string code) {
	Constructor root = Constructor();
	root.setName("root");
	vector<Token> tokens = getPreprocessorTokens(code);
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].value == "export") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0028, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].value != "module") {
				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0029, currentFileName, tokens[i + 1].line));
					break;
				}

				if (tokens[i + 2].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0030, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 2].line));
					break;
				}

				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 4)) {
					throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (findBraceClose(tokens, i + 4, 1) == -1) {
					throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[tokens.size() - 1].line));
					continue;
				}

				Constructor sub_modules = getSubModuleTree(code, Scope(i + 4, findBraceClose(tokens, i + 4, 1) - 1));
				sub_modules.setName(tokens[i + 2].value);
				root.addProperty(sub_modules);
				root.addProperty("export", "1");
			}
		}
		else if (tokens[i].value == "module") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0030, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0030, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 2].value != "{") {
				throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 3)) {
				throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[i + 2].line));
				continue;
			}

			if (findBraceClose(tokens, i + 3, 1) == -1) {
				throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[tokens.size() - 1].line));
				continue;
			}

			Constructor sub_modules = getSubModuleTree(code, Scope(i + 3, findBraceClose(tokens, i + 3, 1) - 1));
			sub_modules.setName(tokens[i + 2].value);
			root.addProperty(sub_modules);
			root.addProperty("export", "0");
		}
	}

	return root;
}

Constructor SlentCompiler::getSubModuleTree(string code, Scope scope) {
	Constructor root = Constructor();
	root.setName("sub_root");
	vector<Token> tokens = getPreprocessorTokens(code);
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].value == "export") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0028, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].value != "module") {
				if (!vec_check_index(tokens, i + 2)) {
					throwCompileMessage(CompileMessage(SL0029, currentFileName, tokens[i + 1].line));
					break;
				}

				if (tokens[i + 2].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0030, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 2].line));
					break;
				}

				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (!vec_check_index(tokens, i + 4)) {
					throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (findBraceClose(tokens, i + 4, 1) == -1) {
					throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[tokens.size() - 1].line));
					continue;
				}

				Constructor sub_modules = getSubModuleTree(code, Scope(i + 4, findBraceClose(tokens, i + 4, 1) - 1));
				sub_modules.setName(tokens[i + 2].value);
				root.addProperty(sub_modules);
				root.addProperty("export", "1");
			}
		}
		else if (tokens[i].value == "module") {
			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0030, currentFileName, tokens[i].line));
				break;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0030, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 1].line));
				break;
			}

			if (tokens[i + 2].value != "{") {
				throwCompileMessage(CompileMessage(SL0031, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (!vec_check_index(tokens, i + 3)) {
				throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[i + 2].line));
				continue;
			}

			if (findBraceClose(tokens, i + 3, 1) == -1) {
				throwCompileMessage(CompileMessage(SL0032, currentFileName, tokens[tokens.size() - 1].line));
				continue;
			}

			Constructor sub_modules = getSubModuleTree(code, Scope(i + 3, findBraceClose(tokens, i + 3, 1) - 1));
			sub_modules.setName(tokens[i + 2].value);
			root.addProperty(sub_modules);
			root.addProperty("export", "0");
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
			else if (regex_match(matched, regex(R"(==|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|=|\+|\-|\*|\/|<|>|\|\||&&|!)"))) {
				tokens.push_back(Token(TokenType::OPERATOR, matched, i));
			}
			else if (regex_match(matched, regex(R"(\(|\)|\{|\}|\[|\]|;|<|>|\.|\,)"))) {
				tokens.push_back(Token(TokenType::SPECIAL_SYMBOL, matched, i));
			}
			else {
				throwCompileMessage(CompileMessage(SL0000(matched), currentFileName, i));
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
		else if (tokens[i].value == "func") {
			int functionBodyEnd = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);

			if (vec_check_index(tokens, functionBodyEnd + 2)) {
				if (tokens[functionBodyEnd + 1].value == ":") {
					if (tokens[functionBodyEnd + 2].value == "public") {

					}
				}
			}

			tuple<Constructor, bool> getFunction_result = getFunction(tokens, Scope(i, findBraceClose(tokens, findBraceClose(tokens, i, -1), 0) - 1));
			if (!get<bool>(getFunction_result)) {
				i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
				continue;
			}
			Constructor function = get<Constructor>(getFunction_result);
			root.addProperty(function);
			i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
			continue;
		}
		else if (tokens[i].value == "class") {
			tuple<Constructor, int, bool> getClass_result = getClass(tokens, i);
			i = get<int>(getClass_result);
			if (!get<bool>(getClass_result)) {
				i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
				continue;
			}
			Constructor class_define = get<Constructor>(getClass_result);
			root.addProperty(class_define);
			i = findBraceClose(tokens, findBraceClose(tokens, i, -1), 0);
			continue;
		}
		else {
			throwCompileMessage(CompileMessage(SL0035, currentFileName, tokens[i].line));
			continue;
		}
	}

	return root;
}

tuple<Constructor, bool> SlentCompiler::getFunction(vector<Token> tokens, Scope scope) {
	int i = scope.start;
	if (tokens[i].value == "func") {
		if ((i + 1) > scope.end) {
			throwCompileMessage(CompileMessage(SL0019, currentFileName, tokens[i].line));
			return make_tuple(Constructor(), false);
		}

		if (tokens[i + 1].type != TokenType::IDENTIFIER) {
			throwCompileMessage(CompileMessage(SL0020, currentFileName, tokens[i + 1].line));
			return make_tuple(Constructor(), false);
		}

		Constructor function = Constructor();
		function.setName("function");
		function.addProperty("name", tokens[i + 1].value);

		if ((i + 2) > scope.end) {
			throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i + 1].line));
			return make_tuple(Constructor(), false);
		}

		if (tokens[i + 2].value != "(") {
			throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i + 2].line));
			return make_tuple(Constructor(), false);
		}
		i = i + 2;

		// get function parameters
		Constructor parameters = Constructor();
		parameters.setName("parameters");

		int k = 0;

		bool error = false;

		if (tokens[i + 1].value == ")") {
			function.addProperty("parameters", "");
			goto get_parameter_finished;
		}

		for (int j = i + 1; j < findBracketClose(tokens, i + 1, 1); j++) {
			if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[j].line));
				j = t_find_next(tokens, j, vector<string> {","});
				error = true;
				continue;
			}

			if ((j + 1) >= findBracketClose(tokens, i + 1, 1)) {
				break;
			}

			if (tokens[j + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[j + 1].line));
				i = t_find_next(tokens, j + 1, vector<string> {","});
				continue;
			}

			if ((j + 2) >= findBracketClose(tokens, i + 1, 1)) {
				break;
			}

			if (tokens[j + 2].value != ",") {
				throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[j + 1].line));
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

		if (error) return make_tuple(Constructor(), false);

		function.addProperty(parameters);
	get_parameter_finished:

		i = t_find_next(tokens, i + 1, vector<string> {")"});

		if ((i + 2) > scope.end) {
			throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i].line));
			return make_tuple(Constructor(), false);
		}

		if (tokens[i + 1].value != "{") {
			if (tokens[i + 1].value == "->") {
				if ((i + 3) > scope.end) {
					throwCompileMessage(CompileMessage(SL0021, currentFileName, tokens[i + 2].line));
					return make_tuple(Constructor(), false);
				}

				if ((tokens[i + 2].type != TokenType::KEYWORD) && (tokens[i + 2].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[i + 3].line));
					return make_tuple(Constructor(), false);
				}
				function.addProperty("return_type", tokens[i + 2].value);
				if (tokens[i + 3].value != "{") {
					throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i + 3].line));
				}
				i = i + 3;
				goto return_type_done;
			}
			throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i].line));
			return make_tuple(Constructor(), false);
		}
		// tokens[i + 1].value == "{" => true
		i = i + 1;

		function.addProperty("return_type", "");

	return_type_done:

		Constructor function_body = getFunctionBody(tokens, Scope(i + 1, findBraceClose(tokens, i + 2, 1) - 1));
		function.addProperty(function_body);

		return make_tuple(function, true);
	}
}

tuple<Constructor, int, bool> SlentCompiler::getClass(vector<Token> tokens, int cursor) {
	Constructor class_define = Constructor();
	class_define.setName("class");
	if (!vec_check_index(tokens, cursor + 1)) {
	}
	if (tokens[cursor + 1].type != TokenType::IDENTIFIER) {
		throwCompileMessage(CompileMessage(SL0009, currentFileName, tokens[cursor + 1].line));
		return make_tuple(class_define, cursor, false);
	}
	class_define.addProperty("name", tokens[cursor + 1].value);
	if (tokens[cursor + 2].value != "{") {
		throwCompileMessage(CompileMessage(SL0010, currentFileName, tokens[cursor + 1].line));
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

vector<Constructor>SlentCompiler::getClassConstructors(vector<Token> tokens, Scope scope) {
	vector<Constructor> class_constructors;
	for (int i = scope.start; i <= scope.end; i++) {
		if (tokens[i].value == "construct") {
			Constructor constructor = Constructor();
			constructor.setName("constructor");

			if (tokens[i + 1].value != "(") {
				throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i + 4].line));
				continue;
			}

			Constructor parameters = Constructor();
			parameters.setName("parameters");
			int k = 0;
			for (int j = i + 2; j < findBracketClose(tokens, i + 2, 1); j++) {
				if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[j].line));
					i = t_find_next(tokens, j, vector<string> {","});
					continue;
				}
				if (tokens[j + 1].type != TokenType::IDENTIFIER) {
					if ((j + 1) >= findBracketClose(tokens, i + 2, 1)) {
						break;
					}
					throwCompileMessage(CompileMessage(SL0013, currentFileName, tokens[j + 1].line));
					i = t_find_next(tokens, j + 1, vector<string> {","});
					continue;
				}
				if (tokens[j + 2].value != ",") {
					if ((j + 2) >= findBracketClose(tokens, i + 2, 1)) {
						break;
					}
					throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[j + 1].line));
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
				throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i + 1].line));
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

			if (!vec_check_index(tokens, i + 1)) {
				throwCompileMessage(CompileMessage(SL0016, currentFileName, tokens[i].line));
				continue;
			}

			// get type
			if ((tokens[i + 1].type != TokenType::KEYWORD) && (tokens[i + 1].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[i + 1].line));
				i = findNextSemicolon(tokens, i + 1);
				continue;
			}

			if (!vec_check_index(tokens, i + 2)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[i + 1].line));
				continue;
			}

			int k;

			// get name
			if (tokens[i + 2].type != TokenType::IDENTIFIER) {
				variable.addProperty("type", tokens[i + 1].value);
				variable.addProperty("nullable", "0");
				variable.addProperty("name", tokens[i + 2].value);

				int offset = 0;

				if (!vec_check_index(tokens, i + 3)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[i + 2].line));
					continue;
				}
				// get access_modifier
				if (tokens[i + 3].value == ":") {
					if (!vec_check_index(tokens, i + 4)) {
						throwCompileMessage(CompileMessage(SL0036, currentFileName, tokens[i + 3].line));
						continue;
					}

					if (tokens[i + 4].value == "public") {
						variable.addProperty("access_modifier", "public");
						offset = 2;
					}
					else if (tokens[i + 4].value == "protected") {
						variable.addProperty("access_modifier", "protected");
						offset = 2;
					}
					else if (tokens[i + 4].value == "private") {
						variable.addProperty("access_modifier", "private");
						offset = 2;
					}
					else {
						throwCompileMessage(CompileMessage(SL0036, currentFileName, tokens[i + 4].line));
						i = findNextSemicolon(tokens, i + 4);
						continue;
					}
				}

				k = i + 2 + offset;
			}
			else if (tokens[i + 2].value == "?") {
				variable.addProperty("nullable", "1");

				if (!vec_check_index(tokens, 3)) {
					throwCompileMessage(CompileMessage(SL0016, currentFileName, tokens[i + 2].line));
					continue;
				}

				if (tokens[i + 3].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013, currentFileName, tokens[i + 3].line));
					i = findNextSemicolon(tokens, i + 3);
					continue;
				}
				variable.addProperty("name", tokens[i + 3].value);

				int offset = 0;

				if (!vec_check_index(tokens, i + 4)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[i + 3].line));
					continue;
				}

				if (tokens[i + 4].value == ":") {
					if (!vec_check_index(tokens, i + 5)) {
						throwCompileMessage(CompileMessage(SL0036, currentFileName, tokens[i + 4].line));
						continue;
					}

					if (tokens[i + 5].value == "public") {
						variable.addProperty("access_modifier", "public");
						offset = 2;
					}
					else if (tokens[i + 5].value == "protected") {
						variable.addProperty("access_modifier", "protected");
						offset = 2;
					}
					else if (tokens[i + 5].value == "private") {
						variable.addProperty("access_modifier", "private");
						offset = 2;
					}
					else {
						throwCompileMessage(CompileMessage(SL0036, currentFileName, tokens[i + 5].line));
						i = findNextSemicolon(tokens, i + 5);
						continue;
					}
				}

				k = i + 3 + offset;
			}
			else if (tokens[i + 2].value == "=") {
				variable.addProperty("type", "auto");
				variable.addProperty("name", tokens[i + 1].value);

				k = 1;
			}
			else if (tokens[i + 2].value == ";") {
				variable.addProperty("name", tokens[i + 1].value);
				throwCompileMessage(CompileMessage(SL0034, currentFileName, tokens[i + 2].line));
				i = i + 2;
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0013, currentFileName, tokens[i + 2].line));
				i = findNextSemicolon(tokens, i + 2);
				continue;
			}

			if (!vec_check_index(tokens, k + 1)) {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[k].line));
				continue;
			}

			if (tokens[k + 1].value == ";") {
				class_variables.push_back(variable);
				i = k + 1;
			}
			else if (tokens[k + 1].value == "=") {
				auto result = getExpression(tokens, i + 2, 0);
				if (!get<bool>(result)) continue;
				Constructor variable_init = get<Constructor>(result);
				variable_init.setName("variable_init");
				variable.addProperty(variable_init);
				class_variables.push_back(variable);
				i = findNextSemicolon(tokens, k + 1);
			}
			else {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, tokens[i].line));
				i = findNextSemicolon(tokens, k + 1);
				continue;
			}
		}
	}

	return class_variables;
}

vector<Constructor> SlentCompiler::getClassFunctions(vector<Token> tokens, Scope scope) {
	vector<Constructor> class_functions;
	for (int i = scope.start; i <= scope.end; i++) {
		if (tokens[i].value == "func") {
			if ((i + 1) > scope.end) {
				throwCompileMessage(CompileMessage(SL0019, currentFileName, tokens[i].line));
				continue;
			}

			if (tokens[i + 1].type != TokenType::IDENTIFIER) {
				throwCompileMessage(CompileMessage(SL0020, currentFileName, tokens[i + 1].line));
				continue;
			}

			Constructor function = Constructor();
			function.setName("function");
			function.addProperty("name", tokens[i + 1].value);

			if ((i + 2) > scope.end) {
				throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i + 1].line));
				continue;
			}

			if (tokens[i + 2].value != "(") {
				throwCompileMessage(CompileMessage(SL0011, currentFileName, tokens[i + 2].line));
				continue;
			}
			i = i + 2;

			// get function parameters
			Constructor parameters = Constructor();
			parameters.setName("parameters");

			int k = 0;

			bool error = false;

			if (tokens[i + 1].value == ")") {
				function.addProperty("parameters", "");
				goto get_parameter_finished;
			}

			for (int j = i + 1; j < findBracketClose(tokens, i + 1, 1); j++) {
				if ((tokens[j].type != TokenType::KEYWORD) && (tokens[j].type != TokenType::IDENTIFIER)) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[j].line));
					j = t_find_next(tokens, j, vector<string> {","});
					error = true;
					continue;
				}

				if ((j + 1) >= findBracketClose(tokens, i + 1, 1)) {
					break;
				}

				if (tokens[j + 1].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[j + 1].line));
					j = t_find_next(tokens, j + 1, vector<string> {","});
					error = true;
					continue;
				}

				if ((j + 2) >= findBracketClose(tokens, i + 1, 1)) {
					break;
				}

				if (tokens[j + 2].value != ",") {
					throwCompileMessage(CompileMessage(SL0014, currentFileName, tokens[j + 1].line));
					j = t_find_next(tokens, j + 2, vector<string> {","});
					error = true;
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
			
			if (error) continue;

			function.addProperty(parameters);
		get_parameter_finished:

			i = t_find_next(tokens, i + 1, vector<string> {")"});

			if ((i + 2) > scope.end) {
				throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i].line));
				continue;
			}

			if (tokens[i + 1].value != "{") {
				if (tokens[i + 1].value == "->") {
					if ((i + 3) > scope.end) {
						throwCompileMessage(CompileMessage(SL0021, currentFileName, tokens[i + 2].line));
						continue;
					}

					if ((tokens[i + 2].type != TokenType::KEYWORD) && (tokens[i + 2].type != TokenType::IDENTIFIER)) {
						throwCompileMessage(CompileMessage(SL0012, currentFileName, tokens[i + 3].line));
						continue;
					}
					function.addProperty("return_type", tokens[i + 2].value);
					if (tokens[i + 3].value != "{") {
						throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i + 3].line));
					}
					i = i + 3;
					goto return_type_done;
				}
				throwCompileMessage(CompileMessage(SL0022, currentFileName, tokens[i].line));
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

		if (line[0].value == "val") { // declear value
			Constructor constant_variable_declear = Constructor();
			constant_variable_declear.setName("variable_decl_l");
			constant_variable_declear.addProperty("isVal", "1");

			if (!vec_check_index(line, 1)) {
				throwCompileMessage(CompileMessage(SL0016, currentFileName, line[0].line));
				continue;
			}

			// get type
			if ((line[1].type != TokenType::KEYWORD) && (line[1].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, line[1].line));
				continue;
			}

			if (!vec_check_index(line, 2)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, line[1].line));
				continue;
			}

			int k;

			// get name
			if (line[2].type == TokenType::IDENTIFIER) {
				constant_variable_declear.addProperty("type", line[1].value);
				constant_variable_declear.addProperty("nullable", "0");
				constant_variable_declear.addProperty("name", line[2].value);

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, line[2].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[3].value == ":") {
					throwCompileMessage(CompileMessage(SL0015, currentFileName, line[3].line));
					continue;
				}

				k = 2;
			}
			else if (line[2].value == "?") {
				constant_variable_declear.addProperty("nullable", "1");

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0016, currentFileName, line[2].line));
					continue;
				}

				if (line[3].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013, currentFileName, line[3].line));
					continue;
				}
				constant_variable_declear.addProperty("name", line[3].value);

				if (!vec_check_index(line, 4)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, line[3].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[4].value == ":") {
					throwCompileMessage(CompileMessage(SL0015, currentFileName, line[4].line));
					continue;
				}

				k = 3;
			}
			else if (line[2].value == "=") {
				constant_variable_declear.addProperty("tpye", "auto");
				constant_variable_declear.addProperty("name", line[1].value);

				k = 1;
			}
			else if (line[2].value == ";") {
				constant_variable_declear.addProperty("name", line[1].value);
				throwCompileMessage(CompileMessage(SL0034, currentFileName, line[2].line));
				throwCompileMessage(CompileMessage(SL0025, currentFileName, line[2].line));
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0013, currentFileName, line[2].line));
				continue;
			}

			if (!vec_check_index(line, k + 1)) {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, line[k].line));
				continue;
			}

			if (line[k + 1].value == "=") {
				auto result = getExpression(line, k + 2, 0);
				if (!get<bool>(result)) continue;
				Constructor variable_init = get<Constructor>(result);
				variable_init.setName("variable_init");
				constant_variable_declear.addProperty(variable_init);
				function_body.addProperty(constant_variable_declear);
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, line[k + 1].line));
				continue;
			}
		}
		else if (line[0].value == "var") {
			Constructor variable_declear = Constructor();
			variable_declear.setName("variable_decl_l");
			variable_declear.addProperty("isVal", "0");

			if (!vec_check_index(line, 1)) {
				throwCompileMessage(CompileMessage(SL0016, currentFileName, line[0].line));
				continue;
			}

			// get type
			if ((line[1].type != TokenType::KEYWORD) && (line[1].type != TokenType::IDENTIFIER)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, line[i].line));
				continue;
			}

			if (!vec_check_index(line, 2)) {
				throwCompileMessage(CompileMessage(SL0012, currentFileName, line[1].line));
				continue;
			}

			int k;

			// get name
			if (line[2].type == TokenType::IDENTIFIER) {
				variable_declear.addProperty("type", line[1].value);
				variable_declear.addProperty("nullable", "0");
				variable_declear.addProperty("name", line[2].value);

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, line[2].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[3].value == ":") {
					throwCompileMessage(CompileMessage(SL0015, currentFileName, line[3].line));
					continue;
				}

				k = 2;
			}
			else if (line[2].value == "?") {
				variable_declear.addProperty("nullable", "1");

				if (!vec_check_index(line, 3)) {
					throwCompileMessage(CompileMessage(SL0016, currentFileName, line[2].line));
					continue;
				}

				if (line[3].type != TokenType::IDENTIFIER) {
					throwCompileMessage(CompileMessage(SL0013, currentFileName, line[3].line));
					continue;
				}
				variable_declear.addProperty("name", line[3].value);

				if (!vec_check_index(line, 4)) {
					throwCompileMessage(CompileMessage(SL0018, currentFileName, line[3].line));
					continue;
				}
				// variable access_modifier is not allowed when declearing local variable
				if (line[4].value == ":") {
					throwCompileMessage(CompileMessage(SL0015, currentFileName, line[4].line));
					continue;
				}

				k = 3;
			}
			else if (line[2].value == "=") {
				variable_declear.addProperty("tpye", "auto");
				variable_declear.addProperty("name", line[1].value);

				k = 1;
			}
			else if (line[2].value == ";") {
				variable_declear.addProperty("name", line[1].value);
				throwCompileMessage(CompileMessage(SL0034, currentFileName, line[2].line));
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0013, currentFileName, line[2].line));
				continue;
			}

			if (!vec_check_index(line, k + 1)) {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, line[k].line));
				continue;
			}

			if (line[k + 1].value == ";") {
				function_body.addProperty(variable_declear);
				continue;
			}
			else if (line[k + 1].value == "=") {
				auto result = getExpression(line, k + 2, 0);
				if (!get<bool>(result)) continue;
				Constructor variable_init = get<Constructor>(result);
				variable_init.setName("variable_init");
				variable_declear.addProperty(variable_init);
				function_body.addProperty(variable_declear);
				continue;
			}
			else {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, line[k + 1].line));
				continue;
			}
		}
		else if (line[0].value == "return") {
			if (!vec_check_index(line, 1)) {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, line[0].line));
				continue;
			}

			Constructor return_ = Constructor();
			return_.setName("return");

			if (line[1].value == ";") {
				return_.addProperty("value", "");
				function_body.addProperty(return_);
				continue;
			}

			auto result = getExpression(line, 1, 0);
			if (!get<bool>(result)) continue;
			Constructor return_val_expression = get<Constructor>(result);
			return_val_expression.setName("value");
			return_.addProperty(return_val_expression);
			function_body.addProperty(return_);
			continue;
		}

		if (line.back().value != ";") {
			tuple<Constructor, bool> expression = getExpression(line, 0, 0);
			if (!get<bool>(expression)) continue;

			function_body.addProperty(get<Constructor>(expression));
			continue;
		}

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

			if (vec_check_index(line, i + 1)) {
				if (line[i + 1].value == ".") {
					if (vec_check_index(line, i + 2) && (line[i + 2].type == TokenType::IDENTIFIER)) {
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
						throwCompileMessage(CompileMessage(SL0018, currentFileName, i + 1));
						return make_tuple(Constructor(), false);
					}
				}
				else {
					return make_tuple(reference, true);
				}
			}
			else {
				throwCompileMessage(CompileMessage(SL0018, currentFileName, i));
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
					throwCompileMessage(CompileMessage(SL0027, currentFileName, line[i + 1].line));
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
					throwCompileMessage(CompileMessage(SL0024, currentFileName, line[i].line));
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
	vector<string> no_comment_codes;

	Constructor module_tree;

	for (auto& [filename, code] : code_files) {
		currentFileName = filename;
		cout << "source code:" << endl << code << endl << endl;
		regex commentRegex("//.*");
		string no_comment_code = regex_replace(code, commentRegex, "");
		no_comment_codes.push_back(no_comment_code);

		Constructor module_tree_file = getModuleTree(code);
		module_tree = get<Constructor>(Constructor::merge(module_tree, module_tree_file));
	}

	vector<Macro> macros = getMacros(module_tree, no_comment_codes);

	for (int i = 0; i < no_comment_codes.size(); i++) {
		currentFileName = get<0>(code_files[i]);
		string preprocessed_code = preprocess(module_tree, no_comment_codes[i], macros);
		cout << "preprocessed_code:" << endl << preprocessed_code << endl << endl;
	}
}