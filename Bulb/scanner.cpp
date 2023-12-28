#include <string>
#include "scanner.h"
#include <stringapiset.h>
#include <fstream>
using namespace std;

wstring stows(string& s) {
	wstring ws;
	uint len = MultiByteToWideChar(CP_UTF8, 0, &s[0], s.size(), 0, 0);
	ws.resize(len, 0);
	MultiByteToWideChar(CP_UTF8, 0, &s[0], s.size(), &ws[0], len);
	return ws;
}

wstring OpenFile(wstring dir) {
	fstream in(dir);
	if (!in.is_open()) {
		return L"";
	}
	in.seekg(0, ios::end);
	uint size = in.tellg();
	string str;
	str.resize(size);
	in.seekg(0, ios::beg);
	in.read(&str[0], size);

	return stows(str);
}

