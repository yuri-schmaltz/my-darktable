local dt = require "darktable"
local status, socket = pcall(require, "socket")
if status then
  dt.print("LuaSocket is available!")
else
  dt.print("LuaSocket NOT available: " .. socket)
end
