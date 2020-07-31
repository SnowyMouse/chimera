#include "io.hpp"
#include <cstdint>
#include <lauxlib.h>
#include <codecvt>
#include <locale>
#include "../signature/hook.hpp"

namespace Chimera {
    template <typename T, typename lua_type, void (*pushval)(lua_State *, lua_type)> static int read_number(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 1) {
            pushval(state, *reinterpret_cast<const T *>(luaL_checkinteger(state, 1)));
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template <typename T, typename lua_type, lua_type (*checkval)(lua_State *, int), bool virtual_protect_allowed> static int write_number(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 2 || (virtual_protect_allowed && args == 3)) {
            bool virtual_protect = false;
            if(args == 3) {
                virtual_protect = lua_toboolean(state, 3);
            }
            auto &from = *reinterpret_cast<T *>(luaL_checkinteger(state, 1));
            auto to = static_cast<T>(checkval(state, 2));
            if(!virtual_protect) {
                from = to;
            }
            else {
                overwrite(&from, to);
            }
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    static int read_bit(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 2) {
            auto *from = reinterpret_cast<std::uint8_t *>(luaL_checkinteger(state, 1));
            auto bit = luaL_checkinteger(state, 2);
            from += bit / 8;
            lua_pushinteger(state, (*from >> bit) & 1);
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template <bool virtual_protect_allowed> static int write_bit(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 3 || (virtual_protect_allowed && args == 4)) {
            auto *from = reinterpret_cast<std::uint8_t *>(luaL_checkinteger(state, 1));
            auto bit = luaL_checkinteger(state, 2);
            auto bit_value = luaL_checkinteger(state, 3) & 1;
            from += bit / 8;
            auto to = static_cast<std::uint8_t>((*from & ~(1 << (bit % 8))) | (bit_value << (bit % 8)));
            bool virtual_protect = false;
            if(args == 4) {
                virtual_protect = lua_toboolean(state, 4);
            }
            if(!virtual_protect) {
                *from = to;
            }
            else {
                overwrite(from, to);
            }
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template<typename T> static void write_any_string(T *to, const T *from, bool virtual_protect) {
        if(!virtual_protect) {
            while(*from) {
                *to = *from;
                to++;
                from++;
            }
        }
        else {
            while(*from) {
                overwrite(to, *from);
                to++;
                from++;
            }
        }
    }

    static int read_char_string(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 1 ) {
            lua_pushstring(state, reinterpret_cast<const char *>(luaL_checkinteger(state, 1)));
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template<bool virtual_protect_allowed> static int write_char_string(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 2 || args == 3) {
            bool virtual_protect = false;
            if(args == 3) {
                virtual_protect = lua_toboolean(state, 3);
            }
            write_any_string(reinterpret_cast<char *>(luaL_checkinteger(state, 1)), luaL_checkstring(state, 2), virtual_protect);
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    static int read_wide_char_string(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 1) {
            auto *str_in = reinterpret_cast<const wchar_t *>(luaL_checkinteger(state, 1));

            // Convert UTF-16 to UTF-8
            char string[512] = {};
            WideCharToMultiByte(CP_UTF8, 0, str_in, -1, string, sizeof(string) / sizeof(*string), nullptr, nullptr);
            lua_pushstring(state, string);

            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    template<bool virtual_protect_allowed> static int write_wide_char_string(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 2 || (virtual_protect_allowed && args == 3)) {
            bool virtual_protect = false;
            if(args == 3) {
                virtual_protect = lua_toboolean(state, 3);
            }

            auto *str_out = reinterpret_cast<const wchar_t *>(luaL_checkinteger(state, 1));

            // Convert UTF-8 to UTF-16
            wchar_t string[512] = {};
            MultiByteToWideChar(CP_UTF8, 0, luaL_checkstring(state, 2), -1, string, sizeof(string) / sizeof(*string));

            write_any_string(string, str_out, virtual_protect);
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    int read_wstring(lua_State *state) {
        int args = lua_gettop(state);
        if(args == 1) {
            lua_pushstring(state, reinterpret_cast<char *>(luaL_checkinteger(state, 1)));
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments");
        }
    }

    void set_up_io_functions(lua_State *state, unsigned int api_version) noexcept {
        // Define read/write functions
        auto read_i8 = read_number<std::int8_t, lua_Integer, lua_pushinteger>;
        auto read_u8 = read_number<std::uint8_t, lua_Integer, lua_pushinteger>;
        auto read_i16 = read_number<std::int16_t, lua_Integer, lua_pushinteger>;
        auto read_u16 = read_number<std::uint16_t, lua_Integer, lua_pushinteger>;
        auto read_i32 = read_number<std::int32_t, lua_Integer, lua_pushinteger>;
        auto read_u32 = read_number<std::uint32_t, lua_Integer, lua_pushinteger>;
        auto read_f32 = read_number<float, lua_Number, lua_pushnumber>;
        auto read_f64 = read_number<double, lua_Number, lua_pushnumber>;

        using lua_call = int(*)(lua_State *);
        lua_call write_i8, write_u8, write_i16, write_u16, write_i32, write_u32, write_f32, write_f64;
        lua_call write_char_string_fn, write_wide_char_string_fn;
        lua_call write_bit_fn;

        // If API version 3 or newer, allow VirtualProtect stuff
        if(api_version >= 3) {
            write_i8  = write_number<std::int8_t, lua_Integer, luaL_checkinteger, true>;
            write_u8  = write_number<std::uint8_t, lua_Integer, luaL_checkinteger, true>;
            write_i16 = write_number<std::int16_t, lua_Integer, luaL_checkinteger, true>;
            write_u16 = write_number<std::uint16_t, lua_Integer, luaL_checkinteger, true>;
            write_i32 = write_number<std::int32_t, lua_Integer, luaL_checkinteger, true>;
            write_u32 = write_number<std::uint32_t, lua_Integer, luaL_checkinteger, true>;
            write_f32 = write_number<float, lua_Number, luaL_checknumber, true>;
            write_f64 = write_number<double, lua_Number, luaL_checknumber, true>;
            write_char_string_fn = write_char_string<true>;
            write_wide_char_string_fn = write_wide_char_string<true>;
            write_bit_fn = write_wide_char_string<true>;
        }
        else {
            write_i8  = write_number<std::int8_t, lua_Integer, luaL_checkinteger, false>;
            write_u8  = write_number<std::uint8_t, lua_Integer, luaL_checkinteger, false>;
            write_i16 = write_number<std::int16_t, lua_Integer, luaL_checkinteger, false>;
            write_u16 = write_number<std::uint16_t, lua_Integer, luaL_checkinteger, false>;
            write_i32 = write_number<std::int32_t, lua_Integer, luaL_checkinteger, false>;
            write_u32 = write_number<std::uint32_t, lua_Integer, luaL_checkinteger, false>;
            write_f32 = write_number<float, lua_Number, luaL_checknumber, false>;
            write_f64 = write_number<double, lua_Number, luaL_checknumber, false>;
            write_char_string_fn = write_char_string<false>;
            write_wide_char_string_fn = write_wide_char_string<false>;
            write_bit_fn = write_wide_char_string<false>;
        }

        // Register read/write values
        #define LUA_REGISTER_READ_WRITE_VAL(name, fnr, fnw) \
        lua_register(state, "read_" name, fnr); \
        lua_register(state, "write_" name, fnw)

        LUA_REGISTER_READ_WRITE_VAL("byte", read_u8, write_u8);
        LUA_REGISTER_READ_WRITE_VAL("u8", read_u8, write_u8);
        LUA_REGISTER_READ_WRITE_VAL("word", read_u16, write_u16);
        LUA_REGISTER_READ_WRITE_VAL("u16", read_u16, write_u16);
        LUA_REGISTER_READ_WRITE_VAL("dword", read_u32, write_u32);
        LUA_REGISTER_READ_WRITE_VAL("u32", read_u32, write_u32);
        LUA_REGISTER_READ_WRITE_VAL("char", read_i8, write_i8);
        LUA_REGISTER_READ_WRITE_VAL("i8", read_i8, write_i8);
        LUA_REGISTER_READ_WRITE_VAL("short", read_i16, write_i16);
        LUA_REGISTER_READ_WRITE_VAL("i16", read_i16, write_i16);
        LUA_REGISTER_READ_WRITE_VAL("int", read_i32, write_i32);
        LUA_REGISTER_READ_WRITE_VAL("long", read_i32, write_i32);
        LUA_REGISTER_READ_WRITE_VAL("i32", read_i32, write_i32);
        LUA_REGISTER_READ_WRITE_VAL("float", read_f32, write_f32);
        LUA_REGISTER_READ_WRITE_VAL("f32", read_f32, write_f32);
        LUA_REGISTER_READ_WRITE_VAL("double", read_f64, write_f64);
        LUA_REGISTER_READ_WRITE_VAL("f64", read_f64, write_f64);
        LUA_REGISTER_READ_WRITE_VAL("string", read_char_string, write_char_string_fn);
        LUA_REGISTER_READ_WRITE_VAL("string8", read_char_string, write_char_string_fn);
        LUA_REGISTER_READ_WRITE_VAL("wide_string", read_wide_char_string, write_wide_char_string_fn);
        LUA_REGISTER_READ_WRITE_VAL("string16", read_wide_char_string, write_wide_char_string_fn);
        LUA_REGISTER_READ_WRITE_VAL("bit", read_bit, write_bit_fn);

        #undef LUA_REGISTER_READ_WRITE_VAL
    }
}
