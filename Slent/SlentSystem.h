#pragma once

#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "tinyxml2.h"

#include "Slent.h"

namespace Slent {

	class SlentSystem {
	public:
		SlentSystem();

		void Run(int argc, char** argv);

	private:
		void LoadFiles(std::string path);
		void LoadConfigFile(std::filesystem::path configFilePath);
		void LoadSourceFile(std::filesystem::path sourceFilePath);
		
		//std::filesystem::path currentPath;

		SlentCompiler* compiler;
		CompilerSetting setting;
	};
}