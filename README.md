# Driver Plus Plus

Windows Kernel Driver with C++ runtime

|name|version|url|changes for dxx|
|-|-|-|-|
|EASTL|v3.10.00|https://github.com/electronicarts/EASTL|
|msgpack|v2.1.5|https://github.com/msgpack/msgpack-c|use eastl, ``throw`` instead of ``ExRaiseStatus``
|apathy||https://github.com/dlecocq/apathy|remove code based os
|lua|v5.3.5|https://www.lua.org/|utf-8 embedded, part of io lib, no os lib|
|luaplus||https://github.com/jjensen/luaplus51-all|
|pugixml|v1.8.1|https://github.com/zeux/pugixml|

## Build

VS2017 + SDK10.17763 + WDK10.17763

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

/*
local function main()
    local f, err = io.open("../dxx.log", "w");
    
    if f then
      
      for i = 1, 10, 1 do
        f:write(i .. "\r\n");
      end;
      
      f:close();
    end;
    
    f, err = io.open("../dxx.log", "r");
    
    if f then
      
      for l in f:lines() do
        print(l);
      end;
      
      f:close();
    end;
    
end;

main();
*/
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

void xml_test()
{
	pugi::xml_document doc;

/*
<?xml version='1.0' encoding='utf-8'?>

<root>
  <dxx a="1" b="2" c="3.3" d="5.6" e="Hello kitty!">
    <text><![CDATA[This is a CDATA String !!!]]></text>
  </dxx>
</root>
*/
	if (doc.load_file("././//System32/../../dxx.xml")) // equal = C:\\dxx.xml
	{
		pugi::xml_node root = doc.first_child();
		pugi::xml_node node = root.child("dxx");

		int a = node.attribute("a").as_int(0);
		__int64 b = node.attribute("b").as_llong(0);
		float c = node.attribute("c").as_float(0.0f);
		double d = node.attribute("d").as_double(0.0);
		eastl::string e = node.attribute("e").as_string("");
		eastl::string cdata = node.child("text").text().as_string("");

		// Driver cannot print float & double, view in WinDBG
		DbgPrintEx(DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, "%d %d %s %s\n", a, b, (c, d, e.c_str()), cdata.c_str());
	}
}

NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(DrvObject);
	UNREFERENCED_PARAMETER(RegPath);

	// NOTE OF PATH:
	// SEPARATOR = '/' !!!
	// BASE PATH = %SystemRoot% !!!

	test_global_class.foo();

	auto p1 = new CLIENT_ID[10];
	delete[] p1;

	stl_test();
	lua_test();
	pack_test();
	xml_test();

	return STATUS_UNSUCCESSFUL;
}
```
