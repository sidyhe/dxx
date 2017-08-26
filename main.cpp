#include <ntddk.h>

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

	return STATUS_UNSUCCESSFUL;
}

