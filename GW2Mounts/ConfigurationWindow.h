#pragma once
#include <d3d9.h>
#include "simpleini\SimpleIni.h"
#include "imgui_ext.h"
#include "Mounts.h"
#include "MountWheel.h"

class ConfigurationWindow
{
public:
	ConfigurationWindow(MountWheel* wheel_window, Mounts* mount_list);
	~ConfigurationWindow();

	void Show();
	bool IsVisible();

	KeySequence& GetKeyBind();

	void InitResources();
	void ConfigureResources(IDirect3DDevice9* dev, HWND game_window);
	void LoadResources();
	void ReleaseResources();
	void DeInitResources();

	bool ProcessInputEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Draw();

private:
	const uint DefaultKeyBind[3] = { VK_SHIFT, VK_MENU, 'M' };

	enum WindowState 
	{
		RESOURCES_NO_INIT,
		RESOURCES_INIT,
		RESOURCES_CONFIG,
		RESOURCES_LOADED,
		WINDOW_VISIBLE
	};
	WindowState State = RESOURCES_NO_INIT;

	KeySequence KeyBind = { VK_SHIFT, VK_MENU, 'M' };
	bool EscPressed = false;

	MountWheel* WheelWindow = nullptr;
	Mounts*	    MountList = nullptr;

	ImGuiKeybind ConfigKeybind;
	ImGuiKeybind WheelKeybind;
	ImGuiKeybind MountKeybinds[Mounts::NUMBER_MOUNTS];

	// Config file settings
	const TCHAR* ConfigName = TEXT("config.ini");
	const TCHAR* ImGuiConfigName = TEXT("imgui_config.ini");

	CSimpleIniA ConfigIniHandler;
	tstring ConfigIniFolder;
	TCHAR ConfigIniLocation[MAX_PATH];
	char ImGuiConfigIniLocation[MAX_PATH];

	void Hide();

	void LoadConfiguration();
	void LoadCustomStyle();
	void DrawKeybindInput(const std::string & name, ImGuiKeybind & setting);

	// Config update events
	void UpdateConfigKeybind(const KeySequence& val);
	void UpdateWheelKeybind(const KeySequence& val);
	void UpdateMountKeybind(Mounts::Mount mount, const KeySequence& val);
	void UpdateWheelScale(float scale);
	void UpdateWheelActionMode(bool enable);
	void UpdateFavoriteMount(Mounts::Mount mount);

	//Utils
	void KeybindFromString(const char* keys, KeySequence& out);
};
