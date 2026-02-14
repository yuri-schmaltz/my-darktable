--[[
  Ollama API Client for darktable
  Provides local LLM/VLM integration.
]]

local dt = require "darktable"

local ollama = {
  host = "http://localhost:11434",
  vision_model = "moondream",
  chat_model = "llama3"
}

-- helper: simple synchronous POST via curl
local function post(endpoint, data)
  local json = require("mcp_bridge").to_json(data)
  local tmp_file = "/tmp/ollama_req.json"
  local f = io.open(tmp_file, "w")
  f:write(json)
  f:close()
  
  local cmd = string.format("curl -s -X POST %s%s -d @%s", ollama.host, endpoint, tmp_file)
  local handle = io.popen(cmd)
  local result = handle:read("*a")
  handle:close()
  return result
end

-- Describe an image (requires a vision model like moondream)
function ollama.describe(image_path)
  dt.print("Ollama: Analyzing " .. image_path .. "...")
  
  -- Read and base64 encode the image
  -- In a real scenario, we'd use a more robust b64 method, but here we use openssl for simplicity
  local b64_cmd = string.format("openssl base64 -A -in '%s'", image_path)
  local b64_handle = io.popen(b64_cmd)
  local b64_data = b64_handle:read("*a")
  b64_handle:close()

  local request = {
    model = ollama.vision_model,
    prompt = "Describe this image in one concise sentence for metadata tagging.",
    stream = false,
    images = { b64_data }
  }
  
  local response_raw = post("/api/generate", request)
  -- naive JSON parsing for the response
  local message = response_raw:match('"response":"(.-)"') or "Error: Could not parse response"
  return message
end

-- Chat with an LLM
function ollama.chat(prompt)
  local request = {
    model = ollama.chat_model,
    messages = {{ role = "user", content = prompt }},
    stream = false
  }
  
  local response_raw = post("/api/chat", request)
  local message = response_raw:match('"content":"(.-)"') or "Error: Could not parse response"
  return message
end

return ollama
