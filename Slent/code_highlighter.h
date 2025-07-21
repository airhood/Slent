#pragma once

#include <string>
#include <vector>

enum class h_TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    LITERAL,
    OPERATOR,
    SPECIAL_SYMBOL,
    UNKNOWN
};

struct h_Token {
    h_TokenType type;
    std::string value;
    int line;
    size_t position;

    h_Token(h_TokenType t, std::string v, int l, size_t pos);
};

// 색상 상수 선언
extern const int BLACK;
extern const int RED;
extern const int GREEN;
extern const int YELLOW;
extern const int BLUE;
extern const int MAGENTA;
extern const int CYAN;
extern const int WHITE;

extern const int BLACK_LIGHT;
extern const int RED_LIGHT;
extern const int GREEN_LIGHT;
extern const int YELLOW_LIGHT;
extern const int BLUE_LIGHT;
extern const int MAGENTA_LIGHT;
extern const int CYAN_LIGHT;
extern const int WHITE_LIGHT;

// 함수 선언
std::string colorString(std::string str, int color);
std::vector<std::string> h_split(const std::string& str, char delimiter);
std::vector<h_Token> h_lexer(std::string code);
int getColorForTokenType(h_TokenType type);
std::string highlightSlentCode(const std::string& code);
