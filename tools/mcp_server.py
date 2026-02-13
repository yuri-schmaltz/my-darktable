#!/usr/bin/env python3
"""
darktable MCP Server
Exposes darktable tools via Model Context Protocol (MCP) using DBus bridge.
"""

import sys
import json
import asyncio
from typing import Any, List, Optional
try:
    from gi.repository import GLib, Gio
except ImportError:
    print("Error: PyGObject (python3-gi) required for DBus communication.")
    sys.exit(1)

# MCP Tool Definition Helper
def make_tool(name: str, description: str, properties: dict, required: List[str] = []):
    return {
        "name": name,
        "description": description,
        "inputSchema": {
            "type": "object",
            "properties": properties,
            "required": required
        }
    }

class DarktableMCPServer:
    def __init__(self):
        self.bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
        self.proxy = Gio.DBusProxy.new_sync(
            self.bus,
            Gio.DBusProxyFlags.NONE,
            None,
            "org.darktable.service",
            "/darktable",
            "org.darktable.service.Remote",
            None
        )

    def call_lua(self, script: str) -> str:
        try:
            # We call the 'Lua' method of the org.darktable.service.Remote interface
            result = self.proxy.call_sync(
                "Lua",
                GLib.Variant("(s)", (script,)),
                Gio.DBusCallFlags.NONE,
                -1,
                None
            )
            return result.unpack()[0]
        except Exception as e:
            return json.dumps({"error": f"DBus call failed: {str(e)}"})

    async def main_loop(self):
        # Read MCP requests from stdin
        while True:
            line = await asyncio.get_event_loop().run_in_executor(None, sys.stdin.readline)
            if not line:
                break
            
            try:
                request = json.loads(line)
                response = await self.handle_request(request)
                if response:
                    print(json.dumps(response), flush=True)
            except Exception as e:
                # Standard MCP error handling would go here
                pass

    async def handle_request(self, request: dict) -> Optional[dict]:
        method = request.get("method")
        params = request.get("params", {})
        request_id = request.get("id")

        if method == "initialize":
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {"tools": {}},
                    "serverInfo": {"name": "darktable-mcp", "version": "0.1.0"}
                }
            }

        elif method == "listTools":
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": {
                    "tools": [
                        make_tool("get_active_image", "Get metadata of the current image in Darkroom", {}),
                        make_tool("apply_exposure", "Adjust exposure of the active image", 
                                  {"ev": {"type": "number", "description": "Exposure value in EV (e.g. 0.5, -1.0)"}}, ["ev"]),
                        make_tool("lua_query", "Execute arbitrary Lua script in darktable", 
                                  {"code": {"type": "string"}}, ["code"]),
                        make_tool("ask_ai_about_image", "Ask a local LLM for advice about the current image", 
                                  {"prompt": {"type": "string"}}, ["prompt"])
                    ]
                }
            }

        elif method == "callTool":
            tool_name = params.get("name")
            tool_args = params.get("arguments", {})
            
            result_str = ""
            if tool_name == "get_active_image":
                result_str = self.call_lua("return mcp_call('get_active_image')")
            elif tool_name == "apply_exposure":
                ev = tool_args.get("ev", 0)
                # This uses the simple_mode logic we implemented earlier
                script = f"darktable.gui.action('iop/exposure/exposure', 0, nil, 'set', {ev}); return 'OK'"
                result_str = self.call_lua(script)
            elif tool_name == "lua_query":
                script = tool_args.get("code", "")
                result_str = self.call_lua(script)
            elif tool_name == "ask_ai_about_image":
                user_prompt = tool_args.get("prompt", "")
                metadata = self.call_lua("return mcp_call('get_active_image')")
                full_prompt = f"IMAGE METADATA: {metadata}\n\nUSER QUESTION: {user_prompt}\n\nProvide specific editing advice based on the metadata."
                # We reuse the lua bridge to call ollama
                script = f"return require('ollama_client').chat([[ {full_prompt} ]])"
                result_str = self.call_lua(script)
            
            try:
                content = json.loads(result_str)
            except:
                content = result_str

            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": {
                    "content": [{"type": "text", "text": json.dumps(content, indent=2)}]
                }
            }

        return None

if __name__ == "__main__":
    server = DarktableMCPServer()
    asyncio.run(server.main_loop())
