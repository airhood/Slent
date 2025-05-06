#pragma once

#include <cstdlib>
#include <cstdint>
#include <cstring>

namespace Slent {
	typedef unsigned long long address;

	enum ValueType : uintptr_t {
		IntType = 0b000,
		FloatType = 0b001,
		StringType = 0b010,
		ObjectType = 0b011,
		HandleType = 0b100
	};

	constexpr uintptr_t TAG_MASK = 0b111;

	struct TaggedPtr {
		uintptr_t value;

		TaggedPtr(void* ptr, ValueType tag) {
			value = (reinterpret_cast<uintptr_t>(ptr) & ~TAG_MASK) | tag;
		}

		void* get_ptr() const {
			return reinterpret_cast<void*>(value & ~TAG_MASK);
		}

		ValueType get_type() const {
			return static_cast<ValueType>(value & TAG_MASK);
		}
	};

	class HeapManager {
	private:
		char* heap_start = nullptr;   // 포인터 산술 연산을 위해 void* 대신 char* 사용
		char* heap_end = nullptr;     // (std::byte* 등 다른 타입의 포인터 사용해고 상관없음. 포인터의 크기는 항상 동일)
		char* current_ptr = nullptr;
		size_t heap_size = 0;

	public:
		HeapManager(size_t size);
		~HeapManager();

		void* alloc_heap(size_t size);
		void reset(); // GC 또는 전체 초기화 시
	};
}
