#pragma once
#include "./lua_c_function.hpp"
#include "./lua_function.hpp"
#include "./lua_bind.hpp"
#include "./rb_tree.h"
#include "./memory_pool.h"

typedef struct st_lua_file* HLUAFILE;

extern HLUAFILE (create_lua_life)(void);
extern void (destroy_lua_life)(HLUAFILE LF);
class CLuaScript;

class CLuaException
{
public:
    explicit CLuaException(){m_what = 0;};
    ~CLuaException(){};

    const char* what() const
    {
        return m_what;
    }

    inline void set_what(const char* what)
    {
        m_what = what;
    }

    const char* m_what;
};

class ILuaEvent
{
public:
    virtual void on_error(CLuaScript* state, const char* err, size_t len, CLuaException& e) = 0;
    virtual void on_count_hook(CLuaScript* state, lua_Debug* ar) = 0;

    virtual void on_pre_call_c(CLuaScript* state, lua_c_function_base* c_func_ptr) = 0;
    virtual void on_post_call_c(CLuaScript* state, lua_c_function_base* c_func_ptr) = 0;
    virtual void on_pre_call_lua(CLuaScript* state, const char* func_name) = 0;
    virtual void on_post_call_lua(CLuaScript* state, const char* func_name) = 0;

    virtual lua_c_function_base* on_register_function(CLuaScript* state, const char* func_name) = 0;
    virtual lua_c_variable_base* on_register_variable(CLuaScript* state, const char* var_name) = 0;
    virtual void on_require_lua_path(CLuaScript* state, const char* require_lua, char* lua_path) = 0;
};

class CLuaScript
{
public:
    CLuaScript(ILuaEvent* evt = 0);
    ~CLuaScript();

    bool load_file(const char* file_name, HLUAFILE LF);
    bool load_buffer(const char* buffer, size_t buffer_size, const char* module_name, HLUAFILE LF);
    bool has_function(const char* lua_function_name);
    void close_lua(void);

    inline lua_State*& get_lua_state(void){return m_L;}
    inline ILuaEvent* get_event(void){return m_evt;}
    inline void set_event(ILuaEvent* lua_evt){m_evt = lua_evt;}
    inline void set_user_data(void* user_data){m_user_data = user_data;}
    inline void* get_user_data(void){return m_user_data;}

protected:
    bool _scan_lua_data(const char* lua_data, size_t lua_data_size, 
        HRBTREE register_c_func_list, HRBTREE unregister_c_func_list, 
        HRBTREE register_c_var_list, HRBTREE require_lua_list);
    void _scan_c_functions(const char* lua_data, size_t lua_data_size, HRBTREE register_c_func_list, HRBTREE unregister_c_func_list);
    void _scan_c_variables(const char* lua_data, size_t lua_data_size, HRBTREE register_c_var_list);
    void _scan_lua_require(const char* lua_data, size_t lua_data_size, HRBTREE require_lua_list);
    bool _load_lua(const char* lua_data, size_t lua_data_size, const char* chunk_name);
private:
    lua_State*      m_L;
    ILuaEvent*      m_evt;
    void*           m_user_data;
};

