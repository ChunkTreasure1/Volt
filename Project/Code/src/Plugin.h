#pragma once

#include <Volt/Plugin/Plugin.h>

#include <EntitySystem/ComponentRegistry.h>

class PLUGIN_API GamePlugin : public Volt::Plugin
{
public:
	~GamePlugin() override = default;

	VT_DECLARE_PLUGIN_GUID("{F2F99642-8530-494C-997B-4101B0FCEEBF}"_guid);

	inline uint32_t GetVersion() const override { return 1; }
	inline std::string_view GetName() const override { return "GamePlugin"; }
	inline std::string_view GetDescription() const override { return "None"; }
	inline std::string_view GetCategory() const override { return "None"; }

	void Initialize() override;
	void Shutdown() override;

private:
};

VT_REGISTER_PLUGIN(GamePlugin);

struct PlayerComponent
{
	static void ReflectType(Volt::TypeDesc<PlayerComponent>& reflect)
	{
		reflect.SetGUID("{4B5BD076-EEDD-4054-93E5-8F673BBC3DE7}"_guid);
		reflect.SetLabel("Player Component");
	}

	REGISTER_COMPONENT(PlayerComponent);
};