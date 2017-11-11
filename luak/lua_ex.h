#pragma once

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

}; // LuaPlus
