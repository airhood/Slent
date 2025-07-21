#pragma once

#include "Constructor.h"

namespace Slent {

    const std::string VERSION = "Slent 0.0.1";

    std::string colorString(std::string str, int color);

    enum class TokenType {
        KEYWORD,
        IDENTIFIER,
        CONSTANT,
        LITERAL,
        OPERATOR,
        SPECIAL_SYMBOL,
        MACRO
    };

    struct Token {
        TokenType type;
        std::string value;
        int line;

        Token(TokenType type, std::string value, int line) :
            type(type), value(value), line(line) {}
    };

    const std::vector<std::string> keywords = {
        "import",
        "use",

        // Access Modifier
        "public",
        "private",
        "protected",

        // Declear Statement
        "module",
        "class",
        "struct",
        "func",
        "val",
        "var",
        "construct",
        "override",
        "virtual",
        "abstract",

        // Date Type
        "void",
        "null",

        "object",
        "bool",
        "char",
        "int",
        "int8",
        "int16",
        "int32",
        "int64",
        "uint",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "float32",
        "float64",
        "decimal",
        "string",

        // Function Return Type
        "->",

        // Memory Allocation
        "new",
        "deep",
        "const",

        // Conditional Statement
        "if",
        "else",
        "switch",
        "case",
        "default",

        // Iterative Statement
        "loop",
        "for",
        "while",
        "do",
        "foreach",

        // Exception Handling Statement
        "try",
        "catch",
        "finally",

        // Jumping Statement
        "break",
        "continue",
        "return"
        "goto",
    };

    // Enum for message types
    enum class MessageType {
        ERROR,
        WARNING,
        MESSAGE
    };

    struct CompileMessage {
        MessageType type;
        std::string message;
        std::string file_name;
        int line_index;

        CompileMessage(MessageType type, std::string message, std::string file_name, int line_index) :
            type(type), message(message), file_name(file_name), line_index(line_index) {}
    };

    // Structure representing a scope
    struct Scope {
        int start;
        int end;

        Scope(int start, int end) : start(start), end(end) {}
    };

    struct OperatorInfo {
        std::string value;
        int precedence;
        enum Associativity { LEFT, RIGHT } associativity;
        bool is_unary_prefix;  // 단항 접두사
        bool is_unary_postfix; // 단항 접미사

        OperatorInfo(std::string val, int prec, Associativity assoc, bool unary_prefix = false, bool unary_postfix = false)
            : value(val), precedence(prec), associativity(assoc), is_unary_prefix(unary_prefix), is_unary_postfix(unary_postfix) {
        }
    };

    const std::vector<OperatorInfo> all_operators = {
        // Postfix Unary Operators
        OperatorInfo("++", 7, OperatorInfo::LEFT, false, true),
        OperatorInfo("--", 7, OperatorInfo::LEFT, false, true),

        // Prefix Unary Operators
        OperatorInfo("!", 6, OperatorInfo::RIGHT, true),
        OperatorInfo("~", 6, OperatorInfo::RIGHT, true),
        OperatorInfo("+", 6, OperatorInfo::RIGHT, true),
        OperatorInfo("-", 6, OperatorInfo::RIGHT, true),
        OperatorInfo("++", 6, OperatorInfo::RIGHT, true),
        OperatorInfo("--", 6, OperatorInfo::RIGHT, true),

        // Multiplicative Operators
        OperatorInfo("*", 5, OperatorInfo::LEFT),
        OperatorInfo("/", 5, OperatorInfo::LEFT),
        OperatorInfo("%", 5, OperatorInfo::LEFT),

        // Additive Operators
        OperatorInfo("+", 4, OperatorInfo::LEFT),
        OperatorInfo("-", 4, OperatorInfo::LEFT),

        // Shift Operators
        OperatorInfo("<<", 3, OperatorInfo::LEFT),
        OperatorInfo(">>", 3, OperatorInfo::LEFT),

        // Relational Operators
        OperatorInfo("<", 2, OperatorInfo::LEFT),
        OperatorInfo(">", 2, OperatorInfo::LEFT),
        OperatorInfo("<=", 2, OperatorInfo::LEFT),
        OperatorInfo(">=", 2, OperatorInfo::LEFT),

        // Equality Operators
        OperatorInfo("==", 1, OperatorInfo::LEFT),
        OperatorInfo("!=", 1, OperatorInfo::LEFT),

        // Bitwise Operators
        OperatorInfo("&", 0, OperatorInfo::LEFT),
        OperatorInfo("^", -1, OperatorInfo::LEFT),
        OperatorInfo("|", -2, OperatorInfo::LEFT),

        // Logical AND
        OperatorInfo("&&", -3, OperatorInfo::LEFT),

        // Logical OR
        OperatorInfo("||", -4, OperatorInfo::LEFT),

        // Assignment Operators
        OperatorInfo("=", -5, OperatorInfo::RIGHT),
        OperatorInfo("+=", -5, OperatorInfo::RIGHT),
        OperatorInfo("-=", -5, OperatorInfo::RIGHT),
        OperatorInfo("*=", -5, OperatorInfo::RIGHT),
        OperatorInfo("/=", -5, OperatorInfo::RIGHT),
        OperatorInfo("%=", -5, OperatorInfo::RIGHT),
        OperatorInfo("<<=", -5, OperatorInfo::RIGHT),
        OperatorInfo(">>=", -5, OperatorInfo::RIGHT),
        OperatorInfo("&=", -5, OperatorInfo::RIGHT),
        OperatorInfo("^=", -5, OperatorInfo::RIGHT),
        OperatorInfo("|=", -5, OperatorInfo::RIGHT),
    };

    // Color constants for terminal output
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

    struct Macro {
        std::string macro_module;

        std::string name;
        std::vector<std::string> parameters;
        std::vector<Token> body_t;
        std::string body;

        std::string toString() {
            std::string str = "";
            str.append("{\n");
            str.append("  name: ");
            str.append(name);
            str.append("  parameters: {\n");
            for (int i = 0; i < parameters.size(); i++) {
                str.append("    " + parameters[i] + "\n");
            }
            str.append("  }\n");
            str.append("}");
        
            return str;
        }
    };

    struct CompilerSetting {
        int max_macro_recursion_depth = 3;
        bool trace_compile_logs = false;
    };

    enum State {
        NONE,
        IF,
        ELSE,
        DO,
        TRY,
        CATCH
    };

    class SlentCompiler {
    public:
        SlentCompiler() = default;

        void ConfigureSetting(CompilerSetting setting) {
            this->compilerSetting = setting;
        }

    private:
        CompilerSetting compilerSetting = CompilerSetting();

        std::vector<std::tuple<std::string, std::string>> code_files;
        std::string currentFileName;

        bool _error = false;

        // preprocess
        std::vector<Token> getPreprocessorTokens(std::string code);
        std::string preprocess(Constructor* module_tree, std::string code, std::vector<Macro> macros);
        std::vector<std::string> getImports(Constructor* module_tree, std::vector<Token> tokens);
        std::vector<Macro> getMacros(Constructor* module_tree, std::vector<std::string> codes);
        std::string runMacros(std::string code, std::vector<Macro> macros);
        std::string runMacro(Macro macro, std::vector<std::string> params);

        // module
        Constructor* getModuleTree(std::string code);
        Constructor* getSubModuleTree(std::vector<Token> tokens, Scope scope);

        // utility
        int t_find_next(std::vector<Token> tokens, int cursor, std::vector<std::string> target);
    
        // lexer
        std::vector<Token> lexer(std::string code);

        // parser
        Constructor* parser(std::vector<Token> tokens);
        std::tuple<Constructor*, bool> getFunction(std::vector<Token>& tokens, Scope scope, bool includeBody);
        std::tuple<Constructor*, int, bool> getClass(std::vector<Token>& tokens, int cursor);
        Constructor* getClassMembers(std::vector<Token>& tokens, Scope scope);
        std::vector<Constructor*> getClassConstructors(std::vector<Token>& tokens, Scope scope);
        std::vector<Constructor*> getClassVariables(std::vector<Token>& tokens, Scope scope);
        std::vector<Constructor*> getClassFunctions(std::vector<Token>& tokens, Scope scope, bool includeBody);
        Constructor* getFunctionBody(std::vector<Token>& tokens, Scope scope);

        Constructor* parseVal(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseVar(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseIf(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseElse(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseSwitch(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseLoop(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseFor(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseWhile(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseDo(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseForeach(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseTry(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseCatch(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseFinally(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseBreak(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseContinue(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseReturn(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);
        Constructor* parseGoto(std::vector<Token>& tokens, int& i, State& state, Constructor*& temp);


        std::tuple<Constructor*, bool> getSwitchStatementBody(std::vector<Token> tokens, Scope scope);

        std::tuple<Constructor*, bool> getExpression(std::vector<Token> line, Scope scope, int depth, bool semicolon);

        Constructor* parsePrimary(const std::vector<Token>& tokens, int& current_token_index, int depth, bool semicolon);
        Constructor* parsePrefixOperator(const std::vector<Token>& tokens, int& current_token_index, int depth, bool semicolon);
        Constructor* parseExpressionPrecedence(const std::vector<Token>& tokens, int& current_token_index, int min_precedence, int depth, bool semicolon);

        const OperatorInfo* getOperatorInfo(const std::string& op_value, bool is_unary_prefix_check, bool is_unary_postfix_check);
        bool isPrefixOperator(const std::vector<Token>& tokens, int index);
        bool isPostfixOperator(const std::vector<Token>& tokens, int index);

        std::tuple<Constructor*, bool> getExternalFunction(std::vector<Token>& tokens, int cursor, bool isDynamic);
        std::vector<std::vector<Token>> split_token(std::vector<Token>& tokens, Scope scope, std::string delimiter);
        bool check_type(std::string type);
        int findBraceClose(std::vector<Token>& tokens, int cursor, int current_brace);
        int findBracketClose(std::vector<Token>& tokens, int cursor, int current_bracket);
        int findNextSemicolon(std::vector<Token>& tokens, int cursor);

        // code generator
        std::string bytecode(Constructor* ast);
        void optimize();

        // error handling
        void throwCompileMessage(CompileMessage compileMessage);

    public:
        void AddFile(std::string file_name, std::string code);
        void Compile();
        bool compileError();
    };
}