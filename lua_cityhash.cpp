/* -*-coding:utf-8-*- */

/* lua cityhash - lua bindings for google cityhash algorithm
 *
 * Copyright (c) 2013- hepengfei <hepengfei@xunlei.com>
 * 
 */


/* lua header */
#include "lua.hpp"

/* library header */
#include <endian.h>             // NOTE: Important! this is linux
                                // specifics.
#include <string.h>
#include "city.h"

#define MODULE_NAME "cityhash"

/* 需要导出的函数，都以MODULE_NAME为前缀 */

static void dump32(char *str, uint32 value)
{
        const uint32 value_be = htobe32(value);
        memcpy(str, &value_be, sizeof(value));
}

static void dump64(char *str, uint64 value)
{
        const uint64 value_be = htobe64(value);
        memcpy(str, &value_be, sizeof(value));
}

static void dump128(char *str, uint128 value)
{
        // bigendian, high bits first
        dump64(str, Uint128High64(value));
        dump64(str + sizeof(uint64), Uint128Low64(value));
}

// static uint32 load32(const char *str)
// {
//         uint32 value;
//         memcpy(&value, str, sizeof(value));
//         return be32toh(value);
// }

static uint64 load64(const char *str)
{
        uint64 value;
        memcpy(&value, str, sizeof(value));
        return be64toh(value);
}

static uint128 load128(const char *str)
{
        // bigendian, high bits first
        uint64 high64 = load64(str);
        uint64 low64 = load64(str + sizeof(uint64));
        return uint128(low64, high64);
}

static uint64 luaX_checkuint64(lua_State *L, int index)
{
        if(lua_type(L, index) == LUA_TSTRING)
        {
                size_t len;
                const char *str = lua_tolstring(L, index, &len);
                if(len != sizeof(uint64))
                {
                        return luaL_error(L, "arg#%d must be %d length",
                                          index, sizeof(uint64));
                }
                return load64(str);
        }
        else
        {
                return luaL_checkinteger(L, index);
        }
}

static uint128 luaX_checkuint128(lua_State *L, int index)
{
        size_t len;
        const char *str = luaL_checklstring(L, index, &len);
        if(len != sizeof(uint64)*2)
        {
                luaL_error(L, "arg#%d must be %d length",
                           index, sizeof(uint64)*2);
                return uint128(0, 0); // never goes here.
        }
        return load128(str);
}

static int cityhash_cityhash64(lua_State *L)
{
        size_t len = 0;
        const char *buf = luaL_checklstring(L, 1, &len);

        uint64 value = 0;
        int narg = lua_gettop(L);
        if(narg == 1)
        {
                value = CityHash64(buf, len);
        }
        else if(narg == 2)
        {
                if(lua_type(L, 2) == LUA_TSTRING)
                {
                        size_t lenarg;
                        const char *str = lua_tolstring(L, 2, &lenarg);
                        if(lenarg == sizeof(uint64))
                        {
                                uint64 seed = load64(str);
                                value = CityHash64WithSeed(buf, len, seed);
                        }
                        else if(lenarg == sizeof(uint64)*2)
                        {
                                uint128 seed = load128(str);
                                uint64 seed0 = Uint128Low64(seed);
                                uint64 seed1 = Uint128High64(seed);
                                value = CityHash64WithSeeds(buf, len, seed0, seed1);
                        }
                        else
                        {
                                return luaL_error(L, "arg#2 must be 8/16 bytes length");
                        }
                }
                else
                {
                        uint64 seed = luaL_checkinteger(L, 2);
                        value = CityHash64WithSeed(buf, len, seed);
                }
        }
        else
        {
                uint64 seed0 = luaX_checkuint64(L, 2);
                uint64 seed1 = luaX_checkuint64(L, 3);
                value = CityHash64WithSeeds(buf, len, seed0, seed1);
        }

        char str[sizeof(uint64)];
        dump64(str, value);
        lua_pushlstring(L, str, sizeof(str));

        return 1;               /* 返回值的个数 */
}

static int cityhash_cityhash128(lua_State *L)
{
        size_t len = 0;
        const char *buf = luaL_checklstring(L, 1, &len);

        uint128 value = uint128(0, 0);
        int narg = lua_gettop(L);
        if(narg == 1)
        {
                value = CityHash128(buf, len);
        }
        else if(narg == 2)
        {
                uint128 seed = luaX_checkuint128(L, 2);
                value = CityHash128WithSeed(buf, len, seed);
        }
        else
        {
                uint64 seedlow64 = luaX_checkuint64(L, 2);
                uint64 seedhigh64 = luaX_checkuint64(L, 3);
                uint128 seed = uint128(seedlow64, seedhigh64);
                value = CityHash128WithSeed(buf, len, seed);
        }

        char str[sizeof(uint64)*2];
        dump128(str, value);
        lua_pushlstring(L, str, sizeof(str));

        return 1;               /* 返回值的个数 */
}

static int cityhash_cityhash32(lua_State *L)
{
        size_t len = 0;
        const char *buf = luaL_checklstring(L, 1, &len);

        uint32 value = CityHash32(buf, len);

        char str[sizeof(uint32)];
        dump32(str, value);
        lua_pushlstring(L, str, sizeof(str));

        return 1;               /* 返回值的个数 */
}

static int cityhash_hash128to64(lua_State *L)
{
        uint128 value128 = luaX_checkuint128(L, 1);
        uint64 value64 = Hash128to64(value128);

        char str64[sizeof(uint64)];
        dump64(str64, value64);
        lua_pushlstring(L, str64, sizeof(str64));

        return 1;               /* 返回值的个数 */
}

static int cityhash_testdata(lua_State *L)
{
        const uint64 k0 = 0xc3a5c85c97cb3127ULL;
        const uint64 kSeed0 = 1234567;
        const uint64 kSeed1 = k0;
        const uint128 kSeed128(kSeed0, kSeed1);
        const int kDataSize = 1 << 20;

        char data[kDataSize];

        uint64 a = 9;
        uint64 b = 777;
        for (int i = 0; i < kDataSize; i++) {
                a += b;
                b += a;
                a = (a ^ (a >> 41)) * k0;
                b = (b ^ (b >> 41)) * k0 + i;
                uint8 u = b >> 37;
                memcpy(data + i, &u, 1);  // uint8 -> char
        }

        lua_pushlstring(L, data, sizeof(data));

        return 1;
}

/* 函数注册表 */
static const luaL_Reg cityhash_reg [] = {
        {"cityhash64", cityhash_cityhash64},
        {"cityhash128", cityhash_cityhash128},
        {"cityhash32", cityhash_cityhash32},
        {"hash128to64", cityhash_hash128to64},
        {"testdata", cityhash_testdata},
        {NULL, NULL}
};

extern "C"
int luaopen_cityhash(lua_State *L) {
        luaL_newlib(L, cityhash_reg);
        return 1;
}


