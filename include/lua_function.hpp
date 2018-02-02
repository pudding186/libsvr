#pragma once

#include "lua_data.hpp"

#define begin_call_lua(L, function_name)    \
    lua_call_lua_check check(L, function_name);\
    lua_getglobal(L, function_name);\
    if (LUA_TFUNCTION != lua_type(L, -1))\
    {\
    lua_pop(L, 1);\
    luaL_error(L, "\n\t<**c++回调lua**>\n\t调用函数%s失败:该函数不存在!", function_name);\
    }

#define end_call_lua(L, argn, function_name)    lua_call(L, argn, 1);\
    if (lua_type(L, -1) != lua_data<Result>::lua_type())\
    {\
    const char* real_return_type = lua_typename(L, lua_type(L, -1));\
    lua_pop(L, -1);\
    luaL_error(L, "\n\t<**c++回调lua**>\n\t调用函数%s失败:期望返回类型%s 实际返回 %s",\
    function_name, lua_data<Result>::c_type_name(), real_return_type);\
    }\
    Result t = lua_data<Result>::lua_to_c(L, -1);\
    lua_pop(L, 1);\
    return t;

#define end_call_lua_no_return(L, argn, function_name)  lua_call(L, argn, 0);

class lua_call_lua_check
{
public:
    lua_call_lua_check(lua_State* L, const char* func_name);
    ~lua_call_lua_check(void);
protected:
    const char* m_func_name;
    lua_State*  m_L;
};

//////////////////////////////////////////////////////////////////////////

template <typename Result>
class lua_function
{
public:
    static Result call(lua_State* L, const char* function_name)
    {
        begin_call_lua(L, function_name);
        end_call_lua(L, 0, function_name);
    }

    template<typename Arg1>
    static Result call(lua_State* L, const char* function_name, Arg1 arg1)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        end_call_lua(L, 1, function_name);
    }

    template<
        typename Arg1,
        typename Arg2
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        end_call_lua(L, 2, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        end_call_lua(L, 3, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        end_call_lua(L, 4, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        end_call_lua(L, 5, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        end_call_lua(L, 6, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        end_call_lua(L, 7, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        end_call_lua(L, 8, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        end_call_lua(L, 9, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9,
        typename Arg10
    >
    static Result call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        lua_data<Arg10>::c_to_lua(L, arg10);
        end_call_lua(L, 10, function_name);
    }

    template<typename Arg1>
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        end_call_lua(L, 1, function_name);
    }

    template<
        typename Arg1,
        typename Arg2
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        end_call_lua(L, 2, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        end_call_lua(L, 3, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        end_call_lua(L, 4, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        end_call_lua(L, 5, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        end_call_lua(L, 6, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        end_call_lua(L, 7, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7, Arg8& arg8)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        end_call_lua(L, 8, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7, Arg8& arg8, Arg9& arg9)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        end_call_lua(L, 9, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9,
        typename Arg10
    >
    static Result call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7, Arg8& arg8, Arg9& arg9, Arg10& arg10)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        lua_data<Arg10>::c_to_lua(L, arg10);
        end_call_lua(L, 10, function_name);
    }

    lua_function(lua_State* L, const char* function_name):
    m_L(L), m_argn(0), m_function_name(function_name), m_check(L, function_name)
    {
        lua_getglobal(L, function_name);
        if (LUA_TFUNCTION != lua_type(L, -1))
        {
            lua_pop(L, 1);
            luaL_error(L, "\n\t<**c++回调lua**>\n\t调用函数%s失败:该函数不存在!", function_name);
        }
    }

    template<typename T>
    void push_param(T t)
    {
        ++m_argn;
        lua_data<T>::c_to_lua(m_L, t);
    }

    template<typename T>
    void push_param_ex(T& t)
    {
        ++m_argn;
        lua_data<T>::c_to_lua(m_L, t);
    }

    Result call(void)
    {
        end_call_lua(m_L, m_argn, m_function_name)
    }
private:
    lua_State*  m_L;
    int         m_argn;
    const char* m_function_name;
    lua_call_lua_check  m_check;
};

template <>
class lua_function<void>
{
public:
    static void call(lua_State* L, const char* function_name)
    {
        begin_call_lua(L, function_name);
        end_call_lua_no_return(L, 0, function_name);
    }

    template<typename Arg1>
    static void call(lua_State* L, const char* function_name, Arg1 arg1)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        end_call_lua_no_return(L, 1, function_name);
    }

    template<
        typename Arg1,
        typename Arg2
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        end_call_lua_no_return(L, 2, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        end_call_lua_no_return(L, 3, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        end_call_lua_no_return(L, 4, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        end_call_lua_no_return(L, 5, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        end_call_lua_no_return(L, 6, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        end_call_lua_no_return(L, 7, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        end_call_lua_no_return(L, 8, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        end_call_lua_no_return(L, 9, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9,
        typename Arg10
    >
    static void call(lua_State* L, const char* function_name, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        lua_data<Arg10>::c_to_lua(L, arg10);
        end_call_lua_no_return(L, 10, function_name);
    }

    template<typename Arg1>
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        end_call_lua_no_return(L, 1, function_name);
    }

    template<
        typename Arg1,
        typename Arg2
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        end_call_lua_no_return(L, 2, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        end_call_lua_no_return(L, 3, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        end_call_lua_no_return(L, 4, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        end_call_lua_no_return(L, 5, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        end_call_lua_no_return(L, 6, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        end_call_lua_no_return(L, 7, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7, Arg8& arg8)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        end_call_lua_no_return(L, 8, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7, Arg8& arg8, Arg9& arg9)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        end_call_lua_no_return(L, 9, function_name);
    }

    template<
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8,
        typename Arg9,
        typename Arg10
    >
    static void call_ex(lua_State* L, const char* function_name, Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6, Arg7& arg7, Arg8& arg8, Arg9& arg9, Arg10& arg10)
    {
        begin_call_lua(L, function_name);
        lua_data<Arg1>::c_to_lua(L, arg1);
        lua_data<Arg2>::c_to_lua(L, arg2);
        lua_data<Arg3>::c_to_lua(L, arg3);
        lua_data<Arg4>::c_to_lua(L, arg4);
        lua_data<Arg5>::c_to_lua(L, arg5);
        lua_data<Arg6>::c_to_lua(L, arg6);
        lua_data<Arg7>::c_to_lua(L, arg7);
        lua_data<Arg8>::c_to_lua(L, arg8);
        lua_data<Arg9>::c_to_lua(L, arg9);
        lua_data<Arg10>::c_to_lua(L, arg10);
        end_call_lua_no_return(L, 10, function_name);
    }

    lua_function(lua_State* L, const char* function_name):
    m_L(L), m_argn(0), m_function_name(function_name), m_check(L, function_name)
    {
        lua_getglobal(L, function_name);
        if (LUA_TFUNCTION != lua_type(L, -1))
        {
            lua_pop(L, 1);
            luaL_error(L, "\n\t<**c++回调lua**>\n\t调用函数%s失败:该函数不存在!", function_name);
        }
    }

    template<typename T>
    void push_param(T t)
    {
        ++m_argn;
        lua_data<T>::c_to_lua(m_L, t);
    }

    template<typename T>
    void push_param_ex(T& t)
    {
        ++m_argn;
        lua_data<T>::c_to_lua(m_L, t);
    }

    void call(void)
    {
        end_call_lua_no_return(m_L, m_argn, m_function_name)
    }

private:
    lua_State*  m_L;
    int         m_argn;
    const char* m_function_name;
    lua_call_lua_check  m_check;
};