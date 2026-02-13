/*
    This file is part of darktable,
    Copyright (C) 2024 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#ifndef DT_LUA_AI_H
#define DT_LUA_AI_H

#include "lua/lua.h"

/* initialize the AI module in the darktable namespace */
int dt_lua_init_ai(lua_State *L);

#endif
