#pragma once

#include "Constructor.h"

namespace Slent {

    const std::string VERSION = "0.0.1";

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

        // Access Modifier
        "public",
        "private",
        "protected",

        // Declear Statement
        "module",
        "class",
        "struct",
        "func",
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
        "goto",
        "return"
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
        int precedence; // 숫자가 높을수록 우선순위 높음
        enum Associativity { LEFT, RIGHT } associativity;
        bool is_unary_prefix;  // 단항 접두사 (예: !)
        bool is_unary_postfix; // 단항 접미사 (예: ++)

        OperatorInfo(std::string val, int prec, Associativity assoc, bool unary_prefix = false, bool unary_postfix = false)
            : value(val), precedence(prec), associativity(assoc), is_unary_prefix(unary_prefix), is_unary_postfix(unary_postfix) {
        }
    };

    const std::vector<OperatorInfo> all_operators = {
        // Postfix Unary Operators (가장 높은 우선순위) - 예: arr[index], func(), ++, --
        // 이들은 TokenType::OPERATOR로 분류되지만, 파서에서 특별히 처리합니다.
        OperatorInfo("++", 7, OperatorInfo::LEFT, false, true), // 후위 증감
        OperatorInfo("--", 7, OperatorInfo::LEFT, false, true), // 후위 감소
        // 참고: 배열 인덱싱 '[]' 및 함수 호출 '()'은 일반적으로 OPERATOR가 아닌 SPECIAL_SYMBOL로 처리되며,
        // parsePrimary에서 특별한 문법으로 다뤄집니다.

        // Prefix Unary Operators - 예: !, ~, +, - (단항), ++, --
        OperatorInfo("!", 6, OperatorInfo::RIGHT, true),  // 논리 NOT
        OperatorInfo("~", 6, OperatorInfo::RIGHT, true),  // 비트 NOT
        OperatorInfo("+", 6, OperatorInfo::RIGHT, true),  // 단항 플러스
        OperatorInfo("-", 6, OperatorInfo::RIGHT, true),  // 단항 마이너스
        OperatorInfo("++", 6, OperatorInfo::RIGHT, true), // 전위 증감
        OperatorInfo("--", 6, OperatorInfo::RIGHT, true), // 전위 감소

        // Multiplicative Operators
        OperatorInfo("*", 5, OperatorInfo::LEFT), // 곱셈
        OperatorInfo("/", 5, OperatorInfo::LEFT), // 나눗셈
        OperatorInfo("%", 5, OperatorInfo::LEFT), // 나머지

        // Additive Operators
        OperatorInfo("+", 4, OperatorInfo::LEFT), // 덧셈 (이항)
        OperatorInfo("-", 4, OperatorInfo::LEFT), // 뺄셈 (이항)

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

        // Logical AND (비트 AND &는 보통 비교 연산자보다 높고, 논리 AND && 보다 높습니다.)
        OperatorInfo("&&", 0, OperatorInfo::LEFT),

        // Logical OR
        OperatorInfo("||", -1, OperatorInfo::LEFT),

        // Assignment Operators (가장 낮은 우선순위, 우측 결합성)
        OperatorInfo("=", -2, OperatorInfo::RIGHT),
        OperatorInfo("+=", -2, OperatorInfo::RIGHT),
        OperatorInfo("-=", -2, OperatorInfo::RIGHT),
        OperatorInfo("*=", -2, OperatorInfo::RIGHT),
        OperatorInfo("/=", -2, OperatorInfo::RIGHT),
        OperatorInfo("%=", -2, OperatorInfo::RIGHT),
        OperatorInfo("<<=", -2, OperatorInfo::RIGHT),
        OperatorInfo(">>=", -2, OperatorInfo::RIGHT),
        // ... 기타 대입 연산자
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
        std::tuple<Constructor*, bool> getFunction(std::vector<Token> tokens, Scope scope, bool includeBody);
        std::tuple<Constructor*, int, bool> getClass(std::vector<Token> tokens, int cursor);
        Constructor* getClassMembers(std::vector<Token> tokens, Scope scope);
        std::vector<Constructor*> getClassConstructors(std::vector<Token> tokens, Scope scope);
        std::vector<Constructor*> getClassVariables(std::vector<Token> tokens, Scope scope);
        std::vector<Constructor*> getClassFunctions(std::vector<Token> tokens, Scope scope, bool includeBody);
        Constructor* getFunctionBody(std::vector<Token> tokens, Scope scope);
        std::tuple<Constructor*, bool> getExpression(std::vector<Token> line, Scope scope, int depth);

        Constructor* parsePrimary(const std::vector<Token>& tokens, int& current_token_index, int depth);
        Constructor* parsePrefixOperator(const std::vector<Token>& tokens, int& current_token_index, int depth);
        Constructor* parseExpressionPrecedence(const std::vector<Token>& tokens, int& current_token_index, int min_precedence, int depth);

        const OperatorInfo* getOperatorInfo(const std::string& op_value, bool is_unary_prefix_check, bool is_unary_postfix_check);
        bool isPrefixOperator(const std::vector<Token>& tokens, int index);
        bool isPostfixOperator(const std::vector<Token>& tokens, int index);

        std::tuple<Constructor*, bool> getExternalFunction(std::vector<Token> tokens, int cursor, bool isDynamic);
        std::vector<std::vector<Token>> split_token(std::vector<Token> tokens, Scope scope, std::string delimiter);
        bool check_type(std::string type);
        int findBraceClose(std::vector<Token> tokens, int cursor, int current_brace);
        int findBracketClose(std::vector<Token> tokens, int cursor, int current_bracket);
        int findNextSemicolon(std::vector<Token> tokens, int cursor);

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