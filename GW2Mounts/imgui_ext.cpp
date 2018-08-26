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
