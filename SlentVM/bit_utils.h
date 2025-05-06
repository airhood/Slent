#pragma once
#include <cstdint>

inline bool get_bit(uint8_t* bits, size_t index) {
	return (bits[index / 8] >> (index % 8)) & 1;
}

inline void set_bit(uint8_t* bits, size_t index, bool value) {
	if (value)
		bits[index / 8] |= (1 << (index % 8));
	else
		bits[index / 8] &= ~(1 << (index % 8));
}
