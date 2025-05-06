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
		char* heap_start = nullptr;   // ������ ��� ������ ���� void* ��� char* ���
		char* heap_end = nullptr;     // (std::byte* �� �ٸ� Ÿ���� ������ ����ذ� �������. �������� ũ��� �׻� ����)
		char* current_ptr = nullptr;
		size_t heap_size = 0;

	public:
		HeapManager(size_t size);
		~HeapManager();

		void* alloc_heap(size_t size);
		void reset(); // GC �Ǵ� ��ü �ʱ�ȭ ��
	};
}
