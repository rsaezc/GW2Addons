#pragma once
#include <functional>
#include <imgui.h>
#include "custom_types.h"

struct ImGuiKeybind
{
	char DisplayKeyBindString[256];
	std::string LastKeyBindString;
	bool IsBeingModified = false;
	std::function<void(const KeySequence&)> SetCallback;

	void InitKeybind(const KeySequence& keys);
	void UpdateKeybind(const KeySequence& keys, bool apply);
	void CancelKeybind();

private:
	void UpdateDisplayString(const KeySequence& keys);
};

ImVec4 operator/(const ImVec4& v, float f);
void ImGuiKeybindInput(const std::string& name, ImGuiKeybind& setting);