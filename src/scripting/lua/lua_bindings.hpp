#ifndef CAD_LUA_BINDINGS_HPP
#define CAD_LUA_BINDINGS_HPP

#include <lua.hpp>

#ifdef __cplusplus
extern "C" {
#endif

void register_lua_bindings(lua_State* L);
void run_lua_script(lua_State* L, const char* script);
void run_lua_script_from_file(lua_State* L, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_LUA_BINDINGS_HPP
