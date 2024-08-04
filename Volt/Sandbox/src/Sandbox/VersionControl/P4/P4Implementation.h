#pragma once

#include "Sandbox/VersionControl/VersionControl.h"

#include "ClientUsers.h"

#include <p4/clientapi.h>
#include <p4/p4libs.h>

#define P4CHECK(e) 	if (e.Test()) \
					{ \
						StrBuf msg; \
						e.Fmt(&msg); \
						VT_LOG(LogSeverity::Error, "VCS: {0}", msg.Text()); \
						VT_ASSERT(false); \
					} \

class P4Implementation final : public VersionControl
{
public:
	P4Implementation() = default;

protected:
	void InitializeImpl() override;
	void ShutdownImpl() override;
	void DisconnectImpl() override;
	bool ConnectImpl(const std::string& server, const std::string& user, const std::string& password) override;

	void AddImpl(const std::filesystem::path& file) override;
	void DeleteImpl(const std::filesystem::path& file) override;
	void EditImpl(const std::filesystem::path& file) override;

	void SubmitImpl(const std::string& message) override;
	void SyncImpl(const std::string& depo = "") override;

	void SwitchStreamImpl(const std::string& newStream) override;
	void RefreshStreamsImpl() override;

	void SwitchWorkspaceImpl(const std::string& newStream) override;
	void RefreshWorkspacesImpl() override;

	const Vector<std::string>& GetWorkspacesImpl() override;
	const Vector<std::string>& GetStreamsImpl() override;
	bool IsConnectedImpl() override;

private:
	StreamsClientUser m_streamsCU;
	WorkspacesClientUser m_workspacesCU;

	ClientUser m_defaultUser;
	ClientApi m_client;

	bool m_isConnected = false;
};

