#include <ntddk.h>

class ThisIsAClass {
public:
	ThisIsAClass() {
		KdBreakPoint();
	}
	virtual ~ThisIsAClass() {
		KdBreakPoint();
	}
};

ThisIsAClass test_global_class;

NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(DrvObject);
	UNREFERENCED_PARAMETER(RegPath);

	auto p1 = new CLIENT_ID[10];
	delete p1;
	auto p2 = new ThisIsAClass();
	delete p2;

	return STATUS_UNSUCCESSFUL;
}

