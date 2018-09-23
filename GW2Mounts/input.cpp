#include "inputs.h"
#include "Utility.h"
#include "imgui/imgui.h"

KeySequence DownKeys;
std::list<EventKey> EventKeys;

struct DelayedInput
{
	uint msg;
	WPARAM wParam;
	LPARAM lParam;

	mstime t;
};

DelayedInput TransformVKey(uint vk, bool down, mstime t)
{
	DelayedInput i;
	i.t = t;
	if (vk == VK_LBUTTON || vk == VK_MBUTTON || vk == VK_RBUTTON)
	{
		i.wParam = i.lParam = 0;
		if (DownKeys.count(VK_CONTROL))
			i.wParam += MK_CONTROL;
		if (DownKeys.count(VK_SHIFT))
			i.wParam += MK_SHIFT;
		if (DownKeys.count(VK_LBUTTON))
			i.wParam += MK_LBUTTON;
		if (DownKeys.count(VK_RBUTTON))
			i.wParam += MK_RBUTTON;
		if (DownKeys.count(VK_MBUTTON))
			i.wParam += MK_MBUTTON;

		const auto& io = ImGui::GetIO();

		i.lParam = MAKELPARAM(((int)io.MousePos.x), ((int)io.MousePos.y));
	}
	else
	{
		i.wParam = vk;
		i.lParam = 1;
		i.lParam += (MapVirtualKeyEx(vk, MAPVK_VK_TO_VSC, 0) & 0xFF) << 16;
		if (!down)
			i.lParam += (1 << 30) + (1 << 31);
	}

	switch (vk)
	{
	case VK_LBUTTON:
		i.msg = down ? WM_LBUTTONDOWN : WM_LBUTTONUP;
		break;
	case VK_MBUTTON:
		i.msg = down ? WM_MBUTTONDOWN : WM_MBUTTONUP;
		break;
	case VK_RBUTTON:
		i.msg = down ? WM_RBUTTONDOWN : WM_RBUTTONUP;
		break;
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
	case VK_PRIOR: case VK_NEXT: // page up and page down
	case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		i.lParam |= 1 << 24; // set extended bit
	default:
		i.msg = down ? WM_KEYDOWN : WM_KEYUP;
		break;
	}

	return i;
}


std::list<DelayedInput> QueuedInputs;

void SendKeybind(const KeySequence& vkeys)
{
	if (vkeys.empty())
		return;

	std::list<uint> vkeys_sorted(vkeys.begin(), vkeys.end());
	vkeys_sorted.sort([](uint& a, uint& b) {
		if (a == VK_CONTROL || a == VK_SHIFT || a == VK_MENU)
			return true;
		else
			return a < b;
	});

	mstime currentTime = timeInMS() + 10;

	for (const auto& vk : vkeys_sorted)
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, true, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}

	currentTime += 50;

	for (const auto& vk : reverse(vkeys_sorted))
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, false, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}
}

void ProcessEventKeysFromInputMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Generate our EventKey list for the current message
	bool eventDown = false;
	switch (msg)
	{
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		eventDown = true;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		if ((msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) && wParam != VK_F10)
		{
			if (((lParam >> 29) & 1) == 1)
				EventKeys.push_back({ VK_MENU, true });
			else
				EventKeys.push_back({ VK_MENU, false });
		}
		if (wParam != VK_ESCAPE)
		{
			EventKeys.push_back({ (uint)wParam, eventDown });
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
		EventKeys.push_back({ (uint)(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
		break;
	}

	// Apply key events now
	for (const auto& k : EventKeys)
	{
		if (k.down)
		{
			DownKeys.insert(k.vk);
		}
		else
		{
			DownKeys.erase(k.vk);
		}
	}
}

bool SendQueuedInputs(HWND window)
{
	if (QueuedInputs.empty())
		return false;

	mstime currentTime = timeInMS();

	auto& qi = QueuedInputs.front();

	if (currentTime < qi.t)
		return true;

	PostMessage(window, qi.msg, qi.wParam, qi.lParam);

	QueuedInputs.pop_front();
	return true;
}
