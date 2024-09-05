#pragma once

#include <Volt/Plugin/Plugin.h>

#include <CoreUtilities/CompilerTraits.h>

class IDiscordManager;

class PLUGIN_API DiscordPlugin : public Volt::Plugin
{
public:
	DiscordPlugin();
	~DiscordPlugin() override = default;

	VT_DECLARE_PLUGIN_GUID("{D45ED8C0-50EF-4BB5-BD83-E081D4641AE6}"_guid);

	inline uint32_t GetVersion() const override { return 1; }
	inline std::string_view GetName() const override { return "DiscordPlugin"; }
	inline std::string_view GetDescription() const override { return ""; }
	inline std::string_view GetCategory() const override { return "None"; }

	void Initialize() override;
	void Shutdown() override;

	VT_INLINE IDiscordManager& GetManager() const { return *m_discordManager; }
	VT_INLINE static DiscordPlugin& GetInstance() { return *s_instance; }

private:
	inline static DiscordPlugin* s_instance = nullptr;

	Scope<IDiscordManager> m_discordManager;
};

VT_REGISTER_PLUGIN(DiscordPlugin);
