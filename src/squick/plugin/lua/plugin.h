#pragma once

#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

class LuaScriptPlugin : public IPlugin
{
public:
    LuaScriptPlugin(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};