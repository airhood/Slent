typedef int address;

#include "SlentVM.h"

using namespace std;
using namespace Slent;

address MemoryManager::find_free_address() {
	for (address i = 0; i < heap_size; i++) {
		if (heap_memory[i] == nullptr) return i;
	}
}

void MemoryManager::set_stack_size(int size) {
	this->stack_size = size;
}

void MemoryManager::set_max_heap_size(int size) {
	this->max_heap_size = size;
}

void MemoryManager::start() {
	heap_memory = (void**)malloc(sizeof(void*) * heap_size);
	for (int i = 0; i < heap_size; i++) {
		heap_memory[i] = nullptr;
	}
}

template <typename T>
int MemoryManager::allocate_heap() {
	T* ptr = (T*)malloc(sizeof(T));

	address free_address = find_free_address();
	heap_memory[free_address] = (void*)ptr;
	heap_usage[free_address] = true;
	return free_address;
}

template <typename T>
void MemoryManager::write_heap(int address, T value) {
	if (!heap_usage[address]) return;
	*((T*)heap_memory[address]) = value;
}

void MemoryManager::free_heap(int address) {
	free(heap_memory[address]);
	heap_memory[address] = nullptr;
}

template <typename T>
tuple<T, bool> MemoryManager::read_heap(int address) {
	if (!heap_usage[address]) return make_tuple(T(), false);
	if (heap_memory[address] == nullptr) return make_tuple(T(), false);
	return make_tuple(*((T*)heap_memory[address]), true);
}

void SlentVM::set_stack_size(int size) {
	memory_manager->set_stack_size(size);
}

void SlentVM::set_max_heap_size(int size) {
	memory_manager->set_max_heap_size(size);
}

void SlentVM::Run(string bytecode) {
	memory_manager->start();
}