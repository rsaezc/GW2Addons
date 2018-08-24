#include <imgui.h>
#include <examples\imgui_impl_dx9.h>
#include <examples\imgui_impl_win32.h>
#include <Shlobj.h>
#include <tchar.h>
#include <sstream>
#include <vector>
#include "Utility.h"
#include "inputs.h"
#include "ConfigurationWindow.h"

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

std::set<unsigned int>& ConfigurationWindow::GetKeyBind()
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
		ConfigKeybind.SetCallback = [this](const std::set<unsigned int>& val) { UpdateConfigKeybind(val); };
		WheelKeybind.InitKeybind(WheelWindow->GetKeyBind());
		WheelKeybind.SetCallback = [this](const std::set<unsigned int>& val) { UpdateWheelKeybind(val); };
		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			std::set<unsigned int> mount_keybind;
			Mounts::Mount mount = static_cast<Mounts::Mount>(i);
			if (MountList->GetMountKeyBind(mount, mount_keybind))
			{
				MountKeybinds[i].InitKeybind(mount_keybind);
			}
			MountKeybinds[i].SetCallback = [this, mount](const std::set<unsigned int>& val) { UpdateMountKeybind(mount, val); };
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

	if (!EventKeys.empty()) //Update keybinds with down event keys
	{
		// If a key was lifted, we consider the key combination *prior* to this key being lifted as the keybind
		bool keyLifted = false;
		auto fullKeybind = DownKeys;
		for (const auto& ek : EventKeys)
		{
			if (!ek.down)
			{
				fullKeybind.insert(ek.vk);
				keyLifted = true;
			}
		}

		ConfigKeybind.UpdateKeybind(fullKeybind, keyLifted);
		WheelKeybind.UpdateKeybind(fullKeybind, keyLifted);

		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			MountKeybinds[i].UpdateKeybind(fullKeybind, keyLifted);
		}
	}
	
	auto& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_KILLFOCUS:
		Hide();
		break;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		if (io.WantCaptureMouse)
		{
			ret_val = true;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		ConfigKeybind.CancelKeybind();
		WheelKeybind.CancelKeybind();
		for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
		{
			MountKeybinds[i].CancelKeybind();
		}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
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
			if (wParam == VK_ESCAPE)
			{
				ConfigKeybind.CancelKeybind();
				WheelKeybind.CancelKeybind();
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

	bool window_visible = true;
	ImGui::Begin("Overlay Options Menu", &window_visible);
	if (!window_visible)
	{
		Hide();
	}

	ImGuiKeybindInput("Option menu Keybind", ConfigKeybind);

	ImGui::Separator();
	ImGui::Text("Overlay options"); 
	ImGuiKeybindInput("Overlay Keybind", WheelKeybind);

	float wheel_scale = WheelWindow->GetWheelScale();
	if (ImGui::SliderFloat("Overlay Scale", &wheel_scale, 0.f, 4.f))
	{
		UpdateWheelScale(wheel_scale);
	}

	bool action_mode = WheelWindow->isActionModeEnabled();
	if (ImGui::Checkbox("Overlay in action mode", &action_mode))
	{
		UpdateWheelActionMode(action_mode);
	}

	ImGui::Separator();
	ImGui::Text("Mount options");

	//Favorite mount list: get current enabled mounts
	std::vector<const char*> FavoriteMountNames;
	FavoriteMountNames.push_back("None");
	for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
	{
		Mounts::Mount mount = static_cast<Mounts::Mount>(i);
		if (MountList->IsMountEnabled(mount))
		{
			FavoriteMountNames.push_back(MountList->GetMountName(mount));
		}
	}
	int favorite_mount = MountList->GetFavoriteMount() + 1;
	if (ImGui::Combo("Favorite Mount", &favorite_mount, FavoriteMountNames.data(), (int)FavoriteMountNames.size()))
	{
		Mounts::Mount fav_mount = static_cast<Mounts::Mount>(favorite_mount - 1);
		UpdateFavoriteMount(fav_mount);
	}

	ImGui::Text("Keybinds (same as game keybinds)");
	for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
	{
		Mounts::Mount mount = static_cast<Mounts::Mount>(i);
		ImGuiKeybindInput(MountList->GetMountName(mount), MountKeybinds[i]);
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
	TCHAR exeFullPath[MAX_PATH];
	GetModuleFileName(0, exeFullPath, MAX_PATH);
	tstring exeFolder;
	SplitFilename(exeFullPath, &exeFolder, nullptr);
	ConfigIniFolder = exeFolder + TEXT("\\addons\\mounts\\");
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
	bool wheel_action_mode = WheelWindow->isActionModeEnabled();
	wheel_action_mode = ConfigIniHandler.GetBoolValue("General", "mount_wheel_action_mode", wheel_action_mode);
	WheelWindow->EnableActionMode(wheel_action_mode);
	float wheel_scale = WheelWindow->GetWheelScale();
	wheel_scale = (float)ConfigIniHandler.GetDoubleValue("General", "mount_wheel_scale", wheel_scale);
	WheelWindow->SetWheelScale(wheel_scale);

	KeySequence keybind = KeyBind;
	const char* config_keybind = ConfigIniHandler.GetValue("Keybinds", "configuration_window", nullptr);
	KeybindFromString(config_keybind, keybind);
	if (!keybind.empty()) KeyBind = keybind;
	keybind = WheelWindow->GetKeyBind();
	const char* wheel_keybind = ConfigIniHandler.GetValue("Keybinds", "mount_wheel", nullptr);
	KeybindFromString(wheel_keybind, keybind);
	if (!keybind.empty()) WheelWindow->SetKeyBind(keybind);

	for (int i = 0; i < Mounts::NUMBER_MOUNTS; i++)
	{
		Mounts::Mount mount = static_cast<Mounts::Mount>(i);
		const char* mount_keybind = ConfigIniHandler.GetValue("Keybinds", MountList->GetMountName(mount), nullptr);
		KeybindFromString(mount_keybind, keybind);
		MountList->SetMountKeyBind(mount, keybind);
	}

	Mounts::Mount favorite_mount = MountList->GetFavoriteMount();
	favorite_mount = static_cast<Mounts::Mount>(ConfigIniHandler.GetLongValue("General", "favorite_mount", (int)favorite_mount));
	MountList->SetFavoriteMount(favorite_mount);
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
			out.insert((uint)val);
		}
	}
}