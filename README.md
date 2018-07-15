# Driver Plus Plus

Windows Kernel Driver with C++ runtime

EASTL 3.10.00

apathy

lua 5.3.5 (utf-8 embedded, base io lib, no os lib)

msgpack 2.1.5

## Build

VS2015 + SDK10.14393 + WDK10.14393

## Example

```C++
class ThisIsAClass {
public:
	ThisIsAClass() {
		DbgPrint("%s\n", __FUNCTION__);
	}
	virtual ~ThisIsAClass() {
		DbgPrint("%s\n", __FUNCTION__);
	}
public:
	void foo() {
		DbgPrint("%s\n", __FUNCTION__);
	}
};

ThisIsAClass test_global_class;

void DbgPrintS(const char* s) {
	DbgPrint("%s\n", s);
}

struct TestPack
{
	int a;
	char b;
	bool c;
	float d;
	double e;
	eastl::string f;
	eastl::vector<eastl::string> g;

	MSGPACK_DEFINE(a, b, c, d, e, f, g);
};

void stl_test()
{
	eastl::make_unique<DRIVER_OBJECT>();
	eastl::make_shared<UNICODE_STRING>();
	eastl::scoped_ptr<double> dptr(new double(3.6));

	eastl::set<int> set_test;
	set_test.insert(1);
	set_test.insert(3);
	set_test.insert(5);
	set_test.erase(1);

	eastl::map<int, int> map_test;
	map_test[0] = 1;
	map_test[10] = 11;
	map_test[20] = 12;
	map_test.erase(11);

	eastl::vector<int> vec_test;
	vec_test.push_back(2);
	vec_test.push_back(3);
	vec_test.push_back(1);
	eastl::stable_sort(vec_test.begin(), vec_test.end(), eastl::less<int>());
	for (auto e : vec_test) {
		DbgPrint("%d\n", e);
	}

	eastl::string s;
	s = "This a string";
	s.append("any");
	DbgPrint("%s\n", s.c_str());

	eastl::wstring ws;
	ws = L"wide string";
	ws.clear();

	eastl::unordered_set<float> us_test;
	us_test.insert(333);

	eastl::unordered_map<double, eastl::string> um_test;
	um_test.insert(eastl::make_pair(6.6, "9.9"));
}

void lua_test()
{
	LuaPlus::LuaStateAuto ls(LuaPlus::LuaState::Create(true));
	{
		LuaPlus::LuaModule _G(ls->GetGlobals());

		_G.def("foo", &test_global_class, &ThisIsAClass::foo);
		_G.def("DbgPrint", &DbgPrintS);
	}

	if (ls->LoadString("foo();\nDbgPrint([[Hello World]]);") == LUA_OK) {
		ls->PCall(0, 0, 0);
	}

	// SEPARATOR = '/' !!!
	// BASE PATH = %SystemRoot% !!!
	if (ls->LoadFile("../dxx.lua") == LUA_OK) // equal = C:\\dxx.lua
	{
		ls->PCall(0, 0, 0);
	}
}

void pack_test()
{
	TestPack tp;
	tp.a = 1;
	tp.b = '0';
	tp.c = false;
	tp.d = 666.666f;
	tp.e = 520.1314;
	tp.f = "ansi or utf8 string";
	tp.g.push_back("many strings");

	msgpack::sbuffer sb;
	msgpack::pack(sb, tp);
	TestPack tp2 = msgpack::unpack(sb.data(), sb.size()).get().as<TestPack>();
}

NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(DrvObject);
	UNREFERENCED_PARAMETER(RegPath);

	test_global_class.foo();

	auto p1 = new CLIENT_ID[10];
	delete[] p1;

	stl_test();
	lua_test();
	pack_test();

	return STATUS_UNSUCCESSFUL;
}
```
