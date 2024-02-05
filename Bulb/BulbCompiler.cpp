#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include "Constructor.h"

using namespace std;


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


class BulbCompiler {
private:
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

    Constructor bytecode_compiler(vector<Token> tokens) {
        Constructor root = Constructor();
        root.setName("root");
        for (int i = 0; i < tokens.size(); i++) {
            Token token = tokens.at(i);
            switch (token.type) {
            case TokenType::KEYWORD:
                if (token.value == "import") {
                    Constructor import = Constructor();
                    import.setName("import");
                    import.addProperty("name", tokens.at(i + 1).value);
                    root.addProperty(import);
                }
                else if (token.value == "class") {
                    Constructor class_define = Constructor();
                    class_define.setName("class");
                    class_define.addProperty("name", tokens.at(i + 1).value);
                    Constructor classMembers = getClassMembers(tokens, i + 2);
                    class_define.addProperty(classMembers);
                    root.addProperty(class_define);
                    i = findBraceClose(tokens, i + 2, 0);
                }
                break;
            case TokenType::IDENTIFIER:
                break;
            case TokenType::CONSTANT:
                break;
            case TokenType::LITERAL:
                break;
            case TokenType::OPERATOR:
                break;
            case TokenType::SPECIAL_SYMBOL:
                break;
            }
        }

        return root;
    }

    Constructor getClassMembers(vector<Token> tokens, int cursor) {
        Constructor members = Constructor();
        members.setName("members");
        int _i;
        for (int i = cursor + 1; i <= findBraceClose(tokens, cursor, 0); i++) {
            Token token = tokens.at(i);
            if (token.value == "var") {
                Constructor variable = Constructor();
                variable.setName("variable");

                if (tokens.at(i + 1).type != TokenType::KEYWORD) {
                    cout << tokens.at(i + 1).value << endl;
                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected variable declear syntax", ""));
                    i = findNextSemicolon(tokens, i + 1);
                    continue;
                }
                variable.addProperty("access modifier", tokens.at(i + 1).value);

                if (!checkType(tokens.at(i + 2))) {
                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected variable declear syntax", ""));
                    i = findNextSemicolon(tokens, i + 2);
                    continue;
                }
                variable.addProperty("type", tokens.at(i + 2).value);

                if (tokens.at(i + 3).type != TokenType::IDENTIFIER) {
                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected variable name", string("Keywords cannot be used as variable name. Use other name instead of \'").append(tokens.at(i + 3).value).append("\'.")));
                }
                variable.addProperty("name", tokens.at(i + 3).value);

                if (tokens.at(i + 4).value != ";") {
                    if (tokens.at(i + 4).value == "=") {
                        if (tokens.at(i + 5).type == TokenType::CONSTANT) { // init variable using constant value
                            Constructor init = Constructor();
                            init.setName("init");
                            init.addProperty("type", "constant_value");
                            init.addProperty("value", tokens.at(i + 5).value);
                            variable.addProperty(init);
                            if (tokens.at(i + 6).value == ";") {
                                i = i + 6;
                                members.addProperty(variable);
                                continue;
                            }
                            else {
                                cout << tokens.at(i + 6).value << endl;
                                throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected variable declear syntax", ""));
                                i = findNextSemicolon(tokens, i + 6);
                                continue;
                            }
                        }
                        else if (tokens.at(i + 5).value == "new") { // init variable by creating object
                            if (classExist(tokens, tokens.at(i + 6).value)) {
                                Constructor init = Constructor();
                                init.setName("init");
                                init.addProperty("type", "object_construct");
                                Constructor object_construct = Constructor();
                                object_construct.setName("object_constructor");
                                int p = 0;
                                for (int j = i + 7; j < findNextSemicolon(tokens, i + 7); j++) {

                                    object_construct.addProperty(string("param").append(to_string(p)), "");
                                }
                                init.addProperty(object_construct);
                                variable.addProperty(init);
                                members.addProperty(variable);
                                continue;
                            }
                            else {
                                try {
                                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Undefined class used", string("class '").append(tokens.at(i + 6).value).append("' is not defined")));
                                    i = findNextSemicolon(tokens, i + 6);
                                    continue;
                                }
                                catch (...) {
                                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected object initializer syntax", "new ClassName(param, param, ... ) is the correct syntax for object initializer"));
                                    continue;
                                }
                            }
                        }
                        else {
                            throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected variable init", "Variable can only be initialized with constant value or object construction."));
                            i = findNextSemicolon(tokens, i);
                            continue;
                        }
                    }
                }
            }
            else if (token.value == "construct") {
                if (tokens.at(i + 1).value == "(") {
                    Constructor constructor = Constructor();
                    constructor.setName("constructor");

                    if (tokens.at(i + 2).value == ")") {
                        constructor.addProperty("param_exist", "0");
                    }
                    else {
                        constructor.addProperty("param_exist", "1");
                        int k;
                        int param_index = 0;
                        bool isFirst = true;

                        // get parameters
                        for (int j = i + 2; j < findBracketClose(tokens, i + 2, 1); j++) {
                            if (isFirst) {
                                Constructor param_define = Constructor();
                                param_define.setName(string("param").append(to_string(param_index)).append("_define"));
                                param_define.addProperty("type", tokens.at(j).value);
                                param_define.addProperty("name", tokens.at(j + 1).value);
                                constructor.addProperty(param_define);
                                param_index++;
                                j = j + 1;
                                k = j;
                            }
                            else if (tokens.at(j).value == ",") {
                                Constructor param_define = Constructor();
                                param_define.setName(string("param").append(to_string(param_index)).append("_define"));
                                param_define.addProperty("type", tokens.at(j + 1).value);
                                param_define.addProperty("name", tokens.at(j + 2).value);
                                constructor.addProperty(param_define);
                                param_index++;
                                j = j + 2;
                                k = j;
                            }
                        }
                        i = k;
                    }

                    if ((tokens.at(i + 1).value == ")") && (tokens.at(i + 2).value == "{")) {

                        i = findBraceClose(tokens, i + 2, 0);
                    }
                    else {
                        throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected constructor syntax", "construct(param a, param b, ... ) is the correct syntax for class constructor."));
                    }
                }
                else {
                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected constructor syntax", "construct(param a, param b, ... ) is the correct syntax for class constructor."));
                }
            }
            else if (token.value == "func") {
                Constructor function = Constructor();
                function.setName("function");
                // get function parameters
                if (tokens.at(i + 1).type == TokenType::IDENTIFIER) {
                    if (tokens.at(i + 2).value == "(") {
                        if (tokens.at(i + 3).value == ")") {
                            function.addProperty("param_exist", "0");
                            i = i + 2;
                        }
                        else {
                            function.addProperty("param_exist", "1");
                            int k = 0;
                            int param_index = 0;
                            bool isFirst = true;

                            // get parameters
                            for (int j = i + 3; i < findBracketClose(tokens, i + 3, 1); j++) {
                                if (isFirst) {
                                    Constructor param_define = Constructor();
                                    param_define.setName(string("param").append(to_string(param_index).append("_define")));
                                    if (!checkType(tokens.at(j))) {
                                        throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected parameter syntax", ""));
                                        i = findNextSemicolon(tokens, j);
                                        continue;
                                    }
                                    param_define.addProperty("type", tokens.at(j).value);
                                    if (tokens.at(j + 1).type != TokenType::IDENTIFIER) {
                                        throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected parameter syntax", ""));
                                        continue;
                                    }
                                    param_define.addProperty("name", tokens.at(j + 1).value);
                                    function.addProperty(param_define);
                                    param_index++;
                                    j = j + 1;
                                    k = j;
                                }
                                else if (tokens.at(i).value == ",") {
                                    Constructor param_define = Constructor();
                                    param_define.setName(string("param").append(to_string(param_index).append("_define")));
                                    if (!checkType(tokens.at(j + 1))) {
                                        throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected parameter syntax", ""));
                                        i = findNextSemicolon(tokens, j + 1);
                                        continue;
                                    }
                                    param_define.addProperty("type", tokens.at(j + 1).value);
                                    if (tokens.at(j + 2).type != TokenType::IDENTIFIER) {
                                        throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected parameter syntax", ""));
                                        continue;
                                    }
                                    param_define.addProperty("name", tokens.at(j + 2).value);
                                    function.addProperty(param_define);
                                    param_index++;
                                    j = j + 2;
                                    k = j;
                                }
                                else {
                                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function declear syntax", "unexpected parameters.\nfunc function_name(param a, param b, ... ) -> return_type {\n  ...\n}\n is the correct syntax for function declearation."));
                                }
                            }
                            i = k;
                        }

                        if ((tokens.at(i + 1).value == ")")) {
                            if (tokens.at(i + 2).value == "->") { // function that has return value
                                if (checkType(tokens.at(i + 3))) {
                                    Constructor return_value = Constructor();
                                    return_value.setName("return_value");
                                    return_value.addProperty("type", tokens.at(i + 3).value);
                                    function.addProperty(return_value);
                                    i = i + 2;
                                }
                                else {
                                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected return expression", "-> ReturnType is the correct syntax for function return expression."));
                                }
                            }

                            if (tokens.at(i + 2).value == "{") { // function that doesn't have return value
                                // get function body
                                for (int j = i + 3; j < findBraceClose(tokens, i + 3, 1); j = findNextSemicolon(tokens, j) + 1) { // read line by line
                                    for (int k = j; k < findNextSemicolon(tokens, j); k++) { // read line
                                        if (tokens.at(k).type == TokenType::KEYWORD) {
                                            if (tokens.at(k).value == "if") {
                                                Constructor if_statement = Constructor();
                                                if_statement.setName("if");
                                                Constructor condition = Constructor();
                                                condition.setName("condition");
                                                if_statement.addProperty(condition);
                                            }
                                        }
                                    }
                                }
                                i = findBraceClose(tokens, i + 3, 1);
                                continue;
                            }
                            else {
                                throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function declear syntax", "func function_name(param a, param b, ... ) -> return_type {\n  ...\n}\n is the correct syntax for function declearation."));
                            }
                        }
                        else {
                            throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function declear syntax", "func function_name(param a, param b, ... ) -> return_type {\n  ...\n}\n is the correct syntax for function declearation."));
                        }
                    }
                    else {
                        throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function declear syntax", "function parameters are required.\nfunc function_name(param a, param b, ... ) -> return_type {\n  ...\n}\n is the correct syntax for function declearation."));
                    }
                }
                else {
                    throwCompileMessage(CompileMessage(MessageType::ERROR, "Unexpected function name", string("Keywords cannot be used as function name. Use other name instead of \'").append(tokens.at(i + 3).value).append("\'.")));
                }
            }
            _i = i;
        }

        return members;
    }

    string colorString(string str, int color) {
        return string("\033[0;").append(to_string(color)).append("m").append(str).append("\033[0m");
    }

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
        switch (compileMessage.type) {
        case MessageType::ERROR:
            type = "Error";
            break;
        case MessageType::WARNING:
            type = "Warning";
            break;
        case MessageType::MESSAGE:
            type = "Message";
            break;
        }
        if (compileMessage.message == "") {
            string str = colorString(string("[").append(type).append("] ").append(compileMessage.name), 31);
            cout << "[" << type << "] " << compileMessage.name << endl;
            cout << str << endl;
            return;
        }
        string str = colorString(string("[").append(type).append("] ").append(compileMessage.name).append(":\n").append(compileMessage.message), 31);
        cout << "[" << type << "] " << compileMessage.name << ": " << compileMessage.message << endl;
        cout << str << endl;
    }

    void parser(string bytecode) {

    }

public:
    void Compile(string code) {
        regex commentRegex("//.*");
        code = regex_replace(code, commentRegex, "");

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

            Constructor bytecode = bytecode_compiler(tokens);
            cout << "bytecode:" << endl << bytecode.toString() << endl;
        }
        catch (const invalid_argument& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
};
