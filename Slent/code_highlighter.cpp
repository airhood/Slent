#include "code_highlighter.h"
#include <sstream>
#include <regex>
#include <algorithm>

// 색상 상수 정의
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

h_Token::h_Token(h_TokenType t, std::string v, int l, size_t pos)
    : type(t), value(v), line(l), position(pos) {
}

std::string colorString(std::string str, int color) {
    return std::string("\033[0;").append(std::to_string(color)).append("m").append(str).append("\033[0m");
}

std::vector<std::string> h_split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    return result;
}

std::vector<h_Token> h_lexer(std::string code) {
    std::vector<h_Token> tokens;
    std::vector<std::string> keywords = {
        "import", "module", "func", "class", "var", "val", "return", "if", "else",
        "while", "do", "loop", "break", "continue", "public", "private", "protected",
        "extern", "override", "int", "float", "bool", "string", "void", "true", "false"
    };

    std::regex tokenRegex(R"(->|==|=|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|\+\+|\-\-|\+|\-|\*|\%|\/|<|>|\|\||&&|!|[a-zA-Z_][a-zA-Z0-9_]*|\b(0[xX][0-9a-fA-F]+|\d+\.?\d*|\d*\.\d+)\b|"[^"]*"|\(|\)|\{|\}|\[|\]|:|;|<|>|\.|,)");
    std::smatch match;
    std::vector<std::string> code_split = h_split(code, '\n');

    for (int i = 0; i < code_split.size(); i++) {
        std::string line = code_split[i];
        size_t lineStart = 0;
        for (int j = 0; j < i; j++) {
            lineStart += code_split[j].length() + 1; // +1 for newline
        }

        while (std::regex_search(line, match, tokenRegex)) {
            std::string matched = match.str();
            size_t tokenPos = lineStart + match.prefix().length();

            if (std::regex_match(matched, std::regex("\"[^\"]*\""))) {
                tokens.push_back(h_Token(h_TokenType::LITERAL, matched, i, tokenPos));
            }
            else if (std::find(keywords.begin(), keywords.end(), matched) != keywords.end()) {
                tokens.push_back(h_Token(h_TokenType::KEYWORD, matched, i, tokenPos));
            }
            else if (std::regex_match(matched, std::regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
                tokens.push_back(h_Token(h_TokenType::IDENTIFIER, matched, i, tokenPos));
            }
            else if (std::regex_match(matched, std::regex("[0-9]+(\\.[0-9]+)?"))) {
                tokens.push_back(h_Token(h_TokenType::CONSTANT, matched, i, tokenPos));
            }
            else if (std::regex_match(matched, std::regex(R"(==|!=|<=|>=|\+\=|\-\=|\*\=|\/\=|\%\=|=|\+\+|\-\-|\+|\-|\*|\%|\/|<|>|\|\||&&|!)"))) {
                tokens.push_back(h_Token(h_TokenType::OPERATOR, matched, i, tokenPos));
            }
            else if (std::regex_match(matched, std::regex(R"(\(|\)|\{|\}|\[|\]|:|;|<|>|\.|\,)"))) {
                tokens.push_back(h_Token(h_TokenType::SPECIAL_SYMBOL, matched, i, tokenPos));
            }
            else {
                tokens.push_back(h_Token(h_TokenType::UNKNOWN, matched, i, tokenPos));
            }

            line = match.suffix().str();
            lineStart += match.prefix().length() + matched.length();
        }
    }
    return tokens;
}

// ========== VS Code Dark Theme ==========
int getColorForTokenType_VSCodeDark(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return BLUE_LIGHT;    // import, func, class, var, if
        case h_TokenType::IDENTIFIER:     return WHITE_LIGHT;   // 변수명, 함수명
        case h_TokenType::CONSTANT:       return GREEN_LIGHT;   // 숫자
        case h_TokenType::LITERAL:        return YELLOW_LIGHT;  // "문자열"
        case h_TokenType::OPERATOR:       return WHITE;         // +, -, ==, !=
        case h_TokenType::SPECIAL_SYMBOL: return WHITE;         // (, ), {, }, ;
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Sublime Text Monokai Theme ==========
int getColorForTokenType_Monokai(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return MAGENTA;       // 키워드 핑크/마젠타
        case h_TokenType::IDENTIFIER:     return WHITE;         // 일반 텍스트
        case h_TokenType::CONSTANT:       return MAGENTA_LIGHT; // 숫자 밝은 보라
        case h_TokenType::LITERAL:        return YELLOW;        // 문자열 노란색
        case h_TokenType::OPERATOR:       return MAGENTA;       // 연산자 핑크
        case h_TokenType::SPECIAL_SYMBOL: return WHITE;         // 괄호 등
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== GitHub Light Theme ==========
int getColorForTokenType_GitHubLight(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return BLUE;          // 키워드 파란색
        case h_TokenType::IDENTIFIER:     return BLACK;         // 일반 텍스트 검은색
        case h_TokenType::CONSTANT:       return BLUE;          // 숫자 파란색
        case h_TokenType::LITERAL:        return RED;           // 문자열 빨간색
        case h_TokenType::OPERATOR:       return BLACK;         // 연산자 검은색
        case h_TokenType::SPECIAL_SYMBOL: return BLACK;         // 특수문자 검은색
        case h_TokenType::UNKNOWN:        return WHITE;     // 에러
        default:                          return BLACK;
    }
}

// ========== IntelliJ IDEA Darcula Theme ==========
int getColorForTokenType_Darcula(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return YELLOW_LIGHT;  // 키워드 주황/노란색
        case h_TokenType::IDENTIFIER:     return WHITE_LIGHT;   // 일반 텍스트
        case h_TokenType::CONSTANT:       return BLUE_LIGHT;    // 숫자 파란색
        case h_TokenType::LITERAL:        return GREEN;         // 문자열 초록색
        case h_TokenType::OPERATOR:       return WHITE;         // 연산자 흰색
        case h_TokenType::SPECIAL_SYMBOL: return YELLOW;        // 괄호 노란색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Atom One Dark Theme ==========
int getColorForTokenType_OneDark(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return MAGENTA_LIGHT; // 키워드 보라색
        case h_TokenType::IDENTIFIER:     return CYAN_LIGHT;    // 변수명 청록색
        case h_TokenType::CONSTANT:       return YELLOW_LIGHT;  // 숫자 주황색
        case h_TokenType::LITERAL:        return GREEN;         // 문자열 초록색
        case h_TokenType::OPERATOR:       return CYAN;          // 연산자 청록색
        case h_TokenType::SPECIAL_SYMBOL: return WHITE_LIGHT;   // 괄호 흰색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Dracula Theme ==========
int getColorForTokenType_Dracula(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return MAGENTA_LIGHT; // 키워드 핑크
        case h_TokenType::IDENTIFIER:     return WHITE_LIGHT;   // 일반 텍스트
        case h_TokenType::CONSTANT:       return MAGENTA;       // 숫자 보라
        case h_TokenType::LITERAL:        return YELLOW;        // 문자열 노란색
        case h_TokenType::OPERATOR:       return MAGENTA_LIGHT; // 연산자 핑크
        case h_TokenType::SPECIAL_SYMBOL: return WHITE;         // 괄호 흰색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Solarized Dark Theme ==========
int getColorForTokenType_SolarizedDark(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return GREEN;         // 키워드 초록색
        case h_TokenType::IDENTIFIER:     return CYAN_LIGHT;    // 변수명 밝은 청록
        case h_TokenType::CONSTANT:       return MAGENTA;       // 숫자 마젠타
        case h_TokenType::LITERAL:        return CYAN;          // 문자열 청록색
        case h_TokenType::OPERATOR:       return GREEN;         // 연산자 초록색
        case h_TokenType::SPECIAL_SYMBOL: return WHITE;         // 괄호 흰색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Nord Theme ==========
int getColorForTokenType_Nord(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return BLUE_LIGHT;    // 키워드 밝은 파란색
        case h_TokenType::IDENTIFIER:     return CYAN_LIGHT;    // 변수명 밝은 청록
        case h_TokenType::CONSTANT:       return MAGENTA_LIGHT; // 숫자 밝은 보라
        case h_TokenType::LITERAL:        return GREEN_LIGHT;   // 문자열 밝은 초록
        case h_TokenType::OPERATOR:       return CYAN;          // 연산자 청록색
        case h_TokenType::SPECIAL_SYMBOL: return WHITE_LIGHT;   // 괄호 밝은 흰색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Gruvbox Dark Theme ==========
int getColorForTokenType_Gruvbox(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return RED_LIGHT;     // 키워드 밝은 빨강
        case h_TokenType::IDENTIFIER:     return BLUE_LIGHT;    // 변수명 밝은 파랑
        case h_TokenType::CONSTANT:       return MAGENTA_LIGHT; // 숫자 밝은 보라
        case h_TokenType::LITERAL:        return GREEN_LIGHT;   // 문자열 밝은 초록
        case h_TokenType::OPERATOR:       return YELLOW;        // 연산자 노란색
        case h_TokenType::SPECIAL_SYMBOL: return WHITE_LIGHT;   // 괄호 밝은 흰색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}

// ========== Material Theme ==========
int getColorForTokenType_Material(h_TokenType type) {
    switch (type) {
        case h_TokenType::KEYWORD:        return MAGENTA_LIGHT; // 키워드 밝은 보라
        case h_TokenType::IDENTIFIER:     return YELLOW_LIGHT;  // 변수명 밝은 노랑
        case h_TokenType::CONSTANT:       return YELLOW_LIGHT;  // 숫자 밝은 노랑
        case h_TokenType::LITERAL:        return GREEN_LIGHT;   // 문자열 밝은 초록
        case h_TokenType::OPERATOR:       return CYAN_LIGHT;    // 연산자 밝은 청록
        case h_TokenType::SPECIAL_SYMBOL: return WHITE_LIGHT;   // 괄호 밝은 흰색
        case h_TokenType::UNKNOWN:        return WHITE;           // 에러
        default:                          return WHITE;
    }
}


std::string highlightSlentCode(const std::string& code) {
    std::vector<h_Token> tokens = h_lexer(code);
    std::string result = code;

    // 역순 정렬
    std::sort(tokens.begin(), tokens.end(), [](const h_Token& a, const h_Token& b) {
        return a.position > b.position;
        });

    for (const h_Token& token : tokens) {
        int color = getColorForTokenType_VSCodeDark(token.type);
        std::string coloredToken = colorString(token.value, color);
        result.replace(token.position, token.value.length(), coloredToken);
    }

    return result;
}
