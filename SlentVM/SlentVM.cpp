typedef int address;

#include "SlentVM.h"
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Slent;

#define vec_check_index(vec, index) (vec.size() >= (index + 1))

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

const int CODE_ERROR = -1;
const int CODE_EXIT = -2;

int BytecodeInterpreter::Interpret(vector<string> command) {
	if (!vec_check_index(command, 1)) {
		return CODE_ERROR;
	}

	if (command[0] == "goto") {
		return stoi(command[1]);
	}
	else if (command[0] == "process") {
		if (command[1] == "exit") {
			return CODE_EXIT;
		}
	}
	else {
		return CODE_ERROR; // error
	}
}

vector<string> split(string str, char Delimiter) {
	istringstream iss(str);
	string buffer;

	vector<string> result;

	while (getline(iss, buffer, Delimiter)) {
		result.push_back(buffer);
	}
	return result;
}

void BytecodeInterpreter::RunBytecode(string bytecode) {
	vector<string> lines = split(bytecode, '\n');
	for (auto& line : lines) {
		vector<string> tokens = split(bytecode, ':');
		vector<string> command = split(tokens[0], ' ');
		string filename = tokens[1];
		string line = tokens[2];
		int result_code = Interpret(command);
		if (result_code == -1) {
			throwRuntimeMessage(RuntimeMessage(VM_MessageType::ERROR, "Internal Error", filename, stoi(line)));
			return;
		}
	}
}

string colorString(string str, int color) {
	return string("\033[0;").append(to_string(color)).append("m").append(str).append("\033[0m");
}

void throwRuntimeMessage(RuntimeMessage runtimeMessage) {
	string type;
	int color;
	switch (runtimeMessage.type) {
		case VM_MessageType::ERROR:
			type = "Error";
			color = RED;
			break;
		case VM_MessageType::WARNING:
			type = "Warning";
			color = YELLOW;
			break;
		case VM_MessageType::MESSAGE:
			type = "Message";
			color = CYAN;
			break;
		default:
			return;
	}
	string str = colorString(string("[").append(type).append("] ")
		.append(runtimeMessage.message)
		.append("(")
		.append(":line ").append(to_string(runtimeMessage.line_index + 1))
		.append(")"), color);
	cout << str << endl;
}

void SlentVM::set_stack_size(int size) {
	memory_manager->set_stack_size(size);
}

void SlentVM::set_max_heap_size(int size) {
	memory_manager->set_max_heap_size(size);
}

void SlentVM::Run(string bytecode) {
	memory_manager->start();
	bytecode_interpreter->RunBytecode(bytecode);
}