#pragma once
#include "custom_types.h"

class InputKeys
{
public:
	struct InputKey
	{
		uint msg;
		WPARAM wParam;
		LPARAM lParam;
	};

	enum InputEvent
	{
		INPUT_NO_EVENT,
		INPUT_PRESS_EVENT,
		INPUT_RELEASE_EVENT
	};

	static void InitInputQueue();
	static bool ProcessInputKeyFromInputMessage(InputKey& input_key, UINT msg, WPARAM wParam, LPARAM lParam);
	static InputEvent GetInputEvent();
	static void ClearInputEvents();
	static void ClearInput();
	static void SendQueuedInputs(HWND window);
	static void SendKeybind(const KeySequence& virtual_keys);
	static bool IsKeybindSent();
	static const KeySequence& GetPressedKeys();
	static const KeySequence& GetLastPressedKeys();

private:
	struct DelayedInput
	{
		InputKey key;
		mstime time_to_send;
	};

	struct EventKey
	{
		uint virtual_key : 31;
		bool is_down : 1;
	};

	static uint WmQueue;
	static KeySequence PressedKeys;
	static KeySequence LastPressedKeys;
	static std::deque<EventKey> EventKeys;
	static std::deque<DelayedInput> KeybindQueue;
	static std::deque<DelayedInput> InputKeyQueue;
	static InputEvent LastInputEvent;

	static DelayedInput DelayedInputFromVirtualKey(uint virtual_key, bool is_down, mstime time_to_send);
	static void ProcessInputKey(InputKey input_key);
};

