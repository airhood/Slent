#include "BitArray.h"

// Constructor
BitArray::BitArray(size_t size) {
    resize(size);
}

// Set a bit
void BitArray::set(size_t index, bool value) {
    if (index >= bits.size() * 8) {
        throw std::out_of_range("Index out of bounds");
    }
    if (value) {
        bits[index / 8] |= (1 << (index % 8)); // Set bit
    }
    else {
        bits[index / 8] &= ~(1 << (index % 8)); // Clear bit
    }
}

// Get a bit
bool BitArray::get(size_t index) const {
    if (index >= bits.size() * 8) {
        throw std::out_of_range("Index out of bounds");
    }
    return (bits[index / 8] & (1 << (index % 8))) != 0; // Get bit
}

// Resize the bit array
void BitArray::resize(size_t newSize) {
    size_t byteSize = (newSize + 7) / 8; // Calculate required byte size
    bits.resize(byteSize);
}

// Return size in bits
size_t BitArray::size() const {
    return bits.size() * 8;
}
