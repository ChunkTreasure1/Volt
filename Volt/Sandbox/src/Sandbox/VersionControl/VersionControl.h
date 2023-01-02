#pragma once

#include <Volt/Core/Base.h>

struct VersionControlSettings
{
	std::string server = "localhost:1666";
	std::string user = "";
	std::string password = "";
};

enum class VersionControlSystem
{
	Perforce
};

class VersionControl
{
public:
	static void Initialize(VersionControlSystem system);
	static void Shutdown();

	static bool Connect(const std::string& server, const std::string& user, const std::string& password);
	static void Disconnect();

	static void Add(const std::filesystem::path& file);
	static void Delete(const std::filesystem::path& file);
	static void Edit(const std::filesystem::path& file);

	static void Submit(const std::string& message);
	static void Sync(const std::string& stream = "");
	
	static void SwitchStream(const std::string& newStream);
	static void RefreshStreams();

	static void SwitchWorkspace(const std::string& workspace);
	static void RefreshWorkspaces();

	static const std::vector<std::string>& GetWorkspaces();
	static const std::vector<std::string>& GetStreams();
	static bool IsConnected();

protected:
	virtual void InitializeImpl() = 0;
	virtual void ShutdownImpl() = 0;
	virtual void DisconnectImpl() = 0;
	virtual bool ConnectImpl(const std::string& server, const std::string& user, const std::string& password) = 0;

	virtual void AddImpl(const std::filesystem::path& file) = 0;
	virtual void DeleteImpl(const std::filesystem::path& file) = 0;
	virtual void EditImpl(const std::filesystem::path& file) = 0;

	virtual void SubmitImpl(const std::string& message) = 0;
	virtual void SyncImpl(const std::string& depo = "") = 0;

	virtual void SwitchWorkspaceImpl(const std::string& newStream) = 0;
	virtual void RefreshWorkspacesImpl() = 0;

	virtual void SwitchStreamImpl(const std::string& newStream) = 0;
	virtual void RefreshStreamsImpl() = 0;

	virtual const std::vector<std::string>& GetWorkspacesImpl() = 0;
	virtual const std::vector<std::string>& GetStreamsImpl() = 0;
	virtual bool IsConnectedImpl() = 0;

private:
	inline static Scope<VersionControl> s_implementation;
};