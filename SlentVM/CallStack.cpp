#include "CallStack.h"

using namespace Slent;

void CallStack::set_max_depth(size_t depth) {
	max_depth = depth;
}

bool CallStack::push_frame(void* return_address, const std::vector<void*>& args) {
	StackFrame* new_frame = new StackFrame;
	new_frame->return_address = return_address;
	new_frame->args = args;
	stack_frames.push_back(new_frame);
	
	if (stack_frames.size() > max_depth) return false;
	return true;
}

void CallStack::pop_frame() {
	if (!stack_frames.empty()) {
		StackFrame* frame = stack_frames.back();
		stack_frames.pop_back();
		delete frame;
	}
}

size_t CallStack::depth() const {
	return stack_frames.size();
}

StackFrame* CallStack::top() const {
	if (!stack_frames.empty()) {
		return stack_frames.back();
	}
	return nullptr;
}