#include "HeapManager.h"
#include <iostream>
#include <cstring>

using namespace Slent;


#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)

inline int ctzll(uint64_t value) {
	unsigned long index;
	if (_BitScanForward64(&index, value))
		return static_cast<int>(index);
	return 64;
}

#else // GCC, Clang
inline int ctzll(uint64_t value) {
	return value ? __builtin_ctzll(value) : 64;
}
#endif


HeapManager::HeapManager(size_t size) {
    heap_size = size;
    heap_start = (char*)std::malloc(size);
    if (!heap_start) {
        std::cerr << "Heap allocation failed!" << std::endl;
        std::exit(1);
    }
    current_ptr = heap_start;
    heap_end = heap_start + size;
}

HeapManager::~HeapManager() {
    std::free(heap_start);
    heap_start = nullptr;
    current_ptr = nullptr;
    heap_end = nullptr;
    heap_size = 0;
}

void* HeapManager::alloc_heap(size_t size) {
    if (current_ptr + size > heap_end) {
        std::cerr << "Heap overflow: unable to allocate " << size << " bytes." << std::endl;
        return nullptr;
    }

    void* result = current_ptr;
    current_ptr += size;
    return result;
}

void HeapManager::reset() {
    current_ptr = heap_start;
}
