#include "vtpch.h"
#include "UIUtility.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"

#include <VoltRHI/ImGui/ImGuiImplementation.h>

ImTextureID UI::GetTextureID(Ref<Volt::Texture2D> texture)
{
	//return Volt::RHI::ImGuiImplementation::Get().GetTextureID(texture);
	return nullptr;
}

ImTextureID UI::GetTextureID(Ref<Volt::Image2D> texture)
{
	return nullptr;
}

ImTextureID UI::GetTextureID(Ref<Volt::RHI::Image2D> texture)
{
	return Volt::RHI::ImGuiImplementation::Get().GetTextureID(texture);
}

void UI::Header(const std::string& text)
{
	ScopedFont font{ FontType::Regular_20 };
	ImGui::TextUnformatted(text.c_str());
}

void UI::PushFont(FontType font)
{
	ImGui::PushFont(myFonts.at(font));
}

void UI::PopFont()
{
	ImGui::PopFont();
}

int32_t UI::LevenshteinDistance(const std::string& str1, const std::string& str2)
{
	int32_t m = (int32_t)str1.length();
	int32_t n = (int32_t)str2.length();
	std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

	// Initializing the first row and column as 0
	for (int i = 0; i <= m; i++)
	{
		dp[i][0] = i;
	}
	for (int j = 0; j <= n; j++)
	{
		dp[0][j] = j;
	}

	// Filling in the rest of the dp array
	for (int i = 1; i <= m; i++)
	{
		for (int j = 1; j <= n; j++)
		{
			int insertion = dp[i][j - 1] + 1;
			int deletion = dp[i - 1][j] + 1;
			int match = dp[i - 1][j - 1];
			int mismatch = dp[i - 1][j - 1] + 1;
			if (str1[i - 1] == str2[j - 1])
			{
				dp[i][j] = std::min(std::min(insertion, deletion), match);
			}
			else
			{
				dp[i][j] = std::min(std::min(insertion, deletion), mismatch);
			}
		}
	}
	return dp[m][n];
}

const std::vector<std::string> UI::GetEntriesMatchingQuery(const std::string& query, const std::vector<std::string>& entries)
{
	std::multimap<int32_t, std::string> scores{};

	for (const auto& entry : entries)
	{
		const int32_t score = LevenshteinDistance(query, entry);
		scores.emplace(score, entry);
	}

	std::vector<std::string> result{};
	for (const auto& [score, entry] : scores)
	{
		if (!Utils::StringContains(Utils::ToLower(entry), Utils::ToLower(query)))
		{
			continue;
		}

		result.emplace_back(entry);
	}

	return result;
}

void UI::RenderMatchingTextBackground(const std::string& query, const std::string& text, const glm::vec4& color, const glm::uvec2& offset)
{
	const auto matchOffset = Utils::ToLower(text).find(Utils::ToLower(query));

	if (matchOffset == std::string::npos)
	{
		return;
	}

	const auto matchPrefix = text.substr(0, matchOffset);
	const auto match = text.substr(matchOffset, query.size());

	const auto prefixSize = ImGui::CalcTextSize(matchPrefix.c_str());
	const auto matchSize = ImGui::CalcTextSize(match.c_str());
	const auto cursorPos = ImGui::GetCursorPos();
	const auto windowPos = ImGui::GetWindowPos();
	const auto scrollX = ImGui::GetScrollX();
	const auto scrollY = ImGui::GetScrollY();

	auto currentWindow = ImGui::GetCurrentWindow();
	const ImColor imguiCol = ImColor(color.x, color.y, color.z, color.w);

	const ImVec2 min = { cursorPos.x - scrollX + prefixSize.x + offset.x, cursorPos.y - scrollY + offset.y };
	const ImVec2 max = { cursorPos.x - scrollX + prefixSize.x + matchSize.x + offset.x, cursorPos.y + matchSize.y - scrollY + offset.y };

	currentWindow->DrawList->AddRectFilled(min + windowPos, max + windowPos, imguiCol);
}

void UI::RenderHighlightedBackground(const glm::vec4& color, float height)
{
	auto currentWindow = ImGui::GetCurrentWindow();
	const auto windowPos = ImGui::GetWindowPos();
	const auto availRegion = ImGui::GetContentRegionMax();
	const auto cursorPos = ImGui::GetCursorPos();

	const auto scrollX = ImGui::GetScrollX();
	const auto scrollY = ImGui::GetScrollY();

	const ImVec2 min = ImGui::GetWindowPos() + ImVec2{ -scrollX, cursorPos.y - scrollY };
	const ImVec2 max = ImGui::GetWindowPos() + ImVec2{ availRegion.x - scrollX, height + cursorPos.y - scrollY };
	currentWindow->DrawList->AddRectFilled(min, max, ImColor{ color.x, color.y, color.z, color.w });
}

bool UI::InputTextWithHint(const std::string& name, std::string& text, const std::string& hint, ImGuiInputTextFlags_ flags /* = ImGuiInputTextFlags_None */)
{
	if (!name.empty())
	{
		ImGui::TextUnformatted(name.c_str());
		ImGui::SameLine();
	}

	std::string id = "##" + std::to_string(s_stackId++);
	return ImGui::InputTextWithHintString(id.c_str(), hint.c_str(), &text, flags);
}
