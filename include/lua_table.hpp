#pragma once

#include "../lib3rd/lua/include/lua.h"
#include "../lib3rd/lua/include/lualib.h"
#include "../lib3rd/lua/include/lauxlib.h"

#include <string>

class lua_c_string 
{
public:
    const char* m_ptr;
    size_t      m_len;

    lua_c_string(void)
    {
        m_ptr = 0;
        m_len = 0;
    }

    lua_c_string(const char* ptr, size_t len)
    {
        m_ptr = ptr;
        m_len = len;
    }
};

class lua_table
{
public:
    class lua_dataproxy;
    friend lua_dataproxy;
    class lua_dataproxy
    {
    public:
        lua_dataproxy(lua_table* inst, const std::string& strkey) : 
          _inst(inst), _key_str_ptr(strkey.c_str()),_key_str_len(strkey.length()), _isdb(false)
          {

          }

          lua_dataproxy(lua_table* inst, double dbkey) : 
          _inst(inst), _dbkey(dbkey), _isdb(true)
          {

          }

          ~lua_dataproxy()
          {
              _inst = 0;
          }

    public:
        void operator = (double value)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_pushnumber(_inst->m_lua_state, value);
            lua_settable(_inst->m_lua_state, _inst->m_stack_index);
        }

        void operator = (const std::string& value)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_pushlstring(_inst->m_lua_state, value.c_str(), value.length());
            lua_settable(_inst->m_lua_state, _inst->m_stack_index);
        }

        void operator = (const lua_table& t)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_pushvalue(_inst->m_lua_state, t.m_stack_index);
            lua_remove(_inst->m_lua_state, t.m_stack_index);
            lua_settable(_inst->m_lua_state, _inst->m_stack_index);
        }

        double getnumber(void)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_gettable(_inst->m_lua_state, _inst->m_stack_index);
            double value = luaL_checknumber(_inst->m_lua_state, -1);

            lua_pop(_inst->m_lua_state, 1);
            return value;
        }

        int getinteger(void)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_gettable(_inst->m_lua_state, _inst->m_stack_index);
            int value = luaL_checkinteger(_inst->m_lua_state, -1);

            lua_pop(_inst->m_lua_state, 1);
            return value;
        }

        std::string getstring(void)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_gettable(_inst->m_lua_state, _inst->m_stack_index);
            std::string value(luaL_checkstring(_inst->m_lua_state, -1));

            lua_pop(_inst->m_lua_state, 1);
            return value;
        }

        lua_table gettable(void)
        {
            if (_isdb)
                lua_pushnumber(_inst->m_lua_state, _dbkey);
            else
                lua_pushlstring(_inst->m_lua_state, _key_str_ptr, _key_str_len);

            lua_gettable(_inst->m_lua_state, _inst->m_stack_index);
            luaL_checktype(_inst->m_lua_state, -1, LUA_TTABLE);

            return lua_table(_inst->m_lua_state, lua_gettop(_inst->m_lua_state));
        }

    private:
        lua_table* _inst;
        //std::string _strkey;
        const char* _key_str_ptr;
        size_t      _key_str_len;
        double _dbkey;
        bool _isdb;
    };

public:
    static lua_table NewTable(lua_State* L)
    {
        lua_newtable(L);
        return lua_table(L, lua_gettop(L));
    }

public:
    lua_table(lua_State* L, int stk_idx) : 
      m_lua_state(L), m_stack_index(stk_idx)
      {

      }

      lua_table() : m_lua_state(0), m_stack_index(0)
      {

      }

      lua_table(const lua_table& lhs)
      {
          m_lua_state = lhs.m_lua_state;
          m_stack_index = lhs.m_stack_index;
      }

      ~lua_table()
      {
          m_lua_state = 0;
          m_stack_index = 0;
      }

      inline lua_State* get_lua_state(void) const
      {
          return m_lua_state;
      }

      inline int get_stack_index(void) const
      {
          return m_stack_index;
      }

      bool isvalid(void)
      {
          return (m_lua_state != 0) && (m_stack_index != 0);
      }

public:

    void set_int(double key, lua_Integer value)
    {
        lua_pushnumber(m_lua_state, key);
        lua_pushinteger(m_lua_state, value);
        lua_settable(m_lua_state, m_stack_index);
    }

    void set_int(const char* key, lua_Integer value, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }
        lua_pushinteger(m_lua_state, value);
        lua_settable(m_lua_state, m_stack_index);
    }

    void set_double(double key, double value)
    {
        lua_pushnumber(m_lua_state, key);
        lua_pushnumber(m_lua_state, value);
        lua_settable(m_lua_state, m_stack_index);
    }

    void set_double(const char* key, double value, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }
        lua_pushnumber(m_lua_state, value);
        lua_settable(m_lua_state, m_stack_index);
    }

    void set_string(double key, const char* value, size_t length = 0)
    {
        lua_pushnumber(m_lua_state, key);
        if (length)
        {
            lua_pushlstring(m_lua_state, value, length);
        }
        else
        {
            lua_pushstring(m_lua_state, value);
        }
        lua_settable(m_lua_state, m_stack_index);
    }
    void set_string(const char* key, const char* value, size_t key_length = 0, size_t length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }

        if (length)
        {
            lua_pushlstring(m_lua_state, value, length);
        }
        else
        {
            lua_pushstring(m_lua_state, value);
        }
        lua_settable(m_lua_state, m_stack_index);
    }

    void set_table(double key, lua_table& value)
    {
        lua_pushnumber(m_lua_state, key);
        lua_pushvalue(m_lua_state, value.m_stack_index);
        lua_remove(m_lua_state, value.m_stack_index);
        lua_settable(m_lua_state, m_stack_index);
    }

    void set_table(const char* key, lua_table& value, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }

        lua_pushvalue(m_lua_state, value.m_stack_index);
        lua_remove(m_lua_state, value.m_stack_index);
        lua_settable(m_lua_state, m_stack_index);
    }

    lua_Integer get_int(double key)
    {
        lua_pushnumber(m_lua_state, key);
        lua_gettable(m_lua_state, m_stack_index);
        lua_Integer value = luaL_checkinteger(m_lua_state, -1);

        lua_pop(m_lua_state, 1);

        return value;
    }

    lua_Integer get_int(const char* key, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }

        lua_gettable(m_lua_state, m_stack_index);
        lua_Integer value = luaL_checkinteger(m_lua_state, -1);

        lua_pop(m_lua_state, 1);

        return value;
    }

    double get_double(double key)
    {
        lua_pushnumber(m_lua_state, key);
        lua_gettable(m_lua_state, m_stack_index);
        double value = luaL_checknumber(m_lua_state, -1);

        lua_pop(m_lua_state, 1);
        return value;
    }

    double get_double(const char* key, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }

        lua_gettable(m_lua_state, m_stack_index);
        double value = luaL_checknumber(m_lua_state, -1);

        lua_pop(m_lua_state, 1);
        return value;
    }

    size_t get_string(double key, const char** value)
    {
        lua_pushnumber(m_lua_state, key);
        lua_gettable(m_lua_state, m_stack_index);

        size_t length = 0;

        *value = luaL_checklstring(m_lua_state, -1, &length);

        lua_pop(m_lua_state, 1);

        return length;
    }

    size_t get_string(const char* key, const char** value, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }

        lua_gettable(m_lua_state, m_stack_index);

        size_t length = 0;

        *value = luaL_checklstring(m_lua_state, -1, &length);

        lua_pop(m_lua_state, 1);

        return length;
    }

    lua_table get_table(double key)
    {
        lua_pushnumber(m_lua_state, key);
        lua_gettable(m_lua_state, m_stack_index);

        luaL_checktype(m_lua_state, -1, LUA_TTABLE);

        return lua_table(m_lua_state, lua_gettop(m_lua_state));
    }

    lua_table get_table(const char* key, size_t key_length = 0)
    {
        if (key_length)
        {
            lua_pushlstring(m_lua_state, key, key_length);
        }
        else
        {
            lua_pushstring(m_lua_state, key);
        }

        lua_gettable(m_lua_state, m_stack_index);

        luaL_checktype(m_lua_state, -1, LUA_TTABLE);

        return lua_table(m_lua_state, lua_gettop(m_lua_state));
    }


    lua_dataproxy operator[] (double key) const
    {
        return lua_dataproxy(const_cast<lua_table*>(this), key);
    }

    lua_dataproxy operator[] (const std::string& key) const
    {
        return lua_dataproxy(const_cast<lua_table*>(this), key);
    }

    size_t size(void) const
    {
        return lua_objlen(m_lua_state, m_stack_index);
    }

    bool empty(void) const
    {
        lua_pushnil(m_lua_state);
        bool bEmpty = (0 == lua_next(m_lua_state, m_stack_index));

        lua_pop(m_lua_state, 2);
        return bEmpty;
    }

    void clear()
    {
        lua_remove(m_lua_state, m_stack_index);
        m_stack_index = 0; 
    }

private:
    lua_State* m_lua_state;
    int m_stack_index;
};