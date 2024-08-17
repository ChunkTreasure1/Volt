#pragma once

#include <Volt/PluginSystem/GamePlugin.h>

class IDiscordManager;

class DiscordPlugin : public Volt::GamePlugin
{
public:
	~DiscordPlugin() override = default;

	inline uint32_t GetVersion() const override { return 1; }
	inline std::string_view GetName() const override { return "DiscordPlugin"; }
	inline std::string_view GetDescription() const override { return ""; }
	inline std::string_view GetCategory() const override { return "None"; }

	void Initialize() override;
	void Shutdown() override;

	void OnEvent(Volt::Event& event) override;

	VT_INLINE IDiscordManager& GetManager() const { return *m_discordManager; }
	VT_INLINE static DiscordPlugin& GetInstance() { return *s_instance; }

private:
	inline static DiscordPlugin* s_instance = nullptr;

	Scope<IDiscordManager> m_discordManager;
};

VT_REGISTER_GAME_PLUGIN(DiscordPlugin);
