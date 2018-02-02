#pragma once

#include "lua_table.hpp"
#include <string>

template<typename T>
class lua_data
{
public:
    static const char* c_type_name(void)
    {
        return "null";
    }

    static int lua_type(void)
    {
        return LUA_TNONE;
    }
};

template<>
class lua_data<lua_table>
{
    typedef lua_table data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return data_type(L, idx);
    }

    static void c_to_lua(lua_State* L, const data_type& v)
    {
        if (L != v.get_lua_state())
        {
            luaL_error(L,"\n\t<**系统错误**>\n\ttable的lua_State非法!");
        }

        if (lua_gettop(L) == v.get_stack_index())
        {
            return;
        }

        lua_pushvalue(L, v.get_stack_index());
        lua_remove(L, v.get_stack_index());
    }

    static const char* c_type_name(void)
    {
        return "table";
    }

    static int lua_type(void)
    {
        return LUA_TTABLE;
    }
};

template<>
class lua_data<const lua_table&>
{
public:
    static lua_table lua_to_c(lua_State* L, int idx)
    {
        return lua_table(L, idx);
    }

    static void c_to_lua(lua_State* L, const lua_table& v)
    {
        if (L != v.get_lua_state())
        {
            luaL_error(L,"\n\t<**系统错误**>\n\ttable的lua_State非法!");
        }

        if (lua_gettop(L) == v.get_stack_index())
        {
            return;
        }

        lua_pushvalue(L, v.get_stack_index());
        lua_remove(L, v.get_stack_index());
    }

    static const char* c_type_name(void)
    {
        return "const lua_table&";
    }

    static int lua_type(void)
    {
        return LUA_TTABLE;
    }
};

template<>
class lua_data<char>
{
    typedef char data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "char";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<signed char>
{
    typedef signed char data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "char";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<unsigned char>
{
    typedef unsigned char data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "unsigned char";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<short>
{
    typedef short data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "short";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<unsigned short>
{
    typedef unsigned short data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "unsigned short";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<int>
{
    typedef int data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "int";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<unsigned int>
{
    typedef unsigned int data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tointeger(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushinteger(L, v);
    }

    static const char* c_type_name(void)
    {
        return "unsigned int";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<double>
{
    typedef double data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (data_type)lua_tonumber(L, idx);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushnumber(L, v);
    }

    static const char* c_type_name(void)
    {
        return "double";
    }

    static int lua_type(void)
    {
        return LUA_TNUMBER;
    }
};

template<>
class lua_data<const char*>
{
public:
    static const char* lua_to_c(lua_State* L, int idx)
    {
        return lua_tostring(L, idx);
    }

    static void c_to_lua(lua_State* L, const char* v)
    {
        lua_pushstring(L, v);
    }

    static const char* c_type_name(void)
    {
        return "const char*";
    }

    static int lua_type(void)
    {
        return LUA_TSTRING;
    }
};

template<>
class lua_data<char*>
{
    typedef char* data_type;
public:
    static char* lua_to_c(lua_State* L, int idx)
    {
        return const_cast<data_type>(lua_tostring(L, idx));
    }

    static void c_to_lua(lua_State* L, char* v)
    {
        lua_pushstring(L, v);
    }

    static const char* c_type_name(void)
    {
        return "char*";
    }

    static int lua_type(void)
    {
        return LUA_TSTRING;
    }
};

template<>
class lua_data<std::string>
{
public:
    static std::string lua_to_c(lua_State* L, int idx)
    {
        size_t len = 0;
        const char* ptr = lua_tolstring(L, idx, &len);

        return std::string(ptr, len);
    }

    static void c_to_lua(lua_State* L, const std::string& v)
    {
        lua_pushlstring(L, v.c_str(), v.length());
    }

    static const char* c_type_name(void)
    {
        return "std::string";
    }

    static int lua_type(void)
    {
        return LUA_TSTRING;
    }
};

template<>
class lua_data<const std::string>
{
public:
    static const std::string lua_to_c(lua_State* L, int idx)
    {
        size_t len = 0;
        const char* ptr = lua_tolstring(L, idx, &len);

        return std::string(ptr, len);
    }

    static void c_to_lua(lua_State* L, const std::string& v)
    {
        lua_pushlstring(L, v.c_str(), v.length());
    }

    static const char* c_type_name(void)
    {
        return "const std::string";
    }

    static int lua_type(void)
    {
        return LUA_TSTRING;
    }
};

template<>
class lua_data<std::string&>
{
public:
    static std::string lua_to_c(lua_State* L, int idx)
    {
        size_t len = 0;
        const char* ptr = lua_tolstring(L, idx, &len);

        return std::string(ptr, len);
    }

    static void c_to_lua(lua_State* L, const std::string& v)
    {
        lua_pushlstring(L, v.c_str(), v.length());
    }

    static const char* c_type_name(void)
    {
        return "std::string&";
    }

    static int lua_type(void)
    {
        return LUA_TSTRING;
    }
};

template<>
class lua_data<const std::string&>
{
public:
    static std::string lua_to_c(lua_State* L, int idx)
    {
        size_t len = 0;
        const char* ptr = lua_tolstring(L, idx, &len);

        return std::string(ptr, len);
    }

    static void c_to_lua(lua_State* L, const std::string& v)
    {
        lua_pushlstring(L, v.c_str(), v.length());
    }

    static const char* c_type_name(void)
    {
        return "const std::string&";
    }

    static int lua_type(void)
    {
        return LUA_TSTRING;
    }
};

template<>
class lua_data<bool>
{
    typedef bool data_type;
public:
    static data_type lua_to_c(lua_State* L, int idx)
    {
        return (lua_toboolean(L, idx) != 0);
    }

    static void c_to_lua(lua_State* L, data_type v)
    {
        lua_pushboolean(L, v ? 1 : 0);
    }

    static const char* c_type_name(void)
    {
        return "bool";
    }

    static int lua_type(void)
    {
        return LUA_TBOOLEAN;
    }
};

template<>
class lua_data<void>
{
public:

    static const char* c_type_name(void)
    {
        return "void";
    }

    static int lua_type(void)
    {
        return LUA_TNIL;
    }
};