#include "SlentSystem.h"

using namespace std;
using namespace Slent;

SlentSystem::SlentSystem() {
	compiler = new SlentCompiler();
}

void SlentSystem::Run(int argc, char** argv) {
	if (argc <= 1) return;

	if (strcmp(argv[1], "run") == 0) {
		if (2 < argc) {
			string filename = string(argv[2]);
		}
		else {
			cout << colorString("Wrong command", RED) << endl;
			return;
		}

		for (int i = 3; i < argc; i++) {
			if (argv[i][0] != '-') {
				cout << colorString("Wrong command", RED) << endl;
				return;
			}
		}
		return;
	}

	if (strcmp(argv[1], "-version") == 0)
	{
		cout << "version: " << VERSION << endl;
		return;
	}
}

void SlentSystem::LoadFiles() {
	currentPath = filesystem::current_path();

	filesystem::path configFilePath;
	int configFileCount = 0; // .config 파일 수 카운트

	// 현재 디렉토리에서 .config 확장자를 가진 파일 찾기
	for (const auto& entry : filesystem::directory_iterator(currentPath)) {
		if (entry.is_regular_file() && entry.path().extension() == ".config") {
			configFilePath = entry.path(); // 첫 번째 .config 파일 경로 저장
			configFileCount++;
		}
	}

	if (configFileCount > 1) {
		cout << colorString("Multiple config files with .config extension found.", RED) << endl;
		return;
	}
	else if (configFileCount == 0) {
		cout << colorString("No config file with .config extension found.", RED) << endl;
		return;
	}
	LoadConfigFile(configFilePath);
}

void SlentSystem::LoadConfigFile(filesystem::path configFilePath) {
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(configFilePath.string().c_str());

	if (error != tinyxml2::XML_SUCCESS) {
		cout << colorString("Error while reading config file: " + string(doc.ErrorIDToName(error)), RED) << endl;
		return;
	}

	tinyxml2::XMLElement* root = doc.FirstChildElement("Slent");
	if (root) {
		tinyxml2::XMLElement* compileItems = root->FirstChildElement("CompileItems");
		if (compileItems) {
			for (tinyxml2::XMLElement* compile = compileItems->FirstChildElement("Compile");
				compile != nullptr;
				compile = compile->NextSiblingElement("Compile")) {
				const char* path = compile->Attribute("path");
				if (path) {
					filesystem::path sourceFilePath = currentPath / "src" / path;
					LoadSourceFile(sourceFilePath);
				}
			}
		}

		tinyxml2::XMLElement* dependencies = root->FirstChildElement("Dependencies");
		if (dependencies) {
			for (tinyxml2::XMLElement* library = dependencies->FirstChildElement("Library");
				library != nullptr;
				library = library->NextSiblingElement("Library")) {
				const char* path = library->Attribute("path");
				if (path) {
					filesystem::path libraryConfigFilePath = currentPath / "lib" / path;
					LoadConfigFile(libraryConfigFilePath);
				}
			}
		}
	}
	else {
		cout << colorString("<Slent> missing.", RED) << endl;
	}
}

void SlentSystem::LoadSourceFile(filesystem::path sourceFilePath) {
	if (filesystem::exists(sourceFilePath)) {
		string fileName = sourceFilePath.filename().string();
		ifstream file(sourceFilePath);
		if (file.is_open()) {
			string fullContent;
			string line;
			while (getline(file, line)) {
				fullContent += line;
			}
			file.close();

			compiler->AddFile(fileName, fullContent);
		}
		else {
			cout << colorString("Cannot open file named '" + fileName + "'.", RED) << endl;
		}
	}
	else {
		cout << colorString("File named '" + sourceFilePath.filename().string() + "' does not exist.", RED) << endl;
	}
}