#pragma once

#include <vector>

namespace Slent {
	struct StackFrame {
		void* return_address;
		std::vector<void*> args;
		std::vector<void*> locals;
	};

	class CallStack {
	private:
		std::vector<StackFrame*> stack_frames;
		size_t max_depth;

	public:
		void set_max_depth(size_t depth);
		bool push_frame(void* return_address, const std::vector<void*>& args);
		void pop_frame();
		size_t depth() const;
		StackFrame* top() const;
	};
}