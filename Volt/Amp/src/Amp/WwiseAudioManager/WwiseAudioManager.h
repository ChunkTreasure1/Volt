#pragma once
#include <unordered_map>

namespace Amp
{
	struct AKObject
	{
		const char* myObjName;
		uint32_t myObjectID;
		std::vector<uint32_t> myPlayingIDs;
	};

	class WwiseAudioManager
	{
		//FUNCTIONS
	public:
		//OBJECT
		static bool CreateAudioObject(uint32_t aObjID, const char* aObjName);
		static bool DeleteAudioObject(uint32_t aObjID);
		static bool ClearAllObjects();
		static bool SetObjectPosition(uint32_t aObjID, const glm::vec3& aPosition, const glm::vec3& aForward, const glm::vec3& aUp);

		//EVENT
		static bool PlayOneShotEvent(const char* eventName);
		static bool PlayOneShotEvent(const char* eventName, const glm::vec3& aPosition, const glm::vec3& aForward, const glm::vec3& aUp);
		static bool PlayEvent(const char* eventName, uint32_t aObjID, uint32_t& aPlayingID);
		static bool StopEvent(uint32_t aPlayingID);
		static bool PauseEvent(uint32_t aPlayingID);
		static bool ResumeEvent(uint32_t aPlayingID);

		//GAME SYNCS
		static bool SetState(const char* aStateGroup, const char* aState);
		static bool SetSwitch(const char* aSwitchGroup, const char* aState, uint64_t AkGameObjectID);

		static bool SetParameter(const char* aParameterName, float avalue, uint64_t AkGameObjectID);
		static bool SetParameter(const char* aParameterName, float avalue, uint64_t AkGameObjectID, int32_t aOvertime);

		//TOOL
		static std::vector<std::string> GetAllEventNames(std::filesystem::path aFilePath);

		//LISTENERS
		static bool RegisterListener(uint32_t aEntityID, const char* aEntityName, bool isDefault);
		static bool UnregisterListener();
		static bool SetListenerPosition(const glm::vec3& aPosition, const glm::vec3& aForward, const glm::vec3& aUp);

	private:

		//VARIABLES
	private:
		inline static std::unordered_map<uint32_t, AKObject> myRegisteredAKObj;
	};
}
