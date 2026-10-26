#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string.h>
#include "loading_bg.h"

namespace loading_bg
{
	// NOTE: The game looks up the loading background layer by the 10 character
	// string "loading_bg". Replacing it with a name that doesn't exist hides it.
	// The exe's sections are renamed by its protector, so every data section is scanned.
	static const char Target[] = "loading_bg";
	static const char Replacement[] = "hidden_bg0";

	static BYTE* Find(BYTE* begin, BYTE* end)
	{
		__try
		{
			// NOTE: memchr is vectorised, which matters because the sections are hundreds of MB
			for (BYTE* p = begin; p < end && (p = (BYTE*)memchr(p, 'l', end - p)) != nullptr; p++)
			{
				// NOTE: Include the terminator so longer strings aren't matched
				if (memcmp(p, Target, sizeof(Target)) == 0)
					return p;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		return nullptr;
	}

	void Hide()
	{
		HMODULE module = GetModuleHandleA(nullptr);
		auto dos = (PIMAGE_DOS_HEADER)module;
		auto nt = (PIMAGE_NT_HEADERS)((BYTE*)module + dos->e_lfanew);
		auto section = IMAGE_FIRST_SECTION(nt);

		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
		{
			if ((section->Characteristics & IMAGE_SCN_MEM_EXECUTE) || !(section->Characteristics & IMAGE_SCN_MEM_READ))
				continue;

			BYTE* begin = (BYTE*)module + section->VirtualAddress;
			BYTE* end = begin + section->Misc.VirtualSize - sizeof(Target);

			BYTE* p = Find(begin, end);
			if (!p)
				continue;

			DWORD old;
			if (VirtualProtect(p, sizeof(Target) - 1, PAGE_READWRITE, &old))
			{
				memcpy(p, Replacement, sizeof(Target) - 1);
				VirtualProtect(p, sizeof(Target) - 1, old, &old);
			}
			return;
		}
	}
}
