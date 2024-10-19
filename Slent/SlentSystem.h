#pragma once

#include "SlentVM.h"
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "tinyxml2.h"

namespace Slent {

	class SlentSystem {
	public:
		SlentSystem();

		void Run(int argc, char** argv);

	private:
		void LoadFiles();
		void LoadConfigFile();
		void LoadSourceFiles();
		
		std::filesystem::path currentPath;
		
		std::vector<std::string> filePaths;

		SlentCompiler* compiler;

	};
}