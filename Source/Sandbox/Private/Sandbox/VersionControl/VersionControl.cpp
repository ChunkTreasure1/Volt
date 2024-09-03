#include "sbpch.h"
#include "VersionControl.h"

#include "Sandbox/VersionControl/P4/P4Implementation.h"

void VersionControl::Initialize(VersionControlSystem system)
{
	switch (system)
	{
		case VersionControlSystem::Perforce:
			s_implementation = CreateScope<P4Implementation>();
			break;

		default:
			VT_LOG(Error, "Invalid version control selected!");
			return;
			break;
	}

	s_implementation->InitializeImpl();
}

void VersionControl::Shutdown()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->ShutdownImpl();
	s_implementation = nullptr;
}

bool VersionControl::Connect(const std::string& server, const std::string& user, const std::string& password)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	return s_implementation->ConnectImpl(server, user, password);
}

void VersionControl::Disconnect()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->DisconnectImpl();
}

void VersionControl::Add(const std::filesystem::path& file)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->AddImpl(file);
}

void VersionControl::Delete(const std::filesystem::path& file)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->DeleteImpl(file);
}

void VersionControl::Edit(const std::filesystem::path& file)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->EditImpl(file);
}

void VersionControl::Submit(const std::string& message)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->SubmitImpl(message);
}

void VersionControl::Sync(const std::string& depo)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->SyncImpl(depo);
}

void VersionControl::SwitchStream(const std::string& newStream)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->SwitchStreamImpl(newStream);
}

void VersionControl::RefreshStreams()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->RefreshStreamsImpl();
}

void VersionControl::SwitchWorkspace(const std::string& workspace)
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->SwitchWorkspaceImpl(workspace);
}

void VersionControl::RefreshWorkspaces()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	s_implementation->RefreshWorkspacesImpl();
}

const Vector<std::string>& VersionControl::GetWorkspaces()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	return s_implementation->GetWorkspacesImpl();
}

const Vector<std::string>& VersionControl::GetStreams()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	return s_implementation->GetStreamsImpl();
}

bool VersionControl::IsConnected()
{
	VT_ASSERT_MSG(s_implementation, "No implementation loaded!");
	return s_implementation->IsConnectedImpl();
}
