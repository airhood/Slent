#pragma once

#include <vector>
#include <stdexcept>

class BitArray {
public:
    BitArray(size_t size);
    void set(size_t index, bool value);
    bool get(size_t index) const;
    void resize(size_t newSize);
    size_t size() const;

private:
    std::vector<uint8_t> bits; // Use uint8_t to store bits
};
