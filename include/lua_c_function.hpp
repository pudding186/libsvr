#pragma once

#include "lua_data.hpp"

class lua_c_variable_base
{
public:
    lua_c_variable_base(){};
    virtual ~lua_c_variable_base(){};

    const char*     m_c_variable_name;
    const char*     m_var_str;
    int             m_var_int;
    int             m_var_type;//0=int 1=str
};

class lua_c_function_base
{
public:
    lua_c_function_base(){};
    virtual ~lua_c_function_base(){};

    lua_CFunction   m_c_func;
    const char*     m_c_function_name;
};

class lua_call_c_check
{
public:
    lua_call_c_check(lua_State* L, lua_c_function_base* c_func_ptr);
    ~lua_call_c_check(void);
protected:
    lua_c_function_base*    m_func_ptr;
    lua_State*  m_L;
};

template<
typename Result,
typename Arg1 = lua_Integer,
typename Arg2 = lua_Integer,
typename Arg3 = lua_Integer,
typename Arg4 = lua_Integer,
typename Arg5 = lua_Integer,
typename Arg6 = lua_Integer,
typename Arg7 = lua_Integer,
typename Arg8 = lua_Integer,
typename Arg9 = lua_Integer,
typename Arg10 = lua_Integer
>
class lua_c_function : public lua_c_function_base
{
    typedef Result (*func_ptr1)(Arg1);
    typedef Result (*func_ptr2)(Arg1, Arg2);
    typedef Result (*func_ptr3)(Arg1, Arg2, Arg3);
    typedef Result (*func_ptr4)(Arg1, Arg2, Arg3, Arg4);
    typedef Result (*func_ptr5)(Arg1, Arg2, Arg3, Arg4, Arg5);
    typedef Result (*func_ptr6)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);
    typedef Result (*func_ptr7)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);
    typedef Result (*func_ptr8)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    typedef Result (*func_ptr9)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);
    typedef Result (*func_ptr10)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10);
public:
    lua_c_function(func_ptr1& func_ptr, const char* function_name):m_func_ptr1(func_ptr), m_argn(1)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr2& func_ptr, const char* function_name):m_func_ptr2(func_ptr), m_argn(2)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr3& func_ptr, const char* function_name):m_func_ptr3(func_ptr), m_argn(3)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr4& func_ptr, const char* function_name):m_func_ptr4(func_ptr), m_argn(4)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr5& func_ptr, const char* function_name):m_func_ptr5(func_ptr), m_argn(5)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr6& func_ptr, const char* function_name):m_func_ptr6(func_ptr), m_argn(6)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr7& func_ptr, const char* function_name):m_func_ptr7(func_ptr), m_argn(7)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr8& func_ptr, const char* function_name):m_func_ptr8(func_ptr), m_argn(8)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr9& func_ptr, const char* function_name):m_func_ptr9(func_ptr), m_argn(9)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr10& func_ptr, const char* function_name):m_func_ptr10(func_ptr), m_argn(10)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };

    static int lua_call_back(lua_State* L)
    {
        lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* func_ptr = 
            (lua_c_function<Result, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>*)lua_touserdata(L, lua_upvalueindex(1));

        lua_call_c_check check(L, func_ptr);

        int push_argn = lua_gettop(L);

        if (push_argn < (func_ptr->m_argn+1))
        {
            char function_describe[256];
            func_ptr->get_function_describe(function_describe, sizeof(function_describe));

            luaL_error(L, "\n\t<**函数 %s 调用出错**>\n\t需要参数:#%d\n\t实际参数:#%d\n\t函数原型:%s.",
                func_ptr->m_c_function_name,
                func_ptr->m_argn,
                push_argn - 1,
                function_describe);
        }

        for (int i = 1; i <= func_ptr->m_argn; i++)
        {
            switch (i)
            {
            case 1:
                {
                    if (lua_type(L, i+1) != lua_data<Arg1>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 2:
                {
                    if (lua_type(L, i+1) != lua_data<Arg2>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 3:
                {
                    if (lua_type(L, i+1) != lua_data<Arg3>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 4:
                {
                    if (lua_type(L, i+1) != lua_data<Arg4>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 5:
                {
                    if (lua_type(L, i+1) != lua_data<Arg5>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 6:
                {
                    if (lua_type(L, i+1) != lua_data<Arg6>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 7:
                {
                    if (lua_type(L, i+1) != lua_data<Arg7>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 8:
                {
                    if (lua_type(L, i+1) != lua_data<Arg8>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 9:
                {
                    if (lua_type(L, i+1) != lua_data<Arg9>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 10:
                {
                    if (lua_type(L, i+1) != lua_data<Arg10>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            default:
                break;
            }
        }

        switch (func_ptr->m_argn)
        {
        case 1:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr1)(
                    lua_data<Arg1>::lua_to_c(L, 2)));
            }
            break;
        case 2:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr2)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3)));
            }
            break;
        case 3:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr3)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4)));
            }
            break;
        case 4:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr4)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5)));
            }
            break;
        case 5:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr5)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6)));
            }
            break;
        case 6:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr6)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7)));
            }
            break;
        case 7:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr7)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8)));
            }
            break;
        case 8:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr8)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8),
                    lua_data<Arg8>::lua_to_c(L, 9)));
            }
            break;
        case 9:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr9)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8),
                    lua_data<Arg8>::lua_to_c(L, 9),
                    lua_data<Arg9>::lua_to_c(L, 10)));
            }
            break;
        case 10:
            {
                lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr10)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8),
                    lua_data<Arg8>::lua_to_c(L, 9),
                    lua_data<Arg9>::lua_to_c(L, 10),
                    lua_data<Arg10>::lua_to_c(L, 11)));
            }
            break;
        }

        return 1;
    }

    void trace_param_error(int index, lua_State* L)
    {
        char function_describe[256];
        get_function_describe(function_describe, sizeof(function_describe));
        const char* push_param_type_name = lua_typename(L, lua_type(L, index+1));

        const char* param_type_name;

        switch (index)
        {
        case 1:
            {
                param_type_name = lua_data<Arg1>::c_type_name();
            }
            break;
        case 2:
            {
                param_type_name = lua_data<Arg2>::c_type_name();
            }
            break;
        case 3:
            {
                param_type_name = lua_data<Arg3>::c_type_name();
            }
            break;
        case 4:
            {
                param_type_name = lua_data<Arg4>::c_type_name();
            }
            break;
        case 5:
            {
                param_type_name = lua_data<Arg5>::c_type_name();
            }
            break;
        case 6:
            {
                param_type_name = lua_data<Arg6>::c_type_name();
            }
            break;
        case 7:
            {
                param_type_name = lua_data<Arg7>::c_type_name();
            }
            break;
        case 8:
            {
                param_type_name = lua_data<Arg8>::c_type_name();
            }
            break;
        case 9:
            {
                param_type_name = lua_data<Arg9>::c_type_name();
            }
            break;
        case 10:
            {
                param_type_name = lua_data<Arg10>::c_type_name();
            }
            break;
        default:
            {
                param_type_name = "unknown";
            }
            break;
        }

        luaL_error(L, "\n\t<**函数 %s 调用出错**>\n\t第%d个参数类型 %s 实际脚本传入类型 %s\n\t函数原型:%s",
            m_c_function_name,
            index,
            param_type_name,
            push_param_type_name,
            function_describe);
    }

    void get_function_describe(char* buff, size_t size)
    {
#define TRY_COPY(data, data_size) append_len += data_size;\
    if (append_len >= size)\
        {\
        memcpy(ptr, data, data_size+size-append_len);\
        buff[size] = 0;\
        return;\
    }\
                            else\
        {\
        memcpy(ptr, data, data_size);\
        ptr += data_size;\
    }


        char* ptr = buff;
        size_t append_len = 0;

        TRY_COPY(lua_data<Result>::c_type_name(), strlen(lua_data<Result>::c_type_name()))
            TRY_COPY(" ", strlen(" "))
            TRY_COPY(m_c_function_name, strlen(m_c_function_name))
            TRY_COPY("(", strlen("("))

            for (int i = 1; i <= m_argn; i++)
            {
                switch (i)
                {
                case 1:
                    {
                        TRY_COPY(lua_data<Arg1>::c_type_name(), strlen(lua_data<Arg1>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 2:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg2>::c_type_name(), strlen(lua_data<Arg2>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 3:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg3>::c_type_name(), strlen(lua_data<Arg3>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 4:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg4>::c_type_name(), strlen(lua_data<Arg4>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 5:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg5>::c_type_name(), strlen(lua_data<Arg5>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 6:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg6>::c_type_name(), strlen(lua_data<Arg6>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 7:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg7>::c_type_name(), strlen(lua_data<Arg7>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 8:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg8>::c_type_name(), strlen(lua_data<Arg8>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 9:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg9>::c_type_name(), strlen(lua_data<Arg9>::c_type_name()))
                    }
                    break;
                case 10:
                    {
                        TRY_COPY(lua_data<Arg10>::c_type_name(), strlen(lua_data<Arg10>::c_type_name()))
                    }
                    break;
                default:
                    buff[size] = 0;
                    return;
                }

                if (i != (m_argn))
                {
                    TRY_COPY(", ", strlen(", "))
                }
            }

            TRY_COPY(");", strlen(");"))
                *ptr = 0;
    }
    union
    {
        func_ptr1   m_func_ptr1;
        func_ptr2   m_func_ptr2;
        func_ptr3   m_func_ptr3;
        func_ptr4   m_func_ptr4;
        func_ptr5   m_func_ptr5;
        func_ptr6   m_func_ptr6;
        func_ptr7   m_func_ptr7;
        func_ptr8   m_func_ptr8;
        func_ptr9   m_func_ptr9;
        func_ptr10  m_func_ptr10;
    };
    int m_argn;
};

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
class lua_c_function<void, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>: public lua_c_function_base
{
    typedef void (*func_ptr1)(Arg1);
    typedef void (*func_ptr2)(Arg1, Arg2);
    typedef void (*func_ptr3)(Arg1, Arg2, Arg3);
    typedef void (*func_ptr4)(Arg1, Arg2, Arg3, Arg4);
    typedef void (*func_ptr5)(Arg1, Arg2, Arg3, Arg4, Arg5);
    typedef void (*func_ptr6)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);
    typedef void (*func_ptr7)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);
    typedef void (*func_ptr8)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    typedef void (*func_ptr9)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);
    typedef void (*func_ptr10)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10);
public:
    lua_c_function(func_ptr1& func_ptr, const char* function_name):m_func_ptr1(func_ptr), m_argn(1)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr2& func_ptr, const char* function_name):m_func_ptr2(func_ptr), m_argn(2)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr3& func_ptr, const char* function_name):m_func_ptr3(func_ptr), m_argn(3)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr4& func_ptr, const char* function_name):m_func_ptr4(func_ptr), m_argn(4)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr5& func_ptr, const char* function_name):m_func_ptr5(func_ptr), m_argn(5)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr6& func_ptr, const char* function_name):m_func_ptr6(func_ptr), m_argn(6)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr7& func_ptr, const char* function_name):m_func_ptr7(func_ptr), m_argn(7)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr8& func_ptr, const char* function_name):m_func_ptr8(func_ptr), m_argn(8)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr9& func_ptr, const char* function_name):m_func_ptr9(func_ptr), m_argn(9)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };
    lua_c_function(func_ptr10& func_ptr, const char* function_name):m_func_ptr10(func_ptr), m_argn(10)
    {
        m_c_function_name =function_name;
        m_c_func = &lua_call_back;
    };

    static int lua_call_back(lua_State* L)
    {
        lua_c_function<void, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* func_ptr = 
            (lua_c_function<void, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>*)lua_touserdata(L, lua_upvalueindex(1));

        lua_call_c_check check(L, func_ptr);

        int push_argn = lua_gettop(L);

        if (push_argn < (func_ptr->m_argn+1))
        {
            char function_describe[256];
            func_ptr->get_function_describe(function_describe, sizeof(function_describe));

            luaL_error(L, "\n\t<**函数 %s 调用出错**>\n\t需要参数:#%d\n\t实际参数:#%d\n\t函数原型:%s.",
                func_ptr->m_c_function_name,
                func_ptr->m_argn,
                push_argn - 1,
                function_describe);
        }

        for (int i = 1; i <= func_ptr->m_argn; i++)
        {
            switch (i)
            {
            case 1:
                {
                    if (lua_type(L, i+1) != lua_data<Arg1>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 2:
                {
                    if (lua_type(L, i+1) != lua_data<Arg2>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 3:
                {
                    if (lua_type(L, i+1) != lua_data<Arg3>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 4:
                {
                    if (lua_type(L, i+1) != lua_data<Arg4>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 5:
                {
                    if (lua_type(L, i+1) != lua_data<Arg5>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 6:
                {
                    if (lua_type(L, i+1) != lua_data<Arg6>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 7:
                {
                    if (lua_type(L, i+1) != lua_data<Arg7>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 8:
                {
                    if (lua_type(L, i+1) != lua_data<Arg8>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 9:
                {
                    if (lua_type(L, i+1) != lua_data<Arg9>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            case 10:
                {
                    if (lua_type(L, i+1) != lua_data<Arg10>::lua_type())
                    {
                        func_ptr->trace_param_error(i, L);
                    }
                }
                break;
            default:
                break;
            }
        }

        switch (func_ptr->m_argn)
        {
        case 1:
            {
                (*func_ptr->m_func_ptr1)(
                    lua_data<Arg1>::lua_to_c(L, 2));
            }
            break;
        case 2:
            {
                (*func_ptr->m_func_ptr2)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3));
            }
            break;
        case 3:
            {
                (*func_ptr->m_func_ptr3)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4));
            }
            break;
        case 4:
            {
                (*func_ptr->m_func_ptr4)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5));
            }
            break;
        case 5:
            {
                (*func_ptr->m_func_ptr5)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6));
            }
            break;
        case 6:
            {
                (*func_ptr->m_func_ptr6)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7));
            }
            break;
        case 7:
            {
                (*func_ptr->m_func_ptr7)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8));
            }
            break;
        case 8:
            {
                (*func_ptr->m_func_ptr8)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8),
                    lua_data<Arg8>::lua_to_c(L, 9));
            }
            break;
        case 9:
            {
                (*func_ptr->m_func_ptr9)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8),
                    lua_data<Arg8>::lua_to_c(L, 9),
                    lua_data<Arg9>::lua_to_c(L, 10));
            }
            break;
        case 10:
            {
                (*func_ptr->m_func_ptr10)(
                    lua_data<Arg1>::lua_to_c(L, 2),
                    lua_data<Arg2>::lua_to_c(L, 3),
                    lua_data<Arg3>::lua_to_c(L, 4),
                    lua_data<Arg4>::lua_to_c(L, 5),
                    lua_data<Arg5>::lua_to_c(L, 6),
                    lua_data<Arg6>::lua_to_c(L, 7),
                    lua_data<Arg7>::lua_to_c(L, 8),
                    lua_data<Arg8>::lua_to_c(L, 9),
                    lua_data<Arg9>::lua_to_c(L, 10),
                    lua_data<Arg10>::lua_to_c(L, 11));
            }
            break;
        }

        return 0;
    }

    void trace_param_error(int index, lua_State* L)
    {
        char function_describe[256];
        get_function_describe(function_describe, sizeof(function_describe));
        const char* push_param_type_name = lua_typename(L, lua_type(L, index+1));

        const char* param_type_name;

        switch (index)
        {
        case 1:
            {
                param_type_name = lua_data<Arg1>::c_type_name();
            }
            break;
        case 2:
            {
                param_type_name = lua_data<Arg2>::c_type_name();
            }
            break;
        case 3:
            {
                param_type_name = lua_data<Arg3>::c_type_name();
            }
            break;
        case 4:
            {
                param_type_name = lua_data<Arg4>::c_type_name();
            }
            break;
        case 5:
            {
                param_type_name = lua_data<Arg5>::c_type_name();
            }
            break;
        case 6:
            {
                param_type_name = lua_data<Arg6>::c_type_name();
            }
            break;
        case 7:
            {
                param_type_name = lua_data<Arg7>::c_type_name();
            }
            break;
        case 8:
            {
                param_type_name = lua_data<Arg8>::c_type_name();
            }
            break;
        case 9:
            {
                param_type_name = lua_data<Arg9>::c_type_name();
            }
            break;
        case 10:
            {
                param_type_name = lua_data<Arg10>::c_type_name();
            }
            break;
        default:
            {
                param_type_name = "unknown";
            }
            break;
        }

        luaL_error(L, "\n\t<**函数 %s 调用出错**>\n\t第%d个参数类型 %s 实际脚本传入类型 %s\n\t函数原型:%s",
            m_c_function_name,
            index,
            param_type_name,
            push_param_type_name,
            function_describe);
    }


    void get_function_describe(char* buff, size_t size)
    {
#define TRY_COPY(data, data_size) append_len += data_size;\
    if (append_len >= size)\
        {\
        memcpy(ptr, data, data_size+size-append_len);\
        buff[size] = 0;\
        return;\
    }\
                            else\
        {\
        memcpy(ptr, data, data_size);\
        ptr += data_size;\
    }


        char* ptr = buff;
        size_t append_len = 0;

        TRY_COPY("void ", strlen("void "))
            TRY_COPY(m_c_function_name, strlen(m_c_function_name))
            TRY_COPY("(", strlen("("))

            for (int i = 1; i <= m_argn; i++)
            {
                switch (i)
                {
                case 1:
                    {
                        TRY_COPY(lua_data<Arg1>::c_type_name(), strlen(lua_data<Arg1>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 2:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg2>::c_type_name(), strlen(lua_data<Arg2>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 3:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg3>::c_type_name(), strlen(lua_data<Arg3>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 4:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg4>::c_type_name(), strlen(lua_data<Arg4>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 5:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg5>::c_type_name(), strlen(lua_data<Arg5>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 6:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg6>::c_type_name(), strlen(lua_data<Arg6>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 7:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg7>::c_type_name(), strlen(lua_data<Arg7>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 8:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg8>::c_type_name(), strlen(lua_data<Arg8>::c_type_name()))
                    }                                                              
                    break;                                                         
                case 9:                                                            
                    {                                                              
                        TRY_COPY(lua_data<Arg9>::c_type_name(), strlen(lua_data<Arg9>::c_type_name()))
                    }
                    break;
                case 10:
                    {
                        TRY_COPY(lua_data<Arg10>::c_type_name(), strlen(lua_data<Arg10>::c_type_name()))
                    }
                    break;
                default:
                    buff[size] = 0;
                    return;
                }

                if (i != (m_argn))
                {
                    TRY_COPY(", ", strlen(", "))
                }
            }

            TRY_COPY(");", strlen(");"))
                *ptr = 0;
    }

    union
    {
        func_ptr1   m_func_ptr1;
        func_ptr2   m_func_ptr2;
        func_ptr3   m_func_ptr3;
        func_ptr4   m_func_ptr4;
        func_ptr5   m_func_ptr5;
        func_ptr6   m_func_ptr6;
        func_ptr7   m_func_ptr7;
        func_ptr8   m_func_ptr8;
        func_ptr9   m_func_ptr9;
        func_ptr10  m_func_ptr10;
    };
    int m_argn;
};

template<typename Result>
class lua_c_function<Result, void>: public lua_c_function_base
{
    typedef Result (*func_ptr)(void);
public:
    lua_c_function(func_ptr& ptr, const char* function_name):m_func_ptr(ptr)
    {
        m_c_function_name = function_name;
        m_c_func = &lua_call_back;
    }

    static int lua_call_back(lua_State* L)
    {
        lua_c_function<Result, void>* func_ptr = 
            (lua_c_function<Result, void>*)lua_touserdata(L, lua_upvalueindex(1));

        lua_call_c_check check(L, func_ptr);

        lua_data<Result>::c_to_lua(L, (*func_ptr->m_func_ptr)());

        return 1;
    }

    func_ptr   m_func_ptr;
};

template<>
class lua_c_function<void, void>: public lua_c_function_base
{
    typedef void(*func_ptr)(void);
public:
    lua_c_function(func_ptr& ptr, const char* function_name):m_func_ptr(ptr)
    {
        m_c_function_name = function_name;
        m_c_func = &lua_call_back;
    }

    static int lua_call_back(lua_State* L)
    {
        lua_c_function<void, void>* func_ptr = 
            (lua_c_function<void, void>*)lua_touserdata(L, lua_upvalueindex(1));

        lua_call_c_check check(L, func_ptr);

        (*func_ptr->m_func_ptr)();

        return 0;
    }

    func_ptr    m_func_ptr;
};