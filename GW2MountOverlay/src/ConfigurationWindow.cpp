#include <imgui.h>
#include <examples\imgui_impl_dx9.h>
#include <examples\imgui_impl_win32.h>
#include <Shlobj.h>
#include <tchar.h>
#include <sstream>
#include <vector>
#include "Utility.h"
#include "InputKeys.h"
#include "ConfigurationWindow.h"

/* Colors */
#define FONT_COLOR					(IM_COL32(210, 208, 191, 255))
#define BORDER_COLOR				FONT_COLOR
#define WINDOW_BG_COLOR				(IM_COL32(87, 59, 31, 200))
#define TITLE_BG_COLOR				(IM_COL32(20, 20, 20, 255))
#define TITLE_ACTIVE_COLOR			(IM_COL32(40, 40, 40, 255))
#define HEADER_BG_COLOR				TITLE_BG_COLOR
#define HEADER_HOVER_COLOR			(IM_COL32(50, 50, 50, 255))
#define WIDGET_BG_COLOR				HEADER_BG_COLOR
#define WIDGET_HOVER_COLOR			HEADER_HOVER_COLOR
#define INPUT_TEXT_BG_COLOR			(IM_COL32(255, 255, 255, 50))
#define INPUT_TEXT_ACTIVE_COLOR		(IM_COL32(20, 20, 20, 50))
#define BUTTON_BG_COLOR				FONT_COLOR
#define BUTTON_HOVER_COLOR			(IM_COL32(240, 238, 211, 255))
#define BUTTON_TEXT_COLOR			HEADER_BG_COLOR
#define CLOSE_BUTTON_HOVER_COLOR	(IM_COL32(70, 70, 70, 255))
#define SLIDER_GRAB_COLOR			BUTTON_BG_COLOR
#define SLIDER_GRAB_ACTIVE_COLOR	BUTTON_HOVER_COLOR
#define COMBO_ARROW_COLOR			WIDGET_BG_COLOR
#define COMBO_ARROW_HOVER_COLOR		WIDGET_HOVER_COLOR
#define CHECKBOX_TICK_COLOR			BUTTON_HOVER_COLOR
#define RESIZE_GRIP_COLOR			BUTTON_BG_COLOR
#define RESIZE_GRIP_HOVER_COLOR		BUTTON_HOVER_COLOR

/* Style */
#define WINDOW_TITLE_HEIGHT			(ImVec2(10.0f, 10.0f))
#define	WINDOW_RECT_BORDER			(0.0f)
#define WIDGET_NO_BORDER			(0.0f)
#define WIDGET_WITH_BORDER			(1.0f)
#define V_SPACE_BETWEEN_GROUPS		(5.0f)
#define V_SPACE_INSIDE_GROUPS		(3.0f)

/* Sizes */ 
#define WINDOW_MIN_SIZE				(ImVec2(388.0f, 475.0f))
#define INPUT_TEXT_WIDTH			(ImGui::GetWindowWidth() * 0.38f)
#define BUTTON_WIDTH				(ImGui::GetWindowWidth() * 0.10f)
#define COMBO_WIDTH					(ImGui::GetWindowWidth() * 0.48f)
#define SLIDER_WIDTH				COMBO_WIDTH

ConfigurationWindow::ConfigurationWindow(MountWheel* wheel_window, Mounts* mount_list)
{
	if (!wheel_window || !mount_list)
	{
		throw std::invalid_argument("Null pointer");
	}
	WheelWindow = wheel_window;
    MountList = mount_list;
}

ConfigurationWindow::~ConfigurationWindow()
{
	DeInitResources();
	WheelWindow = nullptr;
	MountList = nullptr;
}

void ConfigurationWindow::Show()
{
	if (State == RESOURCES_LOADED)
	{
		State = WINDOW_VISIBLE;
	}
}

bool ConfigurationWindow::IsVisible()
{
	return (State == WINDOW_VISIBLE);
}

const KeySequence& ConfigurationWindow::GetKeyBind()
{
	return KeyBind;
}

void ConfigurationWindow::InitResources()
{
	if (State == RESOURCES_NO_INIT)
	{
		ImGui::CreateContext();

		LoadConfiguration();

		// Set ImGui keybinds
		ConfigKeybind.InitKeybind(KeyBind);
		ConfigKeybind.SetCallback = [this](const KeySequence& val) { UpdateConfigKeybind(val); };
		WheelKeybind.InitKeybind(WheelWindow->GetKeyBind());
		WheelKeybind.SetCallback = [this](const KeySequence& val) { UpdateWheelKeybind(val); };
		DismountKeybind.InitKeybind(WheelWindow->GetDismountKeyBind());
		DismountKeybind.SetCallback = [this](const KeySequence& val) { UpdateDismountKeybind(val); };
		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			KeySequence mount_keybind;
			Mounts::Mount mount = static_cast<Mounts::Mount>(i);
			if (MountList->GetMountKeyBind(mount, mount_keybind))
			{
				MountKeybinds[i].InitKeybind(mount_keybind);
			}
			MountKeybinds[i].SetCallback = [this, mount](const KeySequence& val) { UpdateMountKeybind(mount, val); };
		}

		// Init ImGui
		auto& imio = ImGui::GetIO();
		imio.IniFilename = ImGuiConfigIniLocation;
		State = RESOURCES_INIT;
	}
}

void ConfigurationWindow::ConfigureResources(IDirect3DDevice9* dev, HWND game_window)
{
	if (State == RESOURCES_INIT)
	{
		// Setup ImGui binding
		ImGui_ImplDX9_Init(dev);
		ImGui_ImplWin32_Init(game_window);
		State = RESOURCES_CONFIG;
	}
}

void ConfigurationWindow::LoadResources()
{
	if (State == RESOURCES_CONFIG)
	{
		ImGui_ImplDX9_CreateDeviceObjects();
		State = RESOURCES_LOADED;
	}
}

void ConfigurationWindow::ReleaseResources()
{
	if (State == RESOURCES_LOADED)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		State = RESOURCES_CONFIG;
	}
}

void ConfigurationWindow::DeInitResources()
{
	if (State != RESOURCES_NO_INIT)
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		State = RESOURCES_NO_INIT;
	}
}

extern IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool ConfigurationWindow::ProcessInputEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
	bool ret_val = false;

	if (State != WINDOW_VISIBLE)
	{
		return ret_val;
	}

	InputKeys::InputEvent input_event = InputKeys::GetInputEvent();
	if (input_event != InputKeys::INPUT_NO_EVENT) //Update keybinds with down event keys
	{
		// If a key was lifted, we consider the key combination *prior* to this key being lifted as the keybind
		bool keyLifted = false;
		KeySequence fullKeybind;
		if (input_event == InputKeys::INPUT_RELEASE_EVENT)
		{
			fullKeybind = InputKeys::GetLastPressedKeys();
			keyLifted = true;
		}
		else
		{
			fullKeybind = InputKeys::GetPressedKeys();
		}

		ConfigKeybind.UpdateKeybind(fullKeybind, keyLifted);
		WheelKeybind.UpdateKeybind(fullKeybind, keyLifted);
		DismountKeybind.UpdateKeybind(fullKeybind, keyLifted);

		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			MountKeybinds[i].UpdateKeybind(fullKeybind, keyLifted);
		}
	}
	
	auto& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		if (io.WantCaptureMouse)
		{
			ret_val = true;
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		ConfigKeybind.CancelKeybind();
		WheelKeybind.CancelKeybind();
		DismountKeybind.CancelKeybind();
		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			MountKeybinds[i].CancelKeybind();
		}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		if (io.WantCaptureMouse)
		{
			ret_val = true;
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (io.WantCaptureKeyboard)
		{
			ret_val = true;
		}
		else
		{
			if (wParam == VK_ESCAPE)
			{
				EscPressed = true;
				ret_val = true;
			}
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (io.WantCaptureKeyboard)
		{
			ret_val = true;
		}
		else
		{
			if ((wParam == VK_ESCAPE) && EscPressed)
			{
				ConfigKeybind.CancelKeybind();
				WheelKeybind.CancelKeybind();
				DismountKeybind.CancelKeybind();
				for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
				{
					MountKeybinds[i].CancelKeybind();
				}
				Hide();
				ret_val = true;
			}
		}
		break;
	case WM_CHAR:
		if (io.WantTextInput)
		{
			ret_val = true;
		}
		break;
	default:
		ret_val = false;
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	
	return ret_val;
}

void ConfigurationWindow::Draw()
{
	if (State != WINDOW_VISIBLE)
	{
		return;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	LoadCustomStyle();

	/* Center window at start-up and set minimum size */
	ImGuiCenterWindowOnScreen();
	ImGuiSetMinimumWindowSize(WINDOW_MIN_SIZE);

	bool window_visible = true;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
									| ImGuiWindowFlags_NoScrollWithMouse
									| ImGuiWindowFlags_NoCollapse;
	/* Title height */
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, WINDOW_TITLE_HEIGHT);
	/* Button X */
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, CLOSE_BUTTON_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, CLOSE_BUTTON_HOVER_COLOR);
	ImGui::Begin("Overlay Options Menu", &window_visible, window_flags);
	if (!window_visible)
	{
		Hide();
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();

	ImGui::Indent();
    DrawKeybindInput("Option menu Keybind:", ConfigKeybind);
	ImGui::Unindent();
	
	ImGuiAddVerticalSpace(V_SPACE_BETWEEN_GROUPS);

	if (ImGui::CollapsingHeader("Overlay options", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		ImGui::Indent();

		DrawKeybindInput("Overlay Keybind", WheelKeybind);

		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		ImGui::Text("Overlay Scale:");
		float item_width = SLIDER_WIDTH;
		ImGuiSetNextRightAlignWithIndent(item_width);
		ImGui::PushItemWidth(item_width);
		int wheel_scale = (int)(WheelWindow->GetWheelScale() * 100);
		if (ImGui::SliderInt("", &wheel_scale, 10, 150, "%d%%"))
		{
			UpdateWheelScale(wheel_scale / 100.0f);
		}
		ImGui::PopItemWidth();

		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		bool action_mode = WheelWindow->IsActionModeEnabled();
		if (ImGui::Checkbox("Overlay in action mode.", &action_mode))
		{
			UpdateWheelActionMode(action_mode);
		}

		ImGui::Unindent();
	}

	ImGuiAddVerticalSpace(V_SPACE_BETWEEN_GROUPS);

	if (ImGui::CollapsingHeader("Mount options", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		ImGui::Indent();

		ImGui::Text("Favorite Mount:");
		float item_width = COMBO_WIDTH;
		ImGuiSetNextRightAlignWithIndent(item_width);
		ImGui::PushItemWidth(item_width);

		/* Favorite mount list: get current enabled mounts */
		std::vector<const char*> FavoriteMountNames;
		FavoriteMountNames.push_back("None");
		std::vector<Mounts::Mount> EnabledMounts;
		EnabledMounts.push_back(Mounts::NONE);
		int favorite_mount = 0;
		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			Mounts::Mount mount = static_cast<Mounts::Mount>(i);
			if (MountList->IsMountEnabled(mount))
			{
				EnabledMounts.push_back(mount);
				FavoriteMountNames.push_back(MountList->GetMountName(mount));
				if (MountList->GetFavoriteMount() == mount)
				{
					favorite_mount = (int)EnabledMounts.size() - 1;
				}
			}
		}
		/* Arrow button color */
		ImGui::PushStyleColor(ImGuiCol_Button, COMBO_ARROW_COLOR);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COMBO_ARROW_HOVER_COLOR);
		if (ImGui::Combo("", &favorite_mount, FavoriteMountNames.data(), (int)FavoriteMountNames.size()))
		{
			Mounts::Mount fav_mount = EnabledMounts.at(favorite_mount);
			UpdateFavoriteMount(fav_mount);
		}
		ImGui::PopStyleColor(2);
		ImGui::PopItemWidth();
		
		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		ImGui::Text("Keybinds (same as game keybinds):");
		ImGui::Indent();

		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		DrawKeybindInput("Dismount", DismountKeybind);

		bool dismount_calibration = WheelWindow->IsDismountCalibrationEnabled();
		if (ImGui::Checkbox("Dismount calibration.", &dismount_calibration))
		{
			UpdateDismountCalibration(dismount_calibration);
		}

		ImGuiAddVerticalSpace(V_SPACE_INSIDE_GROUPS);

		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			Mounts::Mount mount = static_cast<Mounts::Mount>(i);
			DrawKeybindInput(MountList->GetMountName(mount), MountKeybinds[i]);
		}
		ImGui::Unindent();

		ImGui::Unindent();
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void ConfigurationWindow::Hide()
{
	if (State == WINDOW_VISIBLE)
	{
		State = RESOURCES_LOADED;
		EscPressed = false;
	}
}

void ConfigurationWindow::LoadConfiguration()
{
	// Create folders
	TCHAR user_personal_path[MAX_PATH];
	SHGetFolderPath(0, CSIDL_PERSONAL, GetCurrentProcessToken(), SHGFP_TYPE_CURRENT, user_personal_path);	
	tstring config_path = user_personal_path;
	ConfigIniFolder = config_path + TEXT("\\Guild Wars 2\\Addons\\GW2MountOverlay\\");
	_tcscpy_s(ConfigIniLocation, (ConfigIniFolder + ConfigName).c_str());
#if _UNICODE
	strcpy_s(ImGuiConfigIniLocation, ws2s(ConfigIniFolder + ImGuiConfigName).c_str());
#else
	strcpy_s(ImGuiConfigIniLocation, (ConfigIniFolder + ImGuiConfigName).c_str());
#endif
	SHCreateDirectoryEx(nullptr, ConfigIniFolder.c_str(), nullptr);

	// Load INI settings
	ConfigIniHandler.SetUnicode();
	ConfigIniHandler.LoadFile(ConfigIniLocation);
	bool wheel_action_mode = WheelWindow->IsActionModeEnabled();
	wheel_action_mode = ConfigIniHandler.GetBoolValue("General", "mount_wheel_action_mode", wheel_action_mode);
	WheelWindow->EnableActionMode(wheel_action_mode);
	float wheel_scale = WheelWindow->GetWheelScale();
	wheel_scale = (float)ConfigIniHandler.GetDoubleValue("General", "mount_wheel_scale", wheel_scale);
	WheelWindow->SetWheelScale(wheel_scale);
	const char* dismount_signature = ConfigIniHandler.GetValue("Dismount_Calibration", "icon_signature", nullptr);
	if (dismount_signature)
	{
		WheelWindow->SetDismountSignature(std::string(dismount_signature));
	}
	POINT dismount_icon_pos = WheelWindow->GetDismountIconPos();
	dismount_icon_pos.x = ConfigIniHandler.GetLongValue("Dismount_Calibration", "icon_position_x", dismount_icon_pos.x);
	dismount_icon_pos.y = ConfigIniHandler.GetLongValue("Dismount_Calibration", "icon_position_y", dismount_icon_pos.y);
	WheelWindow->SetDismountIconPos(dismount_icon_pos);
	

	KeySequence keybind = KeyBind;
	const char* config_keybind = ConfigIniHandler.GetValue("Keybinds", "configuration_window", nullptr);
	KeybindFromString(config_keybind, keybind);
	if (!keybind.empty()) KeyBind = keybind;
	keybind = WheelWindow->GetKeyBind();
	const char* wheel_keybind = ConfigIniHandler.GetValue("Keybinds", "mount_wheel", nullptr);
	KeybindFromString(wheel_keybind, keybind);
	if (!keybind.empty()) WheelWindow->SetKeyBind(keybind);
	const char* dismount_keybind = ConfigIniHandler.GetValue("Keybinds", "dismount", nullptr);
	KeybindFromString(dismount_keybind, keybind);
	if (!keybind.empty()) WheelWindow->SetDismountKeyBind(keybind);
	for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
	{
		Mounts::Mount mount = static_cast<Mounts::Mount>(i);
		const char* mount_keybind = ConfigIniHandler.GetValue("Keybinds", MountList->GetMountName(mount), nullptr);
		KeybindFromString(mount_keybind, keybind);
		MountList->SetMountKeyBind(mount, keybind);
	}
	/* To update the favorite mount correctly, first the mounts must be activated setting their keybinds. */
	Mounts::Mount favorite_mount = MountList->GetFavoriteMount();
	favorite_mount = static_cast<Mounts::Mount>(ConfigIniHandler.GetLongValue("General", "favorite_mount", (int)favorite_mount));
	MountList->SetFavoriteMount(favorite_mount);
}

void ConfigurationWindow::LoadCustomStyle()
{
	/* Set window style */
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, WINDOW_RECT_BORDER);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_WITH_BORDER);
	/* Set common colors */
	ImGui::PushStyleColor(ImGuiCol_Text, FONT_COLOR);
	ImGui::PushStyleColor(ImGuiCol_Border, BORDER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, WINDOW_BG_COLOR);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, TITLE_BG_COLOR);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, TITLE_ACTIVE_COLOR);
	ImGui::PushStyleColor(ImGuiCol_Header, HEADER_BG_COLOR);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, HEADER_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HEADER_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, WIDGET_BG_COLOR);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, WIDGET_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, WIDGET_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_BG_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BUTTON_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, BUTTON_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, SLIDER_GRAB_COLOR);
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, SLIDER_GRAB_ACTIVE_COLOR);
	ImGui::PushStyleColor(ImGuiCol_CheckMark, CHECKBOX_TICK_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, RESIZE_GRIP_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, RESIZE_GRIP_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, RESIZE_GRIP_HOVER_COLOR);
}

void ConfigurationWindow::DrawKeybindInput(const std::string& name, ImGuiKeybind& setting)
{
	std::string suffix = "##" + name;

	float input_text_width = INPUT_TEXT_WIDTH;
	float button_width = BUTTON_WIDTH;

	ImGui::Text(name.c_str());

	ImGuiSetNextRightAlignWithIndent(input_text_width + button_width);

	ImGui::PushItemWidth(input_text_width);
	if (setting.IsBeingModified)
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, INPUT_TEXT_ACTIVE_COLOR);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, INPUT_TEXT_BG_COLOR);
	}
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, WIDGET_NO_BORDER);

	ImGui::InputText(suffix.c_str(), setting.DisplayKeyBindString, 256, ImGuiInputTextFlags_ReadOnly);

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();

	ImGuiSetNextRightAlignWithIndent(button_width);

	ImGui::PushStyleColor(ImGuiCol_Text, BUTTON_TEXT_COLOR);
	if (!setting.IsBeingModified)
	{
		if (ImGui::Button(("Set" + suffix).c_str(), ImVec2(button_width, 0.f)))
		{
			setting.IsBeingModified = true;
			setting.DisplayKeyBindString[0] = '\0';
		}
	}
	else
	{
		if (ImGui::Button(("Clear" + suffix).c_str(), ImVec2(button_width, 0.f)))
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
	ImGui::PopStyleColor();
}

void ConfigurationWindow::UpdateConfigKeybind(const KeySequence& val)
{
	if (val.empty()) //Prevent losing the keybind for configuration window
	{
		KeyBind = KeySequence(DefaultKeyBind, DefaultKeyBind + 3);
	}
	else
	{
		KeyBind = val;
	}
	
	std::string setting_value = "";
	for (const auto& k : KeyBind)
		setting_value += std::to_string(k) + ", ";

	ConfigIniHandler.SetValue("Keybinds", "configuration_window", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::UpdateWheelKeybind(const KeySequence& val)
{
	WheelWindow->SetKeyBind(val);
	std::string setting_value = "";
	for (const auto& k : val)
		setting_value += std::to_string(k) + ", ";

	ConfigIniHandler.SetValue("Keybinds", "mount_wheel", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::UpdateDismountKeybind(const KeySequence& val)
{
	WheelWindow->SetDismountKeyBind(val);
	std::string setting_value = "";
	for (const auto& k : val)
		setting_value += std::to_string(k) + ", ";

	ConfigIniHandler.SetValue("Keybinds", "dismount", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::UpdateMountKeybind(Mounts::Mount mount, const KeySequence& val)
{
	MountList->SetMountKeyBind(mount, val);
	std::string setting_value = "";
	for (const auto& k : val)
		setting_value += std::to_string(k) + ", ";

	ConfigIniHandler.SetValue("Keybinds", MountList->GetMountName(mount), (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::UpdateWheelScale(float scale)
{
	WheelWindow->SetWheelScale(scale);
	ConfigIniHandler.SetDoubleValue("General", "mount_wheel_scale", scale);
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::UpdateWheelActionMode(bool enable)
{
	WheelWindow->EnableActionMode(enable);
	ConfigIniHandler.SetBoolValue("General", "mount_wheel_action_mode", enable);
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::UpdateDismountCalibration(bool enable)
{
	WheelWindow->EnableDismountCalibration(enable);
	if (!enable)
	{
		ConfigIniHandler.SetValue("Dismount_Calibration", "icon_signature", WheelWindow->GetDismountSignature().c_str());
		POINT dismount_icon_pos = WheelWindow->GetDismountIconPos();
		ConfigIniHandler.SetLongValue("Dismount_Calibration", "icon_position_x", dismount_icon_pos.x);
		ConfigIniHandler.SetLongValue("Dismount_Calibration", "icon_position_y", dismount_icon_pos.y);
		ConfigIniHandler.SaveFile(ConfigIniLocation);
	}
}

void ConfigurationWindow::UpdateFavoriteMount(Mounts::Mount mount)
{
	MountList->SetFavoriteMount(mount);
	ConfigIniHandler.SetLongValue("General", "favorite_mount", (int)MountList->GetFavoriteMount());
	ConfigIniHandler.SaveFile(ConfigIniLocation);
}

void ConfigurationWindow::KeybindFromString(const char* keys, KeySequence& out)
{
	out.clear();
	if (strnlen_s(keys, 256) > 0)
	{
		std::stringstream ss(keys);

		while (ss.good())
		{
			std::string substr;
			std::getline(ss, substr, ',');
			int val = std::stoi(substr);
			out.push_back((uint)val);
		}
	}
}