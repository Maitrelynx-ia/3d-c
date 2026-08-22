#include "lua_bindings.hpp"
#include "../api/part_api.h"
#include <lua.hpp>
#include <iostream>

static int lua_part_new(lua_State* L) {
    const char* name = luaL_checkstring(L, 1); Part** part = (Part**)lua_newuserdata(L, sizeof(Part*)); *part = part_new(name); luaL_getmetatable(L, "Part"); lua_setmetatable(L, -2); return 1;
}

static int lua_part_delete(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); part_delete(part); return 0; }
static int lua_part_set_name(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); const char* name = luaL_checkstring(L, 2); part_set_name(part, name); return 0; }
static int lua_part_get_name(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); lua_pushstring(L, part_get_name(part)); return 1; }
static int lua_part_create_sketch(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); const char* plane_type = luaL_checkstring(L, 2); Sketch* sketch = part_create_sketch(part, plane_type); Sketch** sketch_ptr = (Sketch**)lua_newuserdata(L, sizeof(Sketch*)); *sketch_ptr = sketch; luaL_getmetatable(L, "Sketch"); lua_setmetatable(L, -2); return 1; }

static int lua_sketch_add_line(lua_State* L) { Sketch* sketch = *(Sketch**)luaL_checkudata(L, 1, "Sketch"); float x1 = luaL_checknumber(L, 2); float y1 = luaL_checknumber(L, 3); float z1 = luaL_checknumber(L, 4); float x2 = luaL_checknumber(L, 5); float y2 = luaL_checknumber(L, 6); float z2 = luaL_checknumber(L, 7); sketch_add_line(sketch, x1, y1, z1, x2, y2, z2); return 0; }
static int lua_sketch_add_circle(lua_State* L) { Sketch* sketch = *(Sketch**)luaL_checkudata(L, 1, "Sketch"); float x = luaL_checknumber(L, 2); float y = luaL_checknumber(L, 3); float z = luaL_checknumber(L, 4); float radius = luaL_checknumber(L, 5); sketch_add_circle(sketch, x, y, z, radius); return 0; }
static int lua_sketch_add_arc(lua_State* L) { Sketch* sketch = *(Sketch**)luaL_checkudata(L, 1, "Sketch"); float x = luaL_checknumber(L, 2); float y = luaL_checknumber(L, 3); float z = luaL_checknumber(L, 4); float radius = luaL_checknumber(L, 5); float start_angle = luaL_checknumber(L, 6); float end_angle = luaL_checknumber(L, 7); sketch_add_arc(sketch, x, y, z, radius, start_angle, end_angle); return 0; }
static int lua_sketch_add_point(lua_State* L) { Sketch* sketch = *(Sketch**)luaL_checkudata(L, 1, "Sketch"); float x = luaL_checknumber(L, 2); float y = luaL_checknumber(L, 3); float z = luaL_checknumber(L, 4); sketch_add_point(sketch, x, y, z); return 0; }

static int lua_part_extrude(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); Sketch* sketch = *(Sketch**)luaL_checkudata(L, 2, "Sketch"); float depth = luaL_checknumber(L, 3); bool is_additive = lua_toboolean(L, 4); part_extrude(part, sketch, depth, is_additive); return 0; }
static int lua_part_revolve(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); Sketch* sketch = *(Sketch**)luaL_checkudata(L, 2, "Sketch"); float angle = luaL_checknumber(L, 3); float ax = luaL_checknumber(L, 4); float ay = luaL_checknumber(L, 5); float az = luaL_checknumber(L, 6); part_revolve(part, sketch, angle, ax, ay, az); return 0; }
static int lua_part_add_hole(lua_State* L) { Part* part = *(Part**)luaL_checkudata(L, 1, "Part"); Sketch* sketch = *(Sketch**)luaL_checkudata(L, 2, "Sketch"); float depth = luaL_checknumber(L, 3); bool is_through = lua_toboolean(L, 4); part_add_hole(part, sketch, depth, is_through); return 0; }

static int lua_assembly_new(lua_State* L) { const char* name = luaL_checkstring(L, 1); Assembly** assembly = (Assembly**)lua_newuserdata(L, sizeof(Assembly*)); *assembly = assembly_new(name); luaL_getmetatable(L, "Assembly"); lua_setmetatable(L, -2); return 1; }
static int lua_assembly_delete(lua_State* L) { Assembly* assembly = *(Assembly**)luaL_checkudata(L, 1, "Assembly"); assembly_delete(assembly); return 0; }
static int lua_assembly_add_part(lua_State* L) { Assembly* assembly = *(Assembly**)luaL_checkudata(L, 1, "Assembly"); Part* part = *(Part**)luaL_checkudata(L, 2, "Part"); assembly_add_part(assembly, part); return 0; }
static int lua_assembly_solve_constraints(lua_State* L) { Assembly* assembly = *(Assembly**)luaL_checkudata(L, 1, "Assembly"); bool result = assembly_solve_constraints(assembly); lua_pushboolean(L, result); return 1; }
static int lua_assembly_save(lua_State* L) { Assembly* assembly = *(Assembly**)luaL_checkudata(L, 1, "Assembly"); const char* filename = luaL_checkstring(L, 2); bool result = assembly_save(assembly, filename); lua_pushboolean(L, result); return 1; }
static int lua_assembly_export_stl(lua_State* L) { Assembly* assembly = *(Assembly**)luaL_checkudata(L, 1, "Assembly"); const char* filename = luaL_checkstring(L, 2); bool result = assembly_export_stl(assembly, filename); lua_pushboolean(L, result); return 1; }

void register_lua_bindings(lua_State* L) {
    luaL_newmetatable(L, "Part"); lua_pushcfunction(L, lua_part_delete); lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, lua_part_set_name); lua_setfield(L, -2, "setName");
    lua_pushcfunction(L, lua_part_get_name); lua_setfield(L, -2, "getName");
    lua_pushcfunction(L, lua_part_create_sketch); lua_setfield(L, -2, "createSketch");
    lua_pushcfunction(L, lua_part_extrude); lua_setfield(L, -2, "extrude");
    lua_pushcfunction(L, lua_part_revolve); lua_setfield(L, -2, "revolve");
    lua_pushcfunction(L, lua_part_add_hole); lua_setfield(L, -2, "addHole"); lua_pop(L, 1);
    luaL_newmetatable(L, "Sketch"); lua_pushcfunction(L, lua_sketch_add_line); lua_setfield(L, -2, "addLine");
    lua_pushcfunction(L, lua_sketch_add_circle); lua_setfield(L, -2, "addCircle");
    lua_pushcfunction(L, lua_sketch_add_arc); lua_setfield(L, -2, "addArc");
    lua_pushcfunction(L, lua_sketch_add_point); lua_setfield(L, -2, "addPoint"); lua_pop(L, 1);
    luaL_newmetatable(L, "Assembly"); lua_pushcfunction(L, lua_assembly_delete); lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, lua_assembly_add_part); lua_setfield(L, -2, "addPart");
    lua_pushcfunction(L, lua_assembly_solve_constraints); lua_setfield(L, -2, "solveConstraints");
    lua_pushcfunction(L, lua_assembly_save); lua_setfield(L, -2, "save");
    lua_pushcfunction(L, lua_assembly_export_stl); lua_setfield(L, -2, "exportSTL"); lua_pop(L, 1);
    lua_register(L, "Part_new", lua_part_new); lua_register(L, "Assembly_new", lua_assembly_new);
}

void run_lua_script(lua_State* L, const char* script) {
    if (luaL_dostring(L, script) != LUA_OK) { const char* error = lua_tostring(L, -1); std::cerr << "Erreur Lua: " << (error ? error : "inconnue") << std::endl; }
}

void run_lua_script_from_file(lua_State* L, const char* filename) {
    if (luaL_dofile(L, filename) != LUA_OK) { const char* error = lua_tostring(L, -1); std::cerr << "Erreur Lua dans " << filename << ": " << (error ? error : "inconnue") << std::endl; }
}
