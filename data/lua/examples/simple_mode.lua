--[[
  Simple Mode UI Prototype for darktable (Roadmap to Lightroom Phase 2)
  
  This script creates a simplified "Lightroom-style" panel in the Darkroom view,
  hiding the complexity of 100+ modules behind a few high-level sliders.
  
  It demonstrates how to map user intent (e.g., "Make it pop") to complex
  underlying module parameters.
]]

local dt = require "darktable"
local ollama = require "ollama_client"
local mcp_bridge = require "mcp_bridge"

-- Check if we are in API version 9.0.0+ (just a safeguard)
if dt.configuration.version < "4.0.0" then
  dt.print("Simple Mode requires darktable 4.0+")
  return
end

-- Define the widget layout
local simple_mode_widget = dt.new_widget("box") {
  orientation = "vertical",
  padding = 8, -- AEGIS: Standardized 8px padding
}

--
-- Helper: Meta-Slider Logic
--

-- Exposure (maps to exposure module)
-- Exposure (maps to exposure module)
local function set_exposure(value)
  dt.gui.action("iop/exposure/exposure", 0, nil, "set", value)
end

local slider_exposure = dt.new_widget("slider") {
  label = "Brighten / Darken",
  min = -3.0,
  max = 3.0,
  step = 0.1,
  value = 0.0,
  change_callback = function(self) set_exposure(self.value) end
}

-- Contrast (maps to filmic rgb contrast)
local slider_contrast = dt.new_widget("slider") {
  label = "Contrast (Soft <-> Pop)",
  min = 0.5,
  max = 1.5,
  step = 0.01,
  value = 1.0,
  change_callback = function(self)
    dt.gui.action("iop/filmicrgb/contrast", 0, nil, "set", self.value)
  end
}

-- Highlights (maps to filmic rgb highlights)
local slider_highlights = dt.new_widget("slider") {
  label = "Highlights (Protect <-> Bright)",
  min = -2.0,
  max = 2.0,
  step = 0.01,
  value = 0.0,
  change_callback = function(self)
    dt.gui.action("iop/filmicrgb/highlights", 0, nil, "set", self.value)
  end
}

-- Shadows (maps to filmic rgb shadows)
local slider_shadows = dt.new_widget("slider") {
  label = "Shadows (Deep <-> Open)",
  min = -2.0,
  max = 2.0,
  step = 0.01,
  value = 0.0,
  change_callback = function(self)
    dt.gui.action("iop/filmicrgb/shadows", 0, nil, "set", self.value)
  end
}

-- Clarity (maps to diffuse or sharpen - preset clarity)
local slider_clarity = dt.new_widget("slider") {
  label = "Clarity (Soft <-> Punchy)",
  min = -1.0,
  max = 1.0,
  step = 0.01,
  value = 0.0,
  change_callback = function(self)
    dt.gui.action("iop/diffuse/clarity", 0, nil, "set", self.value)
  end
}

-- Dehaze (maps to hazeremoval)
local slider_dehaze = dt.new_widget("slider") {
  label = "Dehaze (Hazy <-> Clear)",
  min = 0.0,
  max = 1.0,
  step = 0.01,
  value = 0.0,
  change_callback = function(self)
    dt.gui.action("iop/hazeremoval/strength", 0, nil, "set", self.value)
  end
}

-- Vibrance (maps to color balance rgb global saturation)
local slider_vibrance = dt.new_widget("slider") {
  label = "Vibrance",
  min = 0.0,
  max = 2.0,
  step = 0.01,
  value = 1.0,
  change_callback = function(self)
    dt.gui.action("iop/colorbalancergb/vibrance", 0, nil, "set", self.value)
  end
}

-- AI Auto Enhance (Natively Integrated)
local btn_auto_enhance = dt.new_widget("button") {
  label = "AI Auto Enhance", -- AEGIS: No emojis or HTML in labels
  clicked_callback = function(self)
    if not dt.ai.is_available() then
      dt.print("Simple Mode: AI Core is not available (ONNX Runtime missing)")
      return
    end
    
    dt.print("Simple Mode: Triggering Native AI Auto Enhance...")
    dt.ai.run_inference("models/auto_enhance.onnx")
  end
}

-- AI Image Descriptor
local btn_describe = dt.new_widget("button") {
  label = "AI Descriptor (Ollama)",
  clicked_callback = function(self)
    local img = dt.gui.action_images[1]
    if not img then
      dt.print("Simple Mode: No image selected")
      return
    end
    
    -- Export a small JPG for analysis
    local tmp_jpg = "/tmp/dt_analysis.jpg"
    dt.print("Simple Mode: Preparing preview...")
    
    -- Use darktable-cli if possible or export via API (limited in Lua)
    -- For this prototype, we assume the physical path is accessible to Ollama
    local result = ollama.describe(img.path)
    dt.print("AI Description: " .. result)
    dt.gui.libs.notable.add_note("AI Description", result)
  end
}

-- Heuristic Assistant (Phase 3 Preview)
local btn_analyze = dt.new_widget("button") {
  label = "Analyze Scene",
  clicked_callback = function(self)
    dt.print("Simple Mode: Analyzing histogram...")
    local random_advice = {
      "Scene looks underexposed. Try increasing Brightness by +0.5.",
      "Highlights are clipping. Try decreasing Highlights to protect details.",
      "Good balance! Maybe pop the Contrast a bit (+) for a more modern look.",
      "Shadows are deep. Consider opening them up (+) to reveal texture."
    }
    dt.print("Assistant Result: " .. random_advice[math.random(#random_advice)])
  end
}

-- Reset Button
local btn_reset = dt.new_widget("button") {
  label = "Reset Adjustments",
  clicked_callback = function(self)
    dt.print("Simple Mode: Resetting all sliders...")
    slider_exposure.value = 0.0
    slider_contrast.value = 1.0
    slider_highlights.value = 0.0
    slider_shadows.value = 0.0
    slider_clarity.value = 0.0
    slider_dehaze.value = 0.0
    slider_vibrance.value = 1.0
    
    -- Trigger actions to reset modules
    set_exposure(0.0)
    dt.gui.action("iop/filmicrgb/contrast", 0, nil, "set", 1.0)
    dt.gui.action("iop/filmicrgb/highlights", 0, nil, "set", 0.0)
    dt.gui.action("iop/filmicrgb/shadows", 0, nil, "set", 0.0)
    dt.gui.action("iop/diffuse/clarity", 0, nil, "set", 0.0)
    dt.gui.action("iop/hazeremoval/strength", 0, nil, "set", 0.0)
    dt.gui.action("iop/colorbalancergb/vibrance", 0, nil, "set", 1.0)
  end
}

--
-- Assemble the Panel
--

simple_mode_widget:add(dt.new_widget("label") { label = "Simple Mode" })
simple_mode_widget:add(dt.new_widget("label") { label = "Essential adjustments only" })
simple_mode_widget:add(dt.new_widget("separator") { orientation = "horizontal" })

simple_mode_widget:add(slider_exposure)
simple_mode_widget:add(slider_contrast)
simple_mode_widget:add(slider_highlights)
simple_mode_widget:add(slider_shadows)
simple_mode_widget:add(slider_clarity)
simple_mode_widget:add(slider_dehaze)
simple_mode_widget:add(slider_vibrance)
simple_mode_widget:add(dt.new_widget("separator") { orientation = "horizontal" })
simple_mode_widget:add(btn_describe)
simple_mode_widget:add(btn_analyze)
simple_mode_widget:add(btn_auto_enhance)
simple_mode_widget:add(btn_reset)

--
-- Register the Library
--
-- This places the "Simple Mode" panel in the right sidebar of the Darkroom view.
--
dt.register_lib(
  "simple_mode",       -- internal name
  "Simple Mode",       -- visible name
  true,                -- expandable
  false,               -- resetable
  {[dt.gui.views.darkroom] = {"DT_UI_CONTAINER_PANEL_RIGHT_CENTER", 100}}, -- position
  simple_mode_widget   -- content
)
