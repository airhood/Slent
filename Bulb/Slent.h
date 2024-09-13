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
        SPECIAL_SYMBOL
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
        "int8",
        "int16",
        "int32",
        "int64",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "float32",
        "float64",
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

    struct RuntimeError {
        MessageType type;
        std::string message;
        std::string file_name;
        int line_index;

        RuntimeError() = default;

        RuntimeError(MessageType type, std::string message, std::string file_name, int line_index) :
            type(type), message(message), file_name(file_name), line_index(line_index) {}
    };

    // Structure representing a scope
    struct Scope {
        int start;
        int end;

        Scope(int start, int end) : start(start), end(end) {}
    };

    // Function to split a string based on a delimiter
    std::vector<std::string> split(std::string str, char Delimiter);

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

        std::string name;
        std::vector<std::string> parameters;
        std::vector<Token> body_t;
        std::string body;
    };

    class SlentCompiler {
    private:
        std::vector<std::tuple<std::string, std::string>> code_files;
        std::string currentFileName;

        std::vector<Token> getPreprocessorTokens(std::string code);
        std::string preprocess(std::string code);
        std::string run_macro(Macro macro, std::vector<std::string> params);
        
        int p_find_next(std::string** preprocessor_tokens, int lines, int cursor, std::vector<std::string> target);
        int t_find_next(std::vector<Token> tokens, int cursor, std::vector<std::string> target);
        std::vector<Token> lexer(std::string code);
        Constructor parser(std::vector<Token> tokens);
        std::tuple<Constructor, int, bool> getClass(std::vector<Token> tokens, int cursor);
        Constructor getClassMembers(std::vector<Token> tokens, Scope scope);
        std::vector<Constructor> getClassConstructors(std::vector<Token> tokens, Scope scope);
        Constructor getConstructorBody(std::vector<Token> tokens, Scope scope);
        std::vector<Constructor> getClassVariables(std::vector<Token> tokens, Scope scope);
        std::vector<Constructor> getClassFunctions(std::vector<Token> tokens, Scope scope);
        Constructor getFunctionBody(std::vector<Token> tokens, Scope scope);
        std::tuple<Constructor, bool> getExpression(std::vector<Token> line, int start_index, int depth);
        std::vector<std::vector<Token>> split_token(std::vector<Token> tokens, Scope scope, std::string delimiter);
        bool check_type(std::string type);
        int findBraceClose(std::vector<Token> tokens, int cursor, int current_brace);
        int findBracketClose(std::vector<Token> tokens, int cursor, int current_bracket);
        int findNextSemicolon(std::vector<Token> tokens, int cursor);
        void throwCompileMessage(CompileMessage compileMessage);
        std::string bytecode(Constructor root);
        void optimize();
        void compile_file(std::string file_name, std::string code);

    public:
        void AddFile(std::string file_name, std::string code);
        void Compile();
    };

    struct MemoryRequestResult {
        bool success;
        RuntimeError error;

        MemoryRequestResult(bool success) :
            success(success) {}
    };

    typedef int address;

    class MemoryManager {
    private:
        int stack_size;
        int heap_size;
        int current_stack_index;
        void** stack_memory;
        void** heap_memory;
        
        bool* heap_usage;

        address find_free_address();

    public:
        MemoryManager() {
            stack_size = 0;
            heap_size = 0;
            current_stack_index = 0;
        }

        void set_stack_size(int size);
        void set_heap_size(int size);
        void start();
        template <typename T>
        int allocate_heap();
        template <typename T>
        void write_heap(int address, T value);
        void free_heap(int address);
        template <typename T>
        std::tuple<T, bool> read_heap(int address);
    };

    class SlentVirtualMachine {
    private:
        MemoryManager* memory_manager;
        
    public:
        SlentVirtualMachine() {
            memory_manager = new MemoryManager;
        }

        void set_stack_size(int size);
        void set_heap_size(int size);
        void Run(std::string bytecode);
    };
}