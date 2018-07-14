//////////////////////////////////////////////////////////////////////////	
// dep: stl & apathy
#include <wdm.h>
#ifndef _EASTL
#define _EASTL
#endif
#include "path.hpp"

extern "C"
size_t path_sanitize(char* s, size_t sz, const char* base, const char* expr)
{
	eastl::string t;

	t.append(base);
	t.append("\\");
	t.append(expr);
	for (char& c : t)
	{
		if (c == '\\')
			c = '/';
	}

	apathy::Path path(t);
	t = path.sanitize().string();

	if (t.empty())
		return 0;

	for (char& c : t)
	{
		if (c == '/')
			c = '\\';
	}
	t.insert(0, "\\??\\");

	size_t size = t.size();
	const char* ptr = t.c_str();

	if (sz < size + 1)
		return 0;

	memcpy(s, ptr, size);
	s[size] = '\0';

	return size;
}
