#pragma once
#include <Volt/Scripting/ScriptBase.h>
#include <Amp/AudioManager/AudioManager.h>

#include <Volt/Events/KeyEvent.h>
#include <Volt/Events/ApplicationEvent.h>

class AudioSourceScript : public Volt::ScriptBase
{
public:
	AudioSourceScript(Volt::Entity entity);
	~AudioSourceScript() override = default;

	//void OnAwake() override;
	//void OnDetach() override;
	//void OnStart() override;
	//void OnStop() override;
	void OnUpdate(float aDeltaTime) override;
	void OnEvent(Volt::Event& e) override;
	bool OnKeyEvent(Volt::KeyPressedEvent& e);
	bool OnStopSceneEvent(Volt::OnSceneStopEvent& e);

	//Creates and Plays a sound
	bool Play(const std::string& aPath);
	bool Play(const int aID);
	int GetLatestID();
	Amp::EventInstance* GetInstanceFromID(const int aID);

	////Plays an already created sound
	bool Stop(int aID);
	void StopAll();
	bool Pause(int aID);
	bool Unpause(int aID);

	static Ref<ScriptBase> Create(Volt::Entity aEntity) { return CreateRef<AudioSourceScript>(aEntity); }
	static WireGUID GetStaticGUID() { return "{7D885974-D8F6-4A9B-BA1E-008FBEBAAE72}"_guid; }
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	bool lastPlayedOneShot = false;
	int instanceIDpool = -1;
	std::unordered_map<int, Amp::EventInstance&> instances2DMap;
	std::unordered_map<int, Amp::EventInstance&> instances3DMap;
};