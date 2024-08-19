#pragma once

#include <Volt/PluginSystem/Plugin.h>

class PLUGIN_API ExamplePlugin : public Volt::Plugin
{
public:
	~ExamplePlugin() override = default;

	VT_DECLARE_PLUGIN_GUID("39091412-4860-4001-9CAD-A5CEE6DD8F6E"_guid);

	inline uint32_t GetVersion() const override { return 1; }
	inline std::string_view GetName() const override { return "ExamplePlugin"; }
	inline std::string_view GetDescription() const override { return "Test"; }
	inline std::string_view GetCategory() const override { return "None"; }

	void Initialize() override;
	void Shutdown() override;

private:
};

VT_REGISTER_PLUGIN(ExamplePlugin);
