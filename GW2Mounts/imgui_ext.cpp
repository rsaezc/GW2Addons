#include "imgui_ext.h"
#include "Utility.h"

void ImGuiKeybind::InitKeybind(const KeySequence& keys)
{
	UpdateDisplayString(keys);
	LastKeyBindString.assign(DisplayKeyBindString);
}

void ImGuiKeybind::UpdateKeybind(const KeySequence& keys, bool apply)
{
	if (IsBeingModified)
	{
		ImGui::GetIO().WantCaptureKeyboard = true;
		UpdateDisplayString(keys);
		if (apply)
		{
			LastKeyBindString.assign(DisplayKeyBindString);
			IsBeingModified = false;
			ImGui::GetIO().WantCaptureKeyboard = false;
			SetCallback(keys);
		}
	}
}

void ImGuiKeybind::CancelKeybind()
{
	if (IsBeingModified)
	{
		CancelPending = true;
		ImGui::GetIO().WantCaptureKeyboard = false;
	}
}

void ImGuiKeybind::UpdateDisplayString(const KeySequence& keys)
{
	std::string keybind = "";
	for (const auto& k : keys)
	{
		keybind += GetKeyName(k) + std::string(" + ");
	}

	strcpy_s(DisplayKeyBindString, (keybind.size() > 0 ? keybind.substr(0, keybind.size() - 3) : keybind).c_str());
}

ImVec4 operator/(const ImVec4& v, float f)
{
	return ImVec4(v.x / f, v.y / f, v.z / f, v.w / f);
}

void ImGuiKeybindInput(const std::string& name, ImGuiKeybind& setting)
{
	std::string suffix = "##" + name;

	float windowWidth = ImGui::GetWindowWidth();

	ImGui::PushItemWidth(windowWidth * 0.3f);

	int popcount = 1;
	if (setting.IsBeingModified)
	{
		popcount = 3;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201, 215, 255, 200) / 255.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0, 0, 0, 1));
	}
	else
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));

	ImGui::InputText(suffix.c_str(), setting.DisplayKeyBindString, 256, ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();

	ImGui::PopStyleColor(popcount);

	ImGui::SameLine();

	if (!setting.IsBeingModified)
	{
		if (ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
		{
			setting.IsBeingModified = true;
			setting.DisplayKeyBindString[0] = '\0';
		}
	}
	else
	{
		if (ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
		{
			setting.IsBeingModified = false;
			setting.CancelPending = false;
			setting.DisplayKeyBindString[0] = '\0';
			setting.LastKeyBindString.clear();
			setting.SetCallback(KeySequence());
		}
		else if (setting.CancelPending) 
		{
			setting.IsBeingModified = false;
			setting.CancelPending = false;
			strcpy_s(setting.DisplayKeyBindString, setting.LastKeyBindString.c_str());
		}
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(windowWidth * 0.5f);

	ImGui::Text(name.c_str());

	ImGui::PopItemWidth();
}