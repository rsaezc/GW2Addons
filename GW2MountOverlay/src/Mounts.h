#pragma once
#include <array>
#include <d3d9.h>
#include "custom_types.h"

class Mounts
{
public:
	enum Mount : int
	{
		NONE = -1,
		RAPTOR = 0,
		SPRINGER,
		SKIMMER,
		JACKAL,
		BEETLE,
		GRIFFON,
		NUMBER_MOUNTS
	};

	Mounts();
	~Mounts();

	const char* GetMountName(Mounts::Mount mount);

	void SetFavoriteMount(Mounts::Mount mount);
	Mounts::Mount GetFavoriteMount();

	void SetMountKeyBind(Mounts::Mount mount, const KeySequence& key_bind);
	bool GetMountKeyBind(Mounts::Mount mount, KeySequence& key_bind);

	bool IsMountEnabled(Mounts::Mount mount);

	bool GetMountColor(Mounts::Mount mount, std::array<float, 4>&);

	void LoadTextures(IDirect3DDevice9 * dev, HMODULE dll);
	void UnloadTextures();
	IDirect3DTexture9* GetMountTexture(Mounts::Mount mount);
	IDirect3DTexture9* GetMountLogoTexture(Mounts::Mount mount);

private:
	struct MountType {
		IDirect3DTexture9* MountTexture;
		IDirect3DTexture9* MountLogoTexture;
		KeySequence KeyBind;
		bool Enabled;
		MountType() : MountTexture(nullptr), MountLogoTexture(nullptr), Enabled(false) {};
	};

	const std::array<float, 4> MountColors[6] = {
	{ 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 },
	{ 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 },
	{ 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 },
	{ 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 },
	{ 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 },
	{ 136 / 255.f, 123 / 255.f, 195 / 255.f, 1 }
	};

	Mount FavoriteMount = RAPTOR;
	MountType MountArray[NUMBER_MOUNTS];

	IDirect3DDevice9* Device = nullptr;
	HMODULE DllModule = nullptr;
};

