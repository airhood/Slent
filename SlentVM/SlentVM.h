#pragma once

#include <string>

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

    typedef int address;

    class MemoryManager {
    private:
        int stack_size;
        int heap_size;
        int max_heap_size;
        int current_stack_index; // max index
        void** stack_memory;
        void** heap_memory;

        bool* heap_usage;

        address find_free_address();

    public:
        MemoryManager() {
            stack_size = 0;
            heap_size = 0;
            max_heap_size = 0;
            current_stack_index = -1;
            stack_memory = nullptr;
            heap_memory = nullptr;
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
        int Interpret(std::vector<std::string> command);

    public:
        BytecodeInterpreter() = default;

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

    public:
        SlentVM() {
            bytecode_interpreter = new BytecodeInterpreter;
            memory_manager = new MemoryManager;
        }

        void set_stack_size(int size);
        void set_max_heap_size(int size);
        void Run(std::string bytecode);
    };
}