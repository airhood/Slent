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
		bool LoadFiles(std::string path);
		bool LoadConfigFile(std::filesystem::path configFilePath);
		bool LoadSourceFile(std::filesystem::path sourceFilePath);
		
		//std::filesystem::path currentPath;

		SlentCompiler* compiler;
		CompilerSetting setting;
	};
}