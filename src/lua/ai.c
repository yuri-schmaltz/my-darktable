/*
    This file is part of darktable,
    Copyright (C) 2024 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "common/darktable.h"
#include "common/dt_ai_core.h"
#include "lua/ai.h"
#include "lua/lua.h"

/* darktable.ai.is_available() -> boolean */
static int is_available(lua_State *L)
{
  lua_pushboolean(L, dt_ai_is_available());
  return 1;
}

/* darktable.ai.run_inference(model_path) -> string */
static int run_inference(lua_State *L)
{
  const char *model_path = luaL_checkstring(L, 1);
  dt_print(DT_DEBUG_LUA, "[AI Core] Inference triggered via Lua for model: %s", model_path);
  
  /* 
     This is a placeholder for the actual inference logic.
     In a full implementation, this would call dt_ai_run_inference
     and potentially return a table of results/parameters.
  */
  
  lua_pushstring(L, "inference_triggered");
  return 1;
}

int dt_lua_init_ai(lua_State *L)
{
  dt_lua_push_darktable_lib(L);
  dt_lua_goto_subtable(L, "ai");

  lua_pushstring(L, "is_available");
  lua_pushcfunction(L, is_available);
  lua_settable(L, -3);

  lua_pushstring(L, "run_inference");
  lua_pushcfunction(L, run_inference);
  lua_settable(L, -3);

  lua_pop(L, 1); // remove ai table
  return 0;
}
