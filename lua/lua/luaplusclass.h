#pragma once
#include <eastl/string.h>

namespace LuaPlus
{
#define LUA_SUCCEEDED(x)	((x) == LUA_OK)

/*
注册模块
可以注册静态和动态函数(类成员函数)
但注册动态函数后C++要保证此对象在LUA虚拟机结束之前一直存在
*/
class LuaModule
{
protected:
	LuaObject luaModule;

	void initialize(LuaObject parent, const char* name = NULL) {
		luaplus_assert(parent.IsTable());
		if (name == NULL) {
			luaModule = parent;
		}
		else {
			luaModule = parent[name];
			if (luaModule.IsNil() || !luaModule.IsTable()) {
				luaModule = parent.CreateTable(name);
			}
		}
	}
public:
	LuaModule(LuaObject parent, const char* name = NULL) {
		initialize(parent, name);
	}
	LuaModule(LuaState* ls, const char* name = NULL) {
		initialize(ls->GetGlobals(), name);
	}
	LuaModule(LuaModule& module, const char* name = NULL) {
		initialize(module.luaModule, name);
	}
	LuaObject GetLuaObject() {
		return luaModule;
	}
	// def
	inline LuaModule& def(const char* name, lua_CFunction func) {
		luaModule.Register(name, func);
		return *this;
	}

	template<typename Func>
	inline LuaModule& def(const char* name, Func func) {
		luaModule.RegisterDirect<Func>(name, func);
		return *this;
	}

	template<typename Object, typename Func>
	inline LuaModule& def(const char* name, Object& callee, Func func) {
		luaModule.RegisterDirect<Object, Func>(name, callee, func);
		return *this;
	}

	template<typename Object, typename Func>
	inline LuaModule& def(const char* name, Object *callee, Func func) {
		return def(name, *callee, func);
	}

	template<typename Object>
	inline LuaModule& def(const char* name, Object& callee, int (Object::*func)(LuaState*)) {
		luaModule.Register(name, callee, func);
		return *this;
	}

	template<typename Object>
	inline LuaModule& def(const char* name, Object *callee, int (Object::*func)(LuaState*)) {
		return def(name, *callee, func);
	}
};

class LuaVariant
{
protected:
	LuaObject luaObject;
public:
	LuaVariant(LuaObject object) : luaObject(object) {

	}
public: // basic
	operator bool() {
		return luaObject.GetBoolean();
	}
	operator lua_Integer() {
		return luaObject.GetInteger();
	}
	operator lua_Number() {
		return luaObject.GetNumber();
	}
	operator const char *() {
		return luaObject.GetString();
	}
public: // extra
	operator int() {
		return (int)luaObject.GetInteger();
	}
	operator long() {
		return (long)luaObject.GetInteger();
	}
	operator float() {
		return (float)luaObject.GetNumber();
	}
};

template<typename Object>
class LuaClassStructor
{
protected:
	static int ConstructorEx(LuaState* ls, Object *lpObject) {
		eastl::string metaName = LuaClass<Object>::GetMetaName().c_str();
		LuaObject metaTable = ls->GetGlobal(metaName.c_str());

		if (metaTable.IsNil() || !metaTable.IsTable() || metaTable.Get("__index").IsNil()) {
			ls->Error("Invalid class metatable.");
		}
		// 创建一个this指针容器, 它是个userdata
		LuaStackObject newObject = ls->BoxPointer(lpObject);
		// 赋予元表(function table), 即可索引方法
		newObject.SetMetatable(ls->Push(metaTable));
		ls->Pop();

		// 此时栈顶是BoxPointer, 返回它
		return 1;
	}
public:
	static int Constructor(LuaState* ls) {
		return ConstructorEx(ls, new Object());
	}
	template<typename A1>
	static int Constructor(LuaState* ls) {
		LuaVariant a1 = LuaObject(ls, 1);
		return ConstructorEx(ls, new Object((A1)a1));
	}
	template<typename A1, typename A2>
	static int Constructor(LuaState* ls) {
		LuaVariant a1 = LuaObject(ls, 1);
		LuaVariant a2 = LuaObject(ls, 2);
		return ConstructorEx(ls, new Object((A1)a1, (A2)a2));
	}

	template<typename A1, typename A2, typename A3>
	static int Constructor(LuaState* ls) {
		LuaVariant a1 = LuaObject(ls, 1);
		LuaVariant a2 = LuaObject(ls, 2);
		LuaVariant a3 = LuaObject(ls, 3);
		return ConstructorEx(ls, new Object((A1)a1, (A2)a2, (A3)a3));
	}
public:
	static int Destructor(LuaState* ls) {
		// 删除绑定的对象
		Object *object = (Object*)(ls->UnBoxPointer(1));
		delete object;

		// 删除元表, 即可失去调用函数的功能
		// 防止手动GC后再自动GC
		LuaObject(ls, 1).SetMetatable(ls->PushNil());
		ls->Pop();

		return 0;
	}
};

/*
注册类
必须定义公开的构造和析构函数
若构造函数有参数, 则必须为double /long long /const char* /bool
只能注册动态函数
*/
template<class T>
struct LuaClassTraits;
#define DECLARE_LUACLASS(CLS) \
template<> \
struct LuaPlus::LuaClassTraits<CLS> \
{ \
	constexpr static char* MetaName = #CLS; \
};

template<typename Object>
class LuaClass
{
public:
	static eastl::string GetMetaName() {
		eastl::string _meta_name = "vfTable_";
		_meta_name.append(LuaClassTraits<Object>::MetaName);
		return _meta_name;
	}
protected:
	LuaObject luaModule;
	LuaObject metaTable;

	void initialize(LuaObject parent) {
		luaplus_assert(parent.IsTable());
		luaModule = parent;

		// 获取元表
		metaTable = luaModule.ForceGetTable<const char*>(GetMetaName().c_str());
		metaTable.SetObject("__index", metaTable);
		metaTable.Register("__gc", LuaClassStructor<Object>::Destructor);
	}
public:
	LuaClass(LuaObject& parent) {
		initialize(parent);
	}
	LuaClass(LuaState *ls) {
		initialize(ls->GetGlobals());
	}
	LuaClass(LuaModule& parent) {
		initialize(parent.GetLuaObject());
	}
public:
	// 注册无参构造函数
	inline LuaClass& create(const char* name) {
		luaModule.Register(name, LuaClassStructor<Object>::Constructor);
		return *this;
	}
public:
	// 注册有参构造函数
	template<typename A1>
	inline LuaClass& create(const char* name) {
		luaModule.Register(name, LuaClassStructor<Object>::Constructor<A1>);
		return *this;
	}
	template<typename A1, typename A2>
	inline LuaClass& create(const char* name) {
		luaModule.Register(name, LuaClassStructor<Object>::Constructor<A1, A2>);
		return *this;
	}
	template<typename A1, typename A2, typename A3>
	inline LuaClass& create(const char* name) {
		luaModule.Register(name, LuaClassStructor<Object>::Constructor<A1, A2, A3>);
		return *this;
	}
public:
	// 注册成员函数
	template<typename Func>
	inline LuaClass& def(const char* name, Func func) {
		metaTable.RegisterObjectDirect<Object, Func>(name, (Object*)NULL, func);
		return *this;
	}
};


}; // LuaPlus
