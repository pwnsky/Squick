
#include "plugin.h"
#include "server_module.h"


namespace gameplay_manager::server {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

	CREATE_PLUGIN(pm, Plugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
	DESTROY_PLUGIN(pm, Plugin)
};


//////////////////////////////////////////////////////////////////////////

const int Plugin::GetPluginVersion()
{
	return 0;
}

const std::string Plugin::GetPluginName()
{
	return GET_CLASS_NAME(Plugin);
}

void Plugin::Install()
{
	REGISTER_MODULE(pPluginManager, IServerModule, ServerModule)
}

void Plugin::Uninstall()
{
	UNREGISTER_MODULE(pPluginManager, IServerModule, ServerModule)
}

}