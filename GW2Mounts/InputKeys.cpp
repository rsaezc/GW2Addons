#include "InputKeys.h"
#include "Utility.h"

#define IS_VALID_INPUT_MESSAGE(WM_MESSAGE) (((WM_MESSAGE) == WmQueue)\
										   || ((WM_MESSAGE) == WM_INPUT)\
										   || (((WM_MESSAGE) >= WM_KEYFIRST) && ((WM_MESSAGE) <= WM_KEYLAST))\
										   || (((WM_MESSAGE) >= WM_MOUSEFIRST) && ((WM_MESSAGE) <= WM_MOUSELAST)))

#define IS_INPUTKEY(WM_MESSAGE) (((WM_MESSAGE) == WM_XBUTTONUP) || ((WM_MESSAGE) == WM_XBUTTONDOWN)\
								|| ((WM_MESSAGE) == WM_MBUTTONUP) || ((WM_MESSAGE) == WM_MBUTTONDOWN)\
								|| ((WM_MESSAGE) == WM_SYSKEYUP) || ((WM_MESSAGE) == WM_SYSKEYDOWN)\
								|| ((WM_MESSAGE) == WM_KEYUP) || ((WM_MESSAGE) == WM_KEYDOWN))

#define IS_KEYEVENT(KEY) (((KEY) == VK_MENU) || ((KEY) == VK_SHIFT) || ((KEY) == VK_CONTROL)\
						 || ((KEY) >= VK_MBUTTON) && ((KEY) <= VK_XBUTTON2)\
						 || ((KEY) > VK_HELP) && ((KEY) < VK_LWIN))

uint InputKeys::WmQueue;
KeySequence InputKeys::PressedKeys;
KeySequence InputKeys::LastPressedKeys;
std::deque<InputKeys::EventKey> InputKeys::EventKeys;
std::deque<InputKeys::DelayedInput> InputKeys::KeybindQueue;
std::deque<InputKeys::DelayedInput> InputKeys::InputKeyQueue;
InputKeys::InputEvent InputKeys::LastInputEvent;

InputKeys::DelayedInput InputKeys::DelayedInputFromVirtualKey(uint virtual_key, bool down, mstime time_to_send)
{
	DelayedInput delayed_input;

	if (!IS_KEYEVENT(virtual_key))
	{
		delayed_input.key.msg = WM_NULL;
	}
	else
	{
		delayed_input.time_to_send = time_to_send;
		if ((virtual_key == VK_MBUTTON) || (virtual_key == VK_XBUTTON1) || (virtual_key == VK_XBUTTON2))
		{
			delayed_input.key.wParam = delayed_input.key.lParam = 0;
			for (const auto& key_pressed : PressedKeys)
			{
				switch (key_pressed)
				{
				case VK_CONTROL:
					delayed_input.key.wParam |= MK_CONTROL;
					break;
				case VK_SHIFT:
					delayed_input.key.wParam |= MK_SHIFT;
					break;
				case VK_MBUTTON:
					delayed_input.key.wParam |= MK_MBUTTON;
					break;
				case VK_XBUTTON1:
					delayed_input.key.wParam |= MK_XBUTTON1;
					break;
				case VK_XBUTTON2:
					delayed_input.key.wParam |= MK_XBUTTON2;
					break;
				}
			}
			if (GetKeyState(VK_RBUTTON) & 0x8000) delayed_input.key.wParam |= MK_RBUTTON;
			if (GetKeyState(VK_LBUTTON) & 0x8000) delayed_input.key.wParam |= MK_LBUTTON;
			if (virtual_key == VK_XBUTTON1)
			{
				delayed_input.key.wParam |= MAKEWPARAM(0, XBUTTON1);
			}
			else if (virtual_key == VK_XBUTTON2)
			{
				delayed_input.key.wParam |= MAKEWPARAM(0, XBUTTON2);
			}

			POINT mouse_pos;
			(void)GetCursorPos(&mouse_pos);

			delayed_input.key.lParam = MAKELPARAM(((int)mouse_pos.x), ((int)mouse_pos.y));
		}
		else
		{
			delayed_input.key.wParam = virtual_key;
			delayed_input.key.lParam = 1U;
			delayed_input.key.lParam |= ((MapVirtualKeyEx(virtual_key, MAPVK_VK_TO_VSC, 0) & 0xFF) << 16);
			if (!down)
				delayed_input.key.lParam |= (3U << 30);
		}

		switch (virtual_key)
		{
		case VK_XBUTTON1:
		case VK_XBUTTON2:
			delayed_input.key.msg = down ? WM_XBUTTONDOWN : WM_XBUTTONUP;
			break;
		case VK_MBUTTON:
			delayed_input.key.msg = down ? WM_MBUTTONDOWN : WM_MBUTTONUP;
			break;
		case VK_MENU:
			delayed_input.key.msg = down ? WM_SYSKEYDOWN : WM_KEYUP;
			break;
		default:
			delayed_input.key.msg = down ? WM_KEYDOWN : WM_KEYUP;
			break;
		}
	}
	return delayed_input;
}

void InputKeys::SendKeybind(const KeySequence& virtual_keys)
{
	if (virtual_keys.empty())
		return;

	mstime currentTime = timeInMS() + 10;
	//if alt pressed, released it to prevent disruptions
	if (GetKeyState(VK_MENU) & 0x8000)
	{
		DelayedInput input_key = DelayedInputFromVirtualKey(VK_MENU, false, currentTime);
		KeybindQueue.push_back(input_key);
		currentTime += 20;
	}

	for (const auto& virtual_key : virtual_keys)
	{
		if (std::find(PressedKeys.begin(), PressedKeys.end(), virtual_key) != PressedKeys.end())
			continue;

		DelayedInput input_key = DelayedInputFromVirtualKey(virtual_key, true, currentTime);
		if (input_key.key.msg != WM_NULL)
		{
			KeybindQueue.push_back(input_key);
			currentTime += 20;
		}
	}

	currentTime += 50;

	for (KeySequence::const_reverse_iterator virtual_key = virtual_keys.rbegin(); virtual_key != virtual_keys.rend(); ++virtual_key)
	{
		if (std::find(PressedKeys.begin(), PressedKeys.end(), *virtual_key) != PressedKeys.end())
			continue;

		DelayedInput input_key = DelayedInputFromVirtualKey(*virtual_key, false, currentTime);
		if (input_key.key.msg != WM_NULL)
		{
			KeybindQueue.push_back(input_key);
			currentTime += 20;
		}
	}
}

bool InputKeys::IsKeybindSent()
{
	return KeybindQueue.empty();
}

const KeySequence& InputKeys::GetPressedKeys()
{
	return PressedKeys;
}

const KeySequence& InputKeys::GetLastPressedKeys()
{
	return LastPressedKeys;
}

void InputKeys::ProcessInputKey(InputKey input_key)
{
	LastInputEvent = INPUT_NO_EVENT;

	// Generate EventKey list from input key
	bool eventDown = false;
	switch (input_key.msg)
	{
	case WM_SYSKEYDOWN:
		eventDown = true;
	case WM_SYSKEYUP:
		//WM_KEYUP: VK_MENU is not sent when alt is released, so we manage the 
		//VK_MENU explicitly.
		if (IS_KEYEVENT(input_key.wParam) && (input_key.wParam != VK_MENU))
		{
			if (((input_key.lParam >> 29) & 1) == 1)
			{
				if (eventDown)
				{
					EventKeys.push_back({ (uint)VK_MENU, true });
					EventKeys.push_back({ (uint)input_key.wParam, true });
				}
				else
				{
					EventKeys.push_back({ (uint)input_key.wParam, false });
					EventKeys.push_back({ (uint)VK_MENU, false });
				}
				
			}
		}
		break;
	case WM_KEYDOWN:
		eventDown = true;
	case WM_KEYUP:
		if (IS_KEYEVENT(input_key.wParam))
		{
			//AltGr down--> WM_KEYDOWN: VK_CONTROL and WM_KEYDOWN: VK_MENU with extended
			//AltGr up--> WM_SYSKEYUP: VK_CONTROL and WM_KEYUP: VK_MENU with extended
			// So, removing VK_MENU with extended, AltGr acts as control key
			if ((input_key.wParam == VK_MENU) && (((input_key.lParam >> 24) & 1U) == 1U))
			{
				break;
			}
			EventKeys.push_back({ (uint)input_key.wParam, eventDown });
		}
		break;
	case WM_MBUTTONDOWN:
		eventDown = true;
	case WM_MBUTTONUP:
		EventKeys.push_back({ VK_MBUTTON, eventDown });
		break;
	case WM_XBUTTONDOWN:
		eventDown = true;
	case WM_XBUTTONUP:
		EventKeys.push_back({ (uint)(GET_XBUTTON_WPARAM(input_key.wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
		break;
	}

	// Apply key events now
	LastPressedKeys = PressedKeys;
	for (const auto& event_key : EventKeys)
	{
		uint last_key_pressed = 0U;
		if (!PressedKeys.empty())
		{
			last_key_pressed = PressedKeys.back();
		}

		if (event_key.is_down)
		{
			//Prevent filling pressed key queue with key repetition.
			if (last_key_pressed != event_key.virtual_key)
			{
				PressedKeys.push_back(event_key.virtual_key);
				LastInputEvent = INPUT_PRESS_EVENT;
			}
		}
		else
		{
			if (last_key_pressed == event_key.virtual_key)
			{
				PressedKeys.pop_back();
			}
			else //Key released out of order
			{
				PressedKeys.clear();
			}
			LastInputEvent = INPUT_RELEASE_EVENT;
		}
	}
}

void InputKeys::InitInputQueue()
{
	WmQueue = RegisterWindowMessage(TEXT("WM_INPUTQUEUE"));
}

bool InputKeys::ProcessInputKeyFromInputMessage(InputKey& input_key, UINT msg, WPARAM wParam, LPARAM lParam)
{
	input_key = { msg, wParam, lParam};

	if (!IS_VALID_INPUT_MESSAGE(input_key.msg))
	{
		return false;
	}

	if ((input_key.msg != WmQueue) && !IS_INPUTKEY(input_key.msg))
	{
		return true;
	}

	if (!KeybindQueue.empty())
	{
		if (input_key.msg == WmQueue)
		{
			input_key = KeybindQueue.front().key;
			KeybindQueue.pop_front();
		}
		else
		{
			DelayedInput delayed = { input_key, timeInMS() + 20 };
			InputKeyQueue.push_back(delayed);
			input_key.msg = WM_NULL;
		}
		return false;
	}

	if (!InputKeyQueue.empty())
	{
		if (input_key.msg != WmQueue)
		{
			DelayedInput delayed = { input_key, timeInMS() + 20 };
			InputKeyQueue.push_back(delayed);
			input_key.msg = WM_NULL;
			return false;
		}
		input_key = InputKeyQueue.front().key;
		InputKeyQueue.pop_front();
	}

	/* This shouldn't happen, something wrong. */
	if (input_key.msg == WmQueue)
	{
		input_key.msg = WM_NULL;
		return false;
	}

	ProcessInputKey(input_key);
	return true;
}

InputKeys::InputEvent InputKeys::GetInputEvent()
{
	return LastInputEvent;
}

void InputKeys::ClearInputEvents()
{
	EventKeys.clear();
	LastInputEvent = INPUT_NO_EVENT;
}

void InputKeys::ClearInput()
{
	ClearInputEvents();
	KeybindQueue.clear();
	InputKeyQueue.clear();
	PressedKeys.clear();
}

void InputKeys::SendQueuedInputs(HWND window)
{
	DelayedInput input;

	if (!KeybindQueue.empty())
	{
		input = KeybindQueue.front();
	}
	else if (!InputKeyQueue.empty())
	{
		input = InputKeyQueue.front();
	}
	else
	{
		return;
	}

	mstime currentTime = timeInMS();
	if (currentTime >= input.time_to_send)
	{
		PostMessage(window, WmQueue, 0U, 0);
	}
}

