#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>
#include "UnitQuad.h"
#include "EffectProgressTimer.h"
#include "Mounts.h"
#include "custom_types.h"

class MountWheel
{
public:
	MountWheel(Mounts *mount_list);
	~MountWheel();

	void Show();
	bool IsVisible();

	bool IsWaitingEvent();
	void DismountEndEvent();

	void SetKeyBind(const KeySequence& keybind);
	KeySequence& GetKeyBind();

	void SetDismountKeyBind(const KeySequence& keybind);
	KeySequence& GetDismountKeyBind();

	void SetDismountSignature(const std::string signature);
	std::string GetDismountSignature();

	void SetWheelScale(float scale);
	float GetWheelScale();

	void EnableActionMode(bool enable);
	bool IsActionModeEnabled();

	void EnableDismountCalibration(bool enable);
	bool IsDismountCalibrationEnabled();
	void SetDismountIconPos(POINT position);
	POINT GetDismountIconPos();

	void SetScreenSize(uint width, uint height);

	void LoadResources(IDirect3DDevice9* dev, HMODULE dll, HWND game_window);
	void ReleaseResources();

	bool ProcessInputEvents(UINT msg, WPARAM wParam, LPARAM lParam);
	void Draw();

private:
	enum WindowState
	{
		RESOURCES_NO_INIT,
		RESOURCES_LOADED,
		WINDOW_VISIBLE,
		WINDOW_WAIT_EVENT
	};
	WindowState State = RESOURCES_NO_INIT;

	KeySequence KeyBind = { VK_SHIFT, VK_MENU, 'Z' };
	KeySequence DismountKeyBind;
	std::string DismountSignature;
	float WheelScale = 1.f;
	bool ActionModeEnabled = false;
	bool DismountCalibration = false;
	POINT DismountIconPos = { 0, 0 };	

	SIZE ScreenSize = { 0, 0 };
	D3DXVECTOR2 WheelPosition;
	POINT MousePos = { 0, 0 };
	bool DragEnabled = false;
	POINT DragStartPos = { 0, 0 };
	bool MouseOverWheel = false;
	bool CameraEnabled = false;
	bool LeftMousePressed = false;
	bool EscPressed = false;

	Mounts::Mount CurrentMountHovered = Mounts::NONE;
	Mounts* MountList = nullptr;
	
	EffectProgressTimer WheelFadeInEffect;
	EffectProgressTimer MountHoverEffect;

	std::unique_ptr<UnitQuad> Quad = nullptr;
	ID3DXEffect* MainEffect = nullptr;
	IDirect3DTexture9* BackgroundTexture = nullptr;
	IDirect3DTexture9* ActionCursorTexture = nullptr;
	IDirect3DDevice9* Device = nullptr;
	HMODULE DllModule = nullptr;
	HWND GameWindow = nullptr;

	void DoShow();
	void Hide();
	void DetermineHoveredMount();
	std::string GetDismountSignatureFromScreenCapture();
};

