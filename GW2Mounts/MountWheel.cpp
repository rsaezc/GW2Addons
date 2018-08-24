#define _USE_MATH_DEFINES
#include <math.h>
#include <functional>
#include "resource.h"
#include "inputs.h"
#include "MountWheel.h"

#define SQUARE(x) ((x) * (x))

MountWheel::MountWheel(Mounts *mount_list)
{
	if (!mount_list)
	{
		throw std::invalid_argument("Null pointer");
	}
	MountList = mount_list;
	WheelFadeInEffect.SetEffectDuration(1000);
	WheelFadeInEffect.SetEffectSteps(6);
	MountHoverEffect.SetEffectDuration(1000);
	MountHoverEffect.SetEffectSteps(6);
}

MountWheel::~MountWheel()
{
	ReleaseResources();
	MountList = nullptr;
}

void MountWheel::Show()
{
	if (State == RESOURCES_LOADED)
	{
		State = WINDOW_VISIBLE;
		WheelFadeInEffect.Start();

		if (ActionModeEnabled)
		{
			WheelPosition.x = WheelPosition.y = 0.5f;

			RECT rect = { 0 };
			if (GetWindowRect(GameWindow, &rect))
			{
				if (SetCursorPos((rect.right - rect.left) / 2 + rect.left, (rect.bottom - rect.top) / 2 + rect.top))
				{
					MousePos.x = (LONG)(ScreenSize.cx * 0.5f);
					MousePos.y = (LONG)(ScreenSize.cy * 0.5f);
				}
			}
		}
		else
		{
			(void)GetCursorPos(&MousePos);
			WheelPosition.x = MousePos.x / (float)ScreenSize.cx;
			WheelPosition.y = MousePos.y / (float)ScreenSize.cy;
		}

		DetermineHoveredMount();
	}
}

bool MountWheel::IsVisible()
{
	return (State == WINDOW_VISIBLE);
}

void MountWheel::SetKeyBind(const KeySequence& keybind)
{
	KeyBind = keybind;
}

KeySequence& MountWheel::GetKeyBind()
{
	return KeyBind;
}

void MountWheel::SetWheelScale(float scale)
{
	WheelScale = scale;
}

float MountWheel::GetWheelScale()
{
	return WheelScale;
}

void MountWheel::EnableActionMode(bool enable)
{
	ActionModeEnabled = enable;
}

bool MountWheel::isActionModeEnabled()
{
	return ActionModeEnabled;
}

void MountWheel::SetScreenSize(uint width, uint height)
{
	ScreenSize.cx = width;
	ScreenSize.cy = height;
}

void MountWheel::LoadResources(IDirect3DDevice9 * dev, HMODULE dll, HWND game_window)
{
	if ((State == RESOURCES_NO_INIT) && dev && dll && game_window)
	{
		Device = dev;
		DllModule = dll;
		GameWindow = game_window;
		try
		{
			Quad = std::make_unique<UnitQuad>(Device);
		}
		catch (...)
		{
			Quad = nullptr;
			return;
		}
		ID3DXBuffer* errorBuffer = nullptr;
		D3DXCreateEffectFromResource(Device, DllModule, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr, &MainEffect, &errorBuffer);
		if (errorBuffer)
		{
			errorBuffer->Release();
			errorBuffer = nullptr;
		}
		if (!MainEffect)
		{
			Quad.reset();
			return;
		}
		D3DXCreateTextureFromResource(Device, DllModule, MAKEINTRESOURCE(IDR_BACKGROUND), &BackgroundTexture);
		if (!BackgroundTexture)
		{
			Quad.reset();
			MainEffect->Release();
			MainEffect = nullptr;
			return;
		}
		MountList->LoadTextures(Device, DllModule);
		State = RESOURCES_LOADED;
	}
}

void MountWheel::ReleaseResources()
{
	if (State != RESOURCES_NO_INIT)
	{
		Quad.reset();
		MountList->UnloadTextures();
		BackgroundTexture->Release();
		BackgroundTexture = nullptr;
		MainEffect->Release();
		MainEffect = nullptr;
		Device = nullptr;
		DllModule = nullptr;
		GameWindow = nullptr;
		State = RESOURCES_NO_INIT;
	}
}

bool MountWheel::ProcessInputEvents(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (State != WINDOW_VISIBLE)
	{
		return false;
	}

	switch (msg)
	{
	case WM_KILLFOCUS:
		Hide();
		break;
	case WM_MOUSEMOVE:
		MousePos.x = (signed short)(lParam);
		MousePos.y = (signed short)(lParam >> 16);
		if (!CameraEnabled)
		{
			DetermineHoveredMount();
			if (DragEnabled)
			{
				WheelPosition.x += (MousePos.x - DragStartPos.x) / (float)ScreenSize.cx;
				WheelPosition.y += (MousePos.y - DragStartPos.y) / (float)ScreenSize.cy;
				DragStartPos = MousePos;
				return true;
			}
			if (MouseOverWheel || ActionModeEnabled)
			{
				return true;
			}
		}
		break;
	case WM_INPUT:
		if (ActionModeEnabled)
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
				lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
				return true;
		}
		break;
	case WM_LBUTTONDOWN:
		LeftMousePressed = true;
		if (ActionModeEnabled)
		{
			return true;
		}
		if (MouseOverWheel)
		{
			return true;
		}
		CameraEnabled = true;
		break;
	case WM_LBUTTONUP:
		if (LeftMousePressed)
		{
			if (CurrentMountHovered == Mounts::NONE)
			{
				CurrentMountHovered = MountList->GetFavoriteMount();
			}
			KeySequence mount_keybind;
			if (MountList->GetMountKeyBind(CurrentMountHovered, mount_keybind))
			{
				SendKeybind(mount_keybind);
			}
			bool left_mouse_bypass = CameraEnabled;
			Hide();
			if (!left_mouse_bypass)
			{
				return true;
			}
		}
		break;
	case WM_RBUTTONDOWN:
		if (ActionModeEnabled)
		{
			return true;
		}
		if (MouseOverWheel)
		{
			DragEnabled = true;
			DragStartPos = MousePos;
			return true;
		}
		CameraEnabled = true;
		break;
	case WM_RBUTTONUP:
		if (ActionModeEnabled)
		{
			return true;
		}
		if (DragEnabled)
		{
			DragEnabled = false;
			return true;
		}
		if (CameraEnabled)
		{
			MousePos.x = (signed short)(lParam);
			MousePos.y = (signed short)(lParam >> 16);
			CameraEnabled = false;
		}
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		Hide();
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			EscPressed = true;
			return true;
		}
		Hide();
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if ((wParam == VK_ESCAPE) && EscPressed)
		{
			Hide();
			return true;
		}
		break;
	}
	return false;
}

void MountWheel::Draw()
{
	if (State != WINDOW_VISIBLE)
	{
		return;
	}

	Quad->Bind();

	uint passes = 0;

	// Setup viewport
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = (DWORD)ScreenSize.cx;
	vp.Height = (DWORD)ScreenSize.cy;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	Device->SetViewport(&vp);

	D3DXVECTOR4 screenSize((float)ScreenSize.cx, (float)ScreenSize.cy, 1.f / ScreenSize.cx, 1.f / ScreenSize.cy);
	D3DXVECTOR4 baseSpriteDimensions;
	baseSpriteDimensions.x = WheelPosition.x;
	baseSpriteDimensions.y = WheelPosition.y;
	baseSpriteDimensions.z = WheelScale * 0.5f * screenSize.y * screenSize.z;
	baseSpriteDimensions.w = WheelScale * 0.5f;

	MainEffect->SetTechnique("BgImage");
	MainEffect->SetTexture("texBgImage", BackgroundTexture);
	MainEffect->SetVector("g_vSpriteDimensions", &baseSpriteDimensions);
	MainEffect->SetFloat("g_fFadeInProgress", WheelFadeInEffect.GetProgress());
	MainEffect->SetFloat("g_fHoverProgress", MountHoverEffect.GetProgress());
	MainEffect->SetFloat("g_fTimer", fmod(timeInMS() / 1010.f, 55000.f));
	MainEffect->SetFloat("g_fDeadZoneScale", 0.2f);
	MainEffect->SetInt("g_iMountCount", Mounts::NUMBER_MOUNTS);
	MainEffect->SetInt("g_iMountHovered", CurrentMountHovered);
	MainEffect->SetBool("g_bCenterGlow", (CurrentMountHovered == Mounts::NONE));
	MainEffect->Begin(&passes, 0);
	MainEffect->BeginPass(0);
	Quad->Draw();
	MainEffect->EndPass();
	MainEffect->End();

	MainEffect->SetTechnique("MountImage");
	MainEffect->SetTexture("texBgImage", BackgroundTexture);
	MainEffect->SetVector("g_vScreenSize", &screenSize);
	MainEffect->Begin(&passes, 0);
	MainEffect->BeginPass(0);

	for (int it = Mounts::RAPTOR; it < Mounts::NUMBER_MOUNTS; it++)
	{
		Mounts::Mount mount = static_cast<Mounts::Mount>(it);
		D3DXVECTOR4 spriteDimensions = baseSpriteDimensions;

		float mountAngle = (float)it / (float)Mounts::NUMBER_MOUNTS * 2 * (float)M_PI;
		D3DXVECTOR2 mountLocation = D3DXVECTOR2(cos(mountAngle - (float)M_PI / 2), sin(mountAngle - (float)M_PI / 2)) * 0.25f * 0.66f;

		spriteDimensions.x += mountLocation.x * spriteDimensions.z;
		spriteDimensions.y += mountLocation.y * spriteDimensions.w;

		float mountDiameter = (float)sin((2 * M_PI / (double)Mounts::NUMBER_MOUNTS) / 2) * 2.f * 0.2f * 0.66f;
		if (mount == CurrentMountHovered)
			mountDiameter *= lerp(1.f, 1.1f, smoothstep(MountHoverEffect.GetProgress()));
		else
			mountDiameter *= 0.9f;

		spriteDimensions.z *= mountDiameter;
		spriteDimensions.w *= mountDiameter;

		int v[3] = { it, it, Mounts::NUMBER_MOUNTS };
		MainEffect->SetValue("g_iMountID", v, sizeof(v));
		MainEffect->SetBool("g_bMountHovered", (mount == CurrentMountHovered));
		MainEffect->SetTexture("texMountImage", MountList->GetMountTexture(mount));
		MainEffect->SetVector("g_vSpriteDimensions", &spriteDimensions);
		std::array<float, 4> mount_color = { 255, 255, 255, 255 };
		(void)MountList->GetMountColor(mount, mount_color);
		MainEffect->SetValue("g_vColor", mount_color.data(), sizeof(D3DXVECTOR4));
		MainEffect->CommitChanges();

		Quad->Draw();
	}

	MainEffect->EndPass();
	MainEffect->End();

	if (ActionModeEnabled)
	{
		MainEffect->SetTechnique("Cursor");
		MainEffect->SetTexture("texBgImage", BackgroundTexture);
		MainEffect->SetVector("g_vSpriteDimensions", &D3DXVECTOR4(MousePos.x * screenSize.z, MousePos.y * screenSize.w, 0.05f  * screenSize.y * screenSize.z, 0.05f));

		MainEffect->Begin(&passes, 0);
		MainEffect->BeginPass(0);
		Quad->Draw();
		MainEffect->EndPass();
		MainEffect->End();
	}
}

void MountWheel::Hide()
{
	if (State == WINDOW_VISIBLE)
	{
		State = RESOURCES_LOADED;
		DragEnabled = false;
		MouseOverWheel = false;
		CameraEnabled = false;
		LeftMousePressed = false;
		EscPressed = false;
		CurrentMountHovered = Mounts::NONE;
	}
}

void MountWheel::DetermineHoveredMount()
{
	D3DXVECTOR2 mouse_pos;
	mouse_pos.x = MousePos.x / (float)ScreenSize.cx;
	mouse_pos.y = MousePos.y / (float)ScreenSize.cy;
	mouse_pos -= WheelPosition;

	mouse_pos.y *= (float)ScreenSize.cy / (float)ScreenSize.cx;

	Mounts::Mount LastMountHovered = CurrentMountHovered;


	FLOAT d3dx_mouse_pos = D3DXVec2LengthSq(&mouse_pos);

	if (d3dx_mouse_pos > SQUARE(WheelScale * 0.135f)) //Out of wheel
	{
		MouseOverWheel = false;
		CurrentMountHovered = Mounts::NONE;
	}
	else
	{
		MouseOverWheel = true;
		// Middle circle does not count as a hover event
		if (d3dx_mouse_pos > SQUARE(WheelScale * 0.135f * 0.2f))
		{
			float MouseAngle = atan2(-mouse_pos.y, -mouse_pos.x) - 0.5f * (float)M_PI;
			if (MouseAngle < 0)
			{
				MouseAngle += float(2 * M_PI);
			}

			float MountAngle = float(2 * M_PI) / Mounts::NUMBER_MOUNTS;
			int MountId = int((MouseAngle - MountAngle / 2) / MountAngle + 1) % Mounts::NUMBER_MOUNTS;

			CurrentMountHovered = static_cast<Mounts::Mount>(MountId);
			if (!MountList->IsMountEnabled(CurrentMountHovered))
			{
				CurrentMountHovered = Mounts::NONE;
			}
		}
		else
		{
			CurrentMountHovered = Mounts::NONE;
		}
	}

	if ((CurrentMountHovered != Mounts::NONE) && (LastMountHovered != CurrentMountHovered))
	{
		MountHoverEffect.Start();
	}
		
}

