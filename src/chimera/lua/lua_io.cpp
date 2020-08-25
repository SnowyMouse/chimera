// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cstring>
#include "lua_io.hpp"

namespace Chimera {
    #define bail_if_not_unlocked \
        if((address < 0x40000000 || address > 0x41B00000) && script_from_state(state).sandbox) { \
            luaL_error(state, "invalid address specified (script is sandboxed)"); \
            return 0; \
        }

    template <typename T>
    static int lua_read_int(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            auto address = luaL_checkinteger(state, 1);
            lua_pushinteger(state, *reinterpret_cast<T *>(address));
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template <typename T>
    static int lua_write_int(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 2) {
            auto address = luaL_checkinteger(state, 1);
            bail_if_not_unlocked
            *reinterpret_cast<T *>(address) = static_cast<T>(luaL_checkinteger(state, 2));
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template <typename T>
    static int lua_read_float(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            auto address = luaL_checkinteger(state, 1);
            lua_pushnumber(state,*reinterpret_cast<T *>(address));
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template <typename T>
    static int lua_write_float(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 2) {
            auto address = luaL_checkinteger(state, 1);
            bail_if_not_unlocked
            *reinterpret_cast<T *>(address) = static_cast<T>(luaL_checknumber(state, 2));
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    static int lua_read_string8(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 1) {
            auto address = luaL_checkinteger(state, 1);
            lua_pushstring(state,reinterpret_cast<const char *>(address));
            return 1;
        }
        else{
            return luaL_error(state, "wrong number of arguments in read_string8");
        }
    }

    static int lua_write_string8(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 2) {
            auto address = luaL_checkinteger(state, 1);
            bail_if_not_unlocked
            auto *string = luaL_checkstring(state, 2);
            std::strcpy(reinterpret_cast<char *>(address), string);
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in write_string8");
        }
    }

    static int lua_read_bit(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 2) {
            auto address = luaL_checkinteger(state, 1);
            uint32_t &address_ptr = *reinterpret_cast<uint32_t *>(address);
            uint32_t bit = luaL_checkinteger(state,2);
            if(bit >= (sizeof(address_ptr) * 8)) {
                return luaL_error(state, "invalid argument #2 in read_bit");
            }
            lua_pushinteger(state, (address_ptr >> bit) & 1);
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments in read_bit");
        }
    }

    static int lua_write_bit(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 3) {
            auto address = luaL_checkinteger(state, 1);
            bail_if_not_unlocked
            auto &address_ptr = *reinterpret_cast<uint32_t *>(address);
            uint32_t bit = luaL_checkinteger(state,2);
            if(bit >= (sizeof(address_ptr) * 8)) {
                return luaL_error(state, "invalid argument #2 in write_bit");
            }
            char bit_to_set;
            if(lua_isboolean(state, 3)) bit_to_set = lua_toboolean(state, 3) == true;
            else {
                auto new_bit = luaL_checkinteger(state, 3);
                if(new_bit > 1) {
                    return luaL_error(state, "invalid argument #3 in write_bit");
                }
                bit_to_set = new_bit;
            }
            if(bit_to_set != ((address_ptr >> bit) & 1)) {
                switch((address_ptr >> bit) & 1) {
                    case 0:
                        address_ptr += (1 << bit);
                        break;
                    case 1:
                        address_ptr -= (1 << bit);
                        break;
                }
            }
            return 0;
        }
        else {
            return luaL_error(state,"wrong number of arguments in write_bit");
        }
    }

    void set_io_functions(lua_State *state) noexcept {
        #define lua_register_read_write_int_named(tname,ttype) \
            lua_register(state,"read_" tname,lua_read_int<ttype>); \
            lua_register(state,"write_" tname,lua_write_int<ttype>);

        #define lua_register_read_write_int(ttype) \
            lua_register_read_write_int_named(#ttype, ttype);

        #define lua_register_read_write_float_named(tname,ttype) \
            lua_register(state,"read_" tname,lua_read_float<ttype>); \
            lua_register(state,"write_" tname,lua_write_float<ttype>);

        #define lua_register_read_write_float(ttype) \
            lua_register_read_write_float_named(#ttype, ttype);

        lua_register_read_write_int_named("i8", int8_t);
        lua_register_read_write_int_named("i16", int16_t);
        lua_register_read_write_int_named("i32", int32_t);
        lua_register_read_write_int_named("u8", uint8_t);
        lua_register_read_write_int_named("u16", uint16_t);
        lua_register_read_write_int_named("u32", uint32_t);
        lua_register_read_write_int_named("byte", BYTE);
        lua_register_read_write_int_named("word", WORD);
        lua_register_read_write_int_named("dword", DWORD);
        lua_register_read_write_int(char);
        lua_register_read_write_int(short);
        lua_register_read_write_int(int);
        lua_register_read_write_int(long);
        lua_register_read_write_float_named("f32", float);
        lua_register_read_write_float_named("f64", double);
        lua_register_read_write_float(float);
        lua_register_read_write_float(double);
        lua_register(state, "read_bit", lua_read_bit);
        lua_register(state, "write_bit", lua_write_bit);
        lua_register(state, "read_string8", lua_read_string8);
        lua_register(state, "write_string8", lua_write_string8);
        lua_register(state, "read_string", lua_read_string8);
        lua_register(state, "write_string", lua_write_string8);
    }
}
