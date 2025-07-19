#include "slent_api.h"
#include <iostream>
#include <string>

extern "C" {

	SLENT_API bool string_equals(std::string self, std::string str) {
		return self == str;
	}

	SLENT_API int string_compare(std::string self, std::string str) {
		return self.compare(str);
	}

	SLENT_API char string_getCharAt(std::string self, int index) {
		return self[index];
	}

	SLENT_API int string_length(std::string self) {
		return self.length();
	}

	SLENT_API char* string_toCharArray(std::string self) {
		return const_cast<char*>(self.c_str());
	}

	SLENT_API std::string string_subStr(std::string self, int startIndex) {
		return self.substr(startIndex);
	}

	SLENT_API std::string string_subStr(std::string self, int startIndex, int length) {
		return self.substr(startIndex, length);
	}

}
