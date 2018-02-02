#pragma once

class lua_c_lib
{
public:

    static lua_c_function_base* bind(void (*func_ptr)(void), const char* function_name)
    {
        return new lua_c_function<void, void>(func_ptr, function_name);
    }

    template<
        typename Result
    >
    static lua_c_function_base* bind(Result (*func_ptr)(void), const char* function_name)
    {
        return new lua_c_function<Result, void>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1), const char* function_name)
    {
        return new lua_c_function<Result, Arg1>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2,
        typename Arg3
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>(func_ptr, function_name);
    }

    template<
        typename Result,
        typename Arg1,
        typename Arg2,
        typename Arg3,
        typename Arg4,
        typename Arg5,
        typename Arg6,
        typename Arg7,
        typename Arg8
    >
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>(func_ptr, function_name);
    }

    template<
        typename Result,
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
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>(func_ptr, function_name);
    }

    template<
        typename Result,
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
    static lua_c_function_base* bind(Result (*func_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10), const char* function_name)
    {
        return new lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>(func_ptr, function_name);
    }
};