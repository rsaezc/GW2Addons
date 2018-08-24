#pragma once
#include <set>
#include <list>
#include "custom_types.h"

struct EventKey
{
	uint vk : 31;
	bool down : 1;
};

extern KeySequence DownKeys;
extern std::list<EventKey> EventKeys;

void SendQueuedInputs(HWND window);
void SendKeybind(const KeySequence& vkeys);
void ProcessEventKeysFromInputMessage(UINT msg, WPARAM wParam, LPARAM lParam);