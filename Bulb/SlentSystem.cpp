#include <string.h>
#include <iostream>
#include <string>
#include "Slent.h"
#include "SlentSystem.h"


using namespace std;

namespace Slent {

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

	void SlentSystem::OpenFiles() {

	}
}