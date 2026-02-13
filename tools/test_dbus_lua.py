#!/usr/bin/env python3
import sys
try:
    from gi.repository import GLib, Gio
except ImportError:
    print("PyGObject (python3-gi) is required.")
    sys.exit(1)

def test_dbus():
    try:
        bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
        proxy = Gio.DBusProxy.new_sync(
            bus,
            Gio.DBusProxyFlags.NONE,
            None,
            "org.darktable.service",
            "/darktable",
            "org.darktable.service.Remote",
            None
        )
        
        script = "require 'darktable'; print('LUA DBUS TEST: OK'); return 'SUCCESS'"
        print(f"Sending script: {script}")
        
        result = proxy.call_sync(
            "Lua",
            GLib.Variant("(s)", (script,)),
            Gio.DBusCallFlags.NONE,
            -1,
            None
        )
        
        print(f"Result from darktable: {result.unpack()[0]}")
        
    except Exception as e:
        print(f"Error: {e}")
        print("\nMake sure darktable is running and its DBus service is active.")

if __name__ == "__main__":
    test_dbus()
