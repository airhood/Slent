#include "SlentVM.h"
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
	heap_usage = (bool*)malloc(sizeof(bool) * heap_size);
	heap_marked = (bool*)malloc(sizeof(bool) * heap_size);
	if (heap_memory == NULL || heap_usage == NULL || heap_marked == NULL) {
		cerr << "Failed to allocate memory." << endl;
		return;
	}
	for (int i = 0; i < heap_size; i++) {
		heap_memory[i] = nullptr;
		heap_usage[i] = false;
		heap_marked[i] = false;
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

const int CODE_DEFAULT = 0;
const int CODE_ERROR = -1;
const int CODE_EXIT = -2;

int BytecodeInterpreter::Interpret(vector<string> command) {
	if (!vec_check_index(command, 1)) {
		return CODE_ERROR;
	}

	if (command[0] == "goto") {
		int line = stoi(command[1]);
		if (line < 1) {
			error = "Cannot go to line " + to_string(line);
			return CODE_ERROR;
		}
		return line;
	}
	else if (command[0] == "process") {
		if (command[1] == "exit") {
			if (vec_check_index(command, 2)) {
				*status_code = stoi(command[2]);
			}
			return CODE_EXIT;
		}
	}
	else {
		error = string("Unknown bytecode command: ").append(command[0]);
		return CODE_ERROR;
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

BytecodeInterpreter::BytecodeInterpreter(int* status_code) {
	this->status_code = status_code;
}

void BytecodeInterpreter::RunBytecode(string bytecode) {
	vector<string> lines = split(bytecode, '\n');
	int max_cursor = lines.size() - 1;
	int cursor = 0;
	while (true) {
		vector<string> tokens = split(lines[cursor], ':');
		vector<string> command = split(tokens[0], ' ');

		string filename;
		if (!vec_check_index(tokens, 1)) {
			filename = "unknown";
		}
		else {
			filename = tokens[1];
		}

		string line;
		if (!vec_check_index(tokens, 2)) {
			line = "-1";
		}
		else {
			line = tokens[2];
		}

		int result_code = Interpret(command);

		switch (result_code) {
			case CODE_DEFAULT:
				break;
			case CODE_ERROR:
				throwRuntimeMessage(RuntimeMessage(VM_MessageType::ERROR, string("Bytecode error: ").append(error), filename, stoi(line)));
				return;
			case CODE_EXIT:
				return;
			default:
				cursor = result_code;
				if (cursor > max_cursor) {
					throwRuntimeMessage(RuntimeMessage(VM_MessageType::ERROR, "Bytecode error", filename, stoi(line)));
					return;
				}
				continue;
		}

		cursor++;
		if (cursor > max_cursor) {
			throwRuntimeMessage(RuntimeMessage(VM_MessageType::ERROR, "Bytecode error: exit command missing", filename, stoi(line)));
			return;
		}
	}
}

string colorString(string str, int color) {
	return string("\033[0;").append(to_string(color)).append("m").append(str).append("\033[0m");
}

void Slent::throwRuntimeMessage(RuntimeMessage runtimeMessage) {
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
	
	string line;
	if (runtimeMessage.line_index == -1) {
		line = "unknown";
	}
	else {
		line = to_string(runtimeMessage.line_index + 1);
	}

	string str = colorString(string("[").append(type).append("] ")
		.append(runtimeMessage.message)
		.append("(").append(runtimeMessage.file_name)
		.append(":line ").append(line)
		.append(")"), color);
	cout << str << endl;
}

SlentVM::SlentVM() {
	status_code = new int;
	*status_code = 0;
	bytecode_interpreter = new BytecodeInterpreter(status_code);
	memory_manager = new MemoryManager;
	vm_setting = VMSetting();
}

void SlentVM::ConfigureSetting(VMSetting setting) {
	vm_setting = setting;
}

void SlentVM::Run(string bytecode) {
	memory_manager->start();
	bytecode_interpreter->RunBytecode(bytecode);
	cout << colorString(string("Process exited with code ").append(to_string(*status_code)), CYAN_LIGHT);
}

void SlentVM::applySetting() {
	memory_manager->set_stack_size(vm_setting.stack_size);
	memory_manager->set_max_heap_size(vm_setting.max_heap_size);
}