#include <ntddk.h>
#include <kstl.hpp>

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

NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(DrvObject);
	UNREFERENCED_PARAMETER(RegPath);

	test_global_class.foo();

	auto p1 = new CLIENT_ID[10];
	delete p1;

	stl::make_unique<DRIVER_OBJECT>();
	stl::make_shared<UNICODE_STRING>();
	stl::scoped_ptr<double> dptr(new double(3.6));

	stl::set<int> set_test;
	set_test.insert(1);
	set_test.insert(3);
	set_test.insert(5);
	set_test.erase(1);

	stl::map<int, int> map_test;
	map_test[0] = 1;
	map_test[10] = 11;
	map_test[20] = 12;
	map_test.erase(11);

	stl::vector<int> vec_test;
	vec_test.push_back(2);
	vec_test.push_back(3);
	vec_test.push_back(1);
	stl::stable_sort(vec_test.begin(), vec_test.end(), stl::less<int>());
	for (auto e : vec_test) {
		DbgPrint("%d\n", e);
	}

	stl::string s;
	s = "This a string";
	s.append("any");
	DbgPrint("%s\n", s.c_str());

	stl::wstring ws;
	ws = L"wide string";
	ws.clear();

	stl::unordered_set<float> us_test;
	us_test.insert(333);

	stl::unordered_map<double, stl::string> um_test;
	um_test.insert(stl::make_pair(6.6, "9.9"));

	

	return STATUS_UNSUCCESSFUL;
}

