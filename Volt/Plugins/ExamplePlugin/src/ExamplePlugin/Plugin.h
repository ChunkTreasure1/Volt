#pragma once

#include <Volt/PluginSystem/EnginePlugin.h>

class ExamplePlugin : public Volt::EnginePlugin
{
public:
	~ExamplePlugin() override = default;

	inline uint32_t GetVersion() const override { return 1; }
	inline std::string_view GetName() const override { return "ExamplePlugin"; }
	inline std::string_view GetDescription() const override { return "Test"; }
	inline std::string_view GetCategory() const override { return "None"; }

	void Initialize() override;
	void Shutdown() override;

private:
};

VT_REGISTER_ENGINE_PLUGIN(ExamplePlugin);
