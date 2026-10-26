#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

namespace input
{
	// NOTE: GetAsyncKeyState reads the global keyboard, so ignore keys typed into other windows
	inline bool GameFocused()
	{
		DWORD pid = 0;
		GetWindowThreadProcessId(GetForegroundWindow(), &pid);
		return pid == GetCurrentProcessId();
	}

	// NOTE: True only on the frame the key goes down
	inline bool Pressed(int32_t key, bool& wasDown)
	{
		bool down = GameFocused() && (GetAsyncKeyState(key) & 0x8000) != 0;
		bool pressed = down && !wasDown;
		wasDown = down;
		return pressed;
	}
}
