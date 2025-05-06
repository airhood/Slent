
#include "SlentVM.h"
#include <string>

using namespace Slent;

int main() {
	SlentVM* slentVM = new SlentVM();
	VMSetting setting = VMSetting();
	setting.stack_size = 1024;
	setting.max_heap_size = 1024 * 100;

	slentVM->ConfigureSetting(setting);

	std::string bytecode = "goto 2\nprocess exit 0\nprocess exit 1\nprocess exit 2";

	slentVM->Run(bytecode);
}