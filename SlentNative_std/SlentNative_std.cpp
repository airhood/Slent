#ifdef _WIN32
#define SLENT_API __declspec(dllexport)
#else
#define SLENT_API
#endif

#include <iostream>
#include <string>

extern "C" {
    SLENT_API void print(std::string str) {
        printf("%s", str.c_str());
    }

    SLENT_API void scan(std::string& buffer) {
        std::string input;
        static std::string leftover;

        while (true) {
            std::getline(std::cin, input);

            if (std::cin.eof() || std::cin.fail()) {
                std::cerr << "Error: Input exceeded maximum length or invalid input." << std::endl;
                input.clear();
                break;
            }

            size_t spacePos = input.find(' ');
            if (spacePos != std::string::npos) {
                buffer = input.substr(0, spacePos);
                leftover = input.substr(spacePos + 1);
                break;
            } else {
                buffer = input;
            }
        }
    }

    SLENT_API void scanln(std::string& buffer) {
        std::getline(std::cin, buffer);
    }

    SLENT_API char readChar() {
        return std::cin.get();
    }
}
