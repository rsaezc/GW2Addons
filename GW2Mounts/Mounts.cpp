#include <d3dx9.h>
#include "resource.h"
#include "Mounts.h"

#define MOUNT_IS_ENABLED(pMOUNT)   (!(pMOUNT)->KeyBind.empty())
#define MOUNT_IS_VALID(mount)	   (((mount) != NONE) && ((mount) < NUMBER_MOUNTS))
#define RESOURCES_LOADED()		   (Device)

Mounts::Mounts()
{
}

Mounts::~Mounts()
{
}

const char* Mounts::GetMountName(Mounts::Mount mount)
{
	switch (mount)
	{
	case RAPTOR:
		return "Raptor";
	case SPRINGER:
		return "Springer";
	case SKIMMER:
		return "Skimmer";
	case JACKAL:
		return "Jackal";
	case BEETLE:
		return "Beetle";
	case GRIFFON:
		return "Griffon";
	default:
		return "[Unknown]";
	}
}

void Mounts::SetFavoriteMount(Mounts::Mount mount)
{
	FavoriteMount = NONE;
	if (MOUNT_IS_VALID(mount))
	{
		if (MOUNT_IS_ENABLED(&MountArray[mount]))
		{
			FavoriteMount = mount;
		}
	}
}

Mounts::Mount Mounts::GetFavoriteMount()
{
	return FavoriteMount;
}

void Mounts::SetMountKeyBind(Mounts::Mount mount, const KeySequence& key_bind)
{
	if (MOUNT_IS_VALID(mount))
	{
		MountType *mount_data = &MountArray[mount];
		if (mount_data->KeyBind.empty() != key_bind.empty()) //Mount state change 
		{
			//Update favorite mount
			if (mount == FavoriteMount) //If favorite mount, it was enabled
			{
				FavoriteMount = NONE;
			}
			//Update texture
			if (RESOURCES_LOADED())
			{
				if (mount_data->MountTexture)
				{
					mount_data->MountTexture->Release();
					mount_data->MountTexture = nullptr;
				}
				if (MOUNT_IS_ENABLED(mount_data))
				{
					D3DXCreateTextureFromResource(Device, DllModule, MAKEINTRESOURCE(IDR_DISABLED_MOUNTS + mount), &(mount_data->MountTexture));
				}
				else
				{
					D3DXCreateTextureFromResource(Device, DllModule, MAKEINTRESOURCE(IDR_ENABLED_MOUNTS + mount), &(mount_data->MountTexture));
				}
			}
		}
		mount_data->KeyBind = key_bind;
	}
}

bool Mounts::GetMountKeyBind(Mounts::Mount mount, KeySequence& key_bind)
{
	if (MOUNT_IS_VALID(mount))
	{
		key_bind = MountArray[mount].KeyBind;
		return true;
	}
	return false;
}

bool Mounts::IsMountEnabled(Mounts::Mount mount)
{
	if (MOUNT_IS_VALID(mount))
	{
		return MOUNT_IS_ENABLED(&MountArray[mount]);
	}
	return false;
}

bool Mounts::GetMountColor(Mounts::Mount mount, std::array<float, 4>& color)
{
	if (MOUNT_IS_VALID(mount))
	{
		color = MountColors[mount];
		return true;
	}
	return false;
}

void Mounts::LoadTextures(IDirect3DDevice9 * dev, HMODULE dll)
{
	if (dev && dll)
	{
		if ((Device != dev) || (DllModule != dll))
		{
			UnloadTextures();
		}
	    Device = dev;
		DllModule = dll;
		for (int it = RAPTOR; it < NUMBER_MOUNTS; it++)
		{
			MountType *mount_data = &MountArray[it];
			if (MOUNT_IS_ENABLED(mount_data))
			{
				D3DXCreateTextureFromResource(Device, DllModule, MAKEINTRESOURCE(IDR_ENABLED_MOUNTS + it), &(mount_data->MountTexture));
			}
			else
			{
				D3DXCreateTextureFromResource(Device, DllModule, MAKEINTRESOURCE(IDR_DISABLED_MOUNTS + it), &(mount_data->MountTexture));
			}
		}
	}
}

void Mounts::UnloadTextures()
{
	if (RESOURCES_LOADED())
	{
		Device = nullptr;
		DllModule = nullptr;
		for (int it = RAPTOR; it < NUMBER_MOUNTS; it++)
		{
			MountType *mount_data = &MountArray[it];
			if (mount_data->MountTexture)
			{
				mount_data->MountTexture->Release();
				mount_data->MountTexture = nullptr;
			}
		}
	}
}

IDirect3DTexture9* Mounts::GetMountTexture(Mounts::Mount mount)
{
	if (MOUNT_IS_VALID(mount))
	{
		return MountArray[mount].MountTexture;
	}
	return nullptr;
}

