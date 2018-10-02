#pragma once
#include <functional>
#include <imgui.h>
#include "custom_types.h"

#define ImGuiAddVerticalSpace(v_space) (ImGui::Dummy(ImVec2(0.0f, (float)(v_space))))

#define ImGuiCenterWindowOnScreen() do {\
									ImGuiIO& io = ImGui::GetIO();\
									ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f,\
									io.DisplaySize.y * 0.5f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));\
								    } while(0)

#define ImGuiSetMinimumWindowSize(min_size)	(ImGui::SetNextWindowSizeConstraints((min_size),\
											 ImVec2(FLT_MAX, FLT_MAX)))

#define ImGuiSetNextRightAlignWithIndent(widget_size) (ImGui::SameLine((ImGui::GetWindowWidth()\
													   - (widget_size)) - ImGui::GetStyle().IndentSpacing))

struct ImGuiKeybind
{
	char DisplayKeyBindString[256];
	std::string LastKeyBindString;
	bool IsBeingModified = false;
	bool CancelPending = false;
	std::function<void(const KeySequence&)> SetCallback;

	void InitKeybind(const KeySequence& keys);
	void UpdateKeybind(const KeySequence& keys, bool apply);
	void CancelKeybind();

private:
	void UpdateDisplayString(const KeySequence& keys);
};
