#pragma once

#include <string>

namespace Slent {

    enum class VM_MessageType {
        ERROR,
        WARNING,
        MESSAGE
    };

    struct RuntimeError {
        VM_MessageType type;
        std::string message;
        std::string file_name;
        int line_index;

        RuntimeError() = default;

        RuntimeError(VM_MessageType type, std::string message, std::string file_name, int line_index) :
            type(type), message(message), file_name(file_name), line_index(line_index) {}
    };

    struct MemoryRequestResult {
        bool success;
        RuntimeError error;

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

    struct VMSetting {
        int stack_size;
        int max_heap_size;
    };

    class SlentVM {
    private:
        MemoryManager* memory_manager;
        VMSetting vm_setting;

    public:
        SlentVM() {
            memory_manager = new MemoryManager;
        }

        void set_stack_size(int size);
        void set_max_heap_size(int size);
        void Run(std::string bytecode);
    };
}