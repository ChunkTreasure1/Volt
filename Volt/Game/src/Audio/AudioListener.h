#pragma once
#include <Volt/Scripting/ScriptBase.h>
#include <Amp/AudioManager/AudioManager.h>

class AudioListenerScript : public Volt::ScriptBase
{
public:
	AudioListenerScript(Volt::Entity entity);
	~AudioListenerScript() override = default;

	//void OnAwake() override;
	//void OnDetach() override;
	//void OnStart() override;
	//void OnStop() override;
	//void OnUpdate(float aDeltaTime) override;
	//void OnEvent(Volt::Event& e) override;

	void OnUpdate(float aDeltaTime) override;

	static Ref<ScriptBase> Create(Volt::Entity aEntity) { return CreateRef<AudioListenerScript>(aEntity); }
	static WireGUID GetStaticGUID() { return "{E72EFF9E-A386-4B29-94CF-083F55D5C908}"_guid; }
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:

};