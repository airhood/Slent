#include <iostream>
#include <string>
#include "SlentSystem.h"

using namespace std;

int main(int argc, char** argv) {
	Slent::SlentSystem system = Slent::SlentSystem();
	system.Run(argc, argv);

#ifdef RELEASE
	return 0;
#endif

#ifdef _DEBUG
	string exampleCode;
	//string exampleCode = "#if LEL\n#endif\n#define TEST\n\n#if TEST\nimport Test.UnitTest.ITestable;\n#else\n#error \"test is required\"\n#endif\nimport System;\n\nclass Foo {\n    var public int moo;\n    var public int foo;\n    \n    construct(int _moo, int _foo, int _boo) {\n        moo = _moo;\n        foo = _foo;\n        var boo = _boo + 1;\n        foo += boo;\n        string str = \"int\";\n }\n    \n    func ConvertTo16() -> string { \n        return Boo + 1; \n }\n}\n\nclass Main{\n    \n    @Entry\n    func main() { \n        Foo foo = new Foo(10, 7, 1); \n        foo.ConvertTo16(); \n        \n        string str = \"+\";\n        \n        if (true) {\n            \n        }\n    }\n}";
	//string exampleCode = "#if LEL\n#endif\n#define TEST\n\n#if L_TEST\nimport Test.UnitTest.ITestable;\n#else\n#error \"test is required\"\n#endif\nimport System;\n\nclass Foo {\n\tvar public int moo;\n\tvar public int foo;\n\t\n\tconstruct(int _moo, int _foo, int _boo) {\n\t\tmoo = _moo;\n\t\tfoo = _foo;\n\t\tvar boo = _boo + 1;\n\t\tfoo += boo;\n\t}\n\t\n\tfunc ConvertTo16() -> string {\n\t\treturn Boo + 1;\n\t}\n}\n\nclass Main {\n\t\n\t@Entry\n\tfunc main() {\n\t\tFoo foo = new Foo(10, 7, 1);\n\t\tfoo.ConvertTo16();\n\t\t\n\t\tstring str = \"+\";\n\t\t\n\t\tif (true) {\n\t\t\t\n\t\t}\n\t}\n}";

#endif

	return 0;
}