--[[
  MCP Bridge for darktable
  Provides JSON-formatted metadata and high-level control API for MCP clients.
]]

local dt = require "darktable"

local mcp = {}

-- Simple JSON encoder (subset for metadata)
function mcp.to_json(v)
  if type(v) == "table" then
    local is_array = #v > 0
    local parts = {}
    if is_array then
      for _, val in ipairs(v) do table.insert(parts, mcp.to_json(val)) end
      return "[" .. table.concat(parts, ",") .. "]"
    else
      for k, val in pairs(v) do 
        table.insert(parts, '"' .. tostring(k) .. '":' .. mcp.to_json(val)) 
      end
      return "{" .. table.concat(parts, ",") .. "}"
    end
  elseif type(v) == "string" then
    return '"' .. v:gsub('"', '\\"'):gsub('\n', '\\n') .. '"'
  elseif type(v) == "number" or type(v) == "boolean" then
    return tostring(v)
  else
    return "null"
  end
end

-- Tool: Get active image info
function mcp.get_active_image()
  local view = dt.gui.current_view()
  if view ~= "darkroom" then
    return mcp.to_json({ error = "Must be in darkroom view to query active image" })
  end
  
  local img = dt.gui.action_images[1] -- In darkroom, this is the current image
  if not img then
    return mcp.to_json({ error = "No active image found" })
  end
  
  local info = {
    id = img.id,
    filename = img.filename,
    path = img.path,
    rating = img.rating,
    exif = {
      model = img.exif_model,
      lens = img.exif_lens,
      exposure = img.exif_exposure,
      aperture = img.exif_aperture,
      iso = img.exif_iso,
      focal_length = img.exif_focal_length
    }
  }
  
  return mcp.to_json(info)
end

-- Global entry point for DBus
_G.mcp_call = function(method)
  if method == "get_active_image" then
    return mcp.get_active_image()
  end
  return mcp.to_json({ error = "Unknown method: " .. tostring(method) })
end

dt.print("MCP Bridge loaded. DBus interface ready for mcp_call().")
return mcp
