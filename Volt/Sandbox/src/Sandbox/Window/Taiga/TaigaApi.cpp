#include "sbpch.h"
#include "TaigaApi.h"

#include <httplib.h>
#include <format>

static httplib::SSLClient s_cli("taiga.tga.learnet.se", 443);

bool TaigaAPI::Auth(const std::string& username, const std::string& password, TaigaGeneralInfo& info)
{
	s_cli.enable_server_certificate_verification(false);

	nlohmann::json json;
	json["username"] = username;
	json["password"] = password;
	json["type"] = "normal";

	auto body = json.dump();
	
	if (auto res = s_cli.Post("/api/v1/auth", body.c_str(), body.size(), "application/json"))
	{
		if (res->status == 200)
		{
			nlohmann::json json = nlohmann::json::parse(res->body);
			info.authToken = json["auth_token"];
			info.userId = json["id"];
			return true;
		}
	}
	else
	{
		VT_CORE_WARN(std::format("HTTPS Error: {0}", httplib::to_string(res.error())));
	}
	return false;
}

bool TaigaAPI::GetProjects(const TaigaGeneralInfo& info, std::vector<ItemEntry>& outProjects)
{
	if (info.authToken.empty() || !info.userId.has_value()) { return false; }
	s_cli.enable_server_certificate_verification(false);

	httplib::Headers headers = 
	{
		{"Content-Type", "application/json"},
		{"Authorization", std::format("Bearer {0}", info.authToken)},
	};

	std::string path = "/api/v1/projects?";
	path.append(std::format("member={0}&", info.userId.value()));

	if (auto res = s_cli.Get(path, headers))
	{
		if (res->status == 200)
		{
			nlohmann::json json = nlohmann::json::parse(res->body);
			outProjects.clear();
			for (const auto& obj : json)
			{
				ItemEntry item;
				item.name = obj["name"];
				item.id = obj["id"];
				item.type = ItemType::Project;

				outProjects.emplace_back(item);
			}
			return true;
		}
	}
	else
	{
		VT_CORE_WARN(std::format("HTTPS Error: {0}", httplib::to_string(res.error())));
	}
	return false;
}

bool TaigaAPI::GetUserstories(const TaigaGeneralInfo& info, std::vector<ItemEntry>& outUserstories)
{
	if (info.authToken.empty() || !info.userId.has_value() || !info.selectedProjectId.has_value()) { return false; }
	s_cli.enable_server_certificate_verification(false);

	httplib::Headers headers =
	{
		{"Content-Type", "application/json"},
		{"Authorization", std::format("Bearer {0}", info.authToken)},
	};

	std::string path = "/api/v1/userstories?";
	path.append(std::format("project={0}&", info.selectedProjectId.value()));
	path.append(std::format("assigned_to={0}&", info.userId.value()));

	if (auto res = s_cli.Get(path, headers))
	{
		if (res->status == 200)
		{
			nlohmann::json json = nlohmann::json::parse(res->body);
			outUserstories.clear();
			for (const auto& obj : json)
			{
				ItemEntry item;
				item.name = obj["subject"];
				item.id = obj["id"];
				item.type = ItemType::Userstory;

				outUserstories.emplace_back(item);
			}
			return true;
		}
	}
	else
	{
		VT_CORE_WARN(std::format("HTTPS Error: {0}", httplib::to_string(res.error())));
	}
	return false;
}

bool TaigaAPI::GetTasks(const TaigaGeneralInfo& info, std::vector<ItemEntry>& outTasks)
{
	if (info.authToken.empty() || !info.userId.has_value() || !info.selectedProjectId.has_value() || !info.selectedUserstoryId.has_value()) { return false; }
	s_cli.enable_server_certificate_verification(false);

	httplib::Headers headers =
	{
		{"Content-Type", "application/json"},
		{"Authorization", std::format("Bearer {0}", info.authToken)},
	};

	std::string path = "/api/v1/tasks?";
	path.append(std::format("project={0}&", info.selectedProjectId.value()));
	path.append(std::format("assigned_to={0}&", info.userId.value()));
	path.append(std::format("user_story={0}&", info.selectedUserstoryId.value()));

	if (auto res = s_cli.Get(path, headers))
	{
		if (res->status == 200)
		{
			nlohmann::json json = nlohmann::json::parse(res->body);
			outTasks.clear();
			for (const auto& obj : json)
			{
				ItemEntry item;
				item.name = obj["subject"];
				item.id = obj["id"];
				item.type = ItemType::Task;

				outTasks.emplace_back(item);
			}
			return true;
		}
	}
	else
	{
		VT_CORE_WARN(std::format("HTTPS Error: {0}", httplib::to_string(res.error())));
	}
	return false;
}

bool TaigaAPI::GetIssues(const TaigaGeneralInfo& info, std::vector<ItemEntry>& outIssues)
{
	if (info.authToken.empty() || !info.userId.has_value() || !info.selectedProjectId.has_value()) { return false; }
	s_cli.enable_server_certificate_verification(false);

	httplib::Headers headers =
	{
		{"Content-Type", "application/json"},
		{"Authorization", std::format("Bearer {0}", info.authToken)},
	};

	std::string path = "/api/v1/issues?";
	path.append(std::format("project={0}&", info.selectedProjectId.value()));
	path.append(std::format("assigned_to={0}&", info.userId.value()));

	if (auto res = s_cli.Get(path, headers))
	{
		if (res->status == 200)
		{
			nlohmann::json json = nlohmann::json::parse(res->body);
			outIssues.clear();
			for (const auto& obj : json)
			{
				ItemEntry item;
				item.name = obj["subject"];
				item.id = obj["id"];
				item.type = ItemType::Issue;

				outIssues.emplace_back(item);
			}
			return true;
		}
	}
	else
	{
		VT_CORE_WARN(std::format("HTTPS Error: {0}", httplib::to_string(res.error())));
	}
	return false;
}
