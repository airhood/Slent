#pragma once

#include <string>
#include <vector>

namespace Slent {

    // Color constants for terminal output
    const int BLACK = 30;
    const int RED = 31;
    const int GREEN = 32;
    const int YELLOW = 33;
    const int BLUE = 34;
    const int MAGENTA = 35;
    const int CYAN = 36;
    const int WHITE = 37;

    const int BLACK_LIGHT = 90;
    const int RED_LIGHT = 91;
    const int GREEN_LIGHT = 92;
    const int YELLOW_LIGHT = 93;
    const int BLUE_LIGHT = 94;
    const int MAGENTA_LIGHT = 95;
    const int CYAN_LIGHT = 96;
    const int WHITE_LIGHT = 97;

    enum class VM_MessageType {
        ERROR,
        WARNING,
        MESSAGE
    };

    struct RuntimeMessage {
        VM_MessageType type;
        std::string message;
        std::string file_name;
        int line_index;

        RuntimeMessage() = default;

        RuntimeMessage(VM_MessageType type, std::string message, std::string file_name, int line_index) :
            type(type), message(message), file_name(file_name), line_index(line_index) {}
    };

    struct MemoryRequestResult {
        bool success;
        RuntimeMessage error;

        MemoryRequestResult(bool success) :
            success(success) {}
    };

    typedef unsigned long long address;

    class MemoryManager {
    private:
        size_t stack_size;
        size_t current_stack_index; // max index
        void** stack_memory;

        address find_free_address();

    public:
        MemoryManager() {
            stack_size = 0;
            current_stack_index = -1;
            stack_memory = nullptr;
        }

        void set_stack_size(int size);
        void set_max_heap_size(int size);
        void start();
        template <typename T>
        int allocate_heap();
        template <typename T>
        void write_heap(int address, T value);
        void free_heap(int address);
        template <typename T>
        std::tuple<T, bool> read_heap(int address);
    };

    class BytecodeInterpreter {
    private:
        int* status_code;
        std::string error;

        int Interpret(std::vector<std::string> command);

    public:
        BytecodeInterpreter(int* status_code);

        void RunBytecode(std::string bytecode);
    };

    struct VMSetting {
        int stack_size;
        int max_heap_size;
    };

    void throwRuntimeMessage(RuntimeMessage compileMessage);

    class SlentVM {
    private:
        BytecodeInterpreter* bytecode_interpreter;
        MemoryManager* memory_manager;
        VMSetting vm_setting;

        int* status_code;

    public:
        SlentVM();

        void ConfigureSetting(VMSetting setting);
        void Run(std::string bytecode);

    private:
        void applySetting();
    };
}