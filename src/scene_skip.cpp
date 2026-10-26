#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string.h>
#include "scene_skip.h"

namespace scene_skip
{
	static const int DispSlot = 4;

	typedef void(__fastcall* DispFn)(void*);

	static const char* TaskNames[] =
	{
		".?AVTaskStage@stage_detail@@",
		".?AVTaskMovie@@",
		".?AVTaskRobDisp@@",
		".?AVTaskAuth3d@auth_3d_detail@@",
	};

	static const int TaskCount = sizeof(TaskNames) / sizeof(TaskNames[0]);

	static DispFn Originals[TaskCount];
	static int32_t HotKey = 0;
	static bool Skipping = false;
	static bool Forced = false;
	static bool KeyDown = false;
	static bool ExternalPoll = false;
	static BYTE* Image;

	void Poll()
	{
		if (!HotKey)
			return;

		bool down = (GetAsyncKeyState(HotKey) & 0x8000) != 0;
		if (down && !KeyDown && !Forced)
			Skipping = !Skipping;
		KeyDown = down;
	}

	void SetExternalPoll(bool external)
	{
		ExternalPoll = external;
	}

	void SetForced(bool forced)
	{
		// NOTE: Leaving full dim resets the PV to rendering
		if (Forced && !forced)
			Skipping = false;
		Forced = forced;
	}

	template <int N>
	static void __fastcall Hooked(void* self)
	{
		// NOTE: Many tasks can draw per frame, so only poll here when nothing else does it once per frame
		if (!ExternalPoll)
			Poll();
		if (!Skipping && !Forced)
			Originals[N](self);
	}

	static DispFn Hooks[TaskCount] = { Hooked<0>, Hooked<1>, Hooked<2>, Hooked<3> };

	static bool IsReadable(const void* p, size_t size)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQuery(p, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT)
			return false;
		if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;
		return (BYTE*)p + size <= (BYTE*)mbi.BaseAddress + mbi.RegionSize;
	}

	static BYTE* FindBytes(BYTE* begin, BYTE* end, const void* pat, size_t size, size_t step)
	{
		const BYTE first = *(const BYTE*)pat;
		for (BYTE* p = begin; p + size <= end; p += step)
		{
			// NOTE: memchr is vectorised, which matters because the sections are hundreds of MB
			if (step == 1)
			{
				p = (BYTE*)memchr(p, first, end - size + 1 - p);
				if (!p)
					return nullptr;
			}

			if (p[0] == first && memcmp(p, pat, size) == 0)
				return p;
		}
		return nullptr;
	}

	// NOTE: Only readable non-executable sections are searched, and only their initialised bytes.
	// The exe's sections are renamed by its protector, so they can't be picked by name.
	template <typename F>
	static BYTE* ForEachSection(F search)
	{
		auto nt = (PIMAGE_NT_HEADERS)(Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);
		auto section = IMAGE_FIRST_SECTION(nt);

		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
		{
			if ((section->Characteristics & IMAGE_SCN_MEM_EXECUTE) || !(section->Characteristics & IMAGE_SCN_MEM_READ))
				continue;

			BYTE* begin = Image + section->VirtualAddress;
			BYTE* end = begin + min(section->Misc.VirtualSize, section->SizeOfRawData);

			__try
			{
				if (BYTE* result = search(begin, end))
					return result;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
		return nullptr;
	}

	static void** FindVtable(const char* rttiName)
	{
		size_t nameSize = strlen(rttiName) + 1;
		BYTE* name = ForEachSection([&](BYTE* begin, BYTE* end) { return FindBytes(begin, end, rttiName, nameSize, 1); });
		if (!name)
			return nullptr;

		// NOTE: The type descriptor is 0x10 bytes before its name
		uint32_t typeDescriptorRva = (uint32_t)((name - 0x10) - Image);

		// NOTE: Complete object locator: signature 1, offset 0, then the type descriptor RVA at +0xC.
		// The vtable stores a pointer to it in the slot right before the first function.
		BYTE* pointer = ForEachSection([&](BYTE* begin, BYTE* end) -> BYTE*
		{
			for (BYTE* p = begin; (p = FindBytes(p, end, &typeDescriptorRva, 4, 4)) != nullptr; p += 4)
			{
				BYTE* col = p - 0xC;
				if (col < Image || *(uint32_t*)col != 1 || *(uint32_t*)(col + 4) != 0)
					continue;

				BYTE* found = ForEachSection([&](BYTE* b, BYTE* e) { return FindBytes(b, e, &col, sizeof(col), 8); });
				if (found)
					return found;
			}
			return nullptr;
		});

		return pointer ? (void**)(pointer + 8) : nullptr;
	}

	void Init(int32_t hotkey, bool startEnabled)
	{
		HotKey = hotkey;
		Skipping = startEnabled;
		Image = (BYTE*)GetModuleHandleA(nullptr);

		for (int i = 0; i < TaskCount; i++)
		{
			void** vtable = FindVtable(TaskNames[i]);
			if (!vtable || !IsReadable(&vtable[DispSlot], sizeof(void*)))
				continue;

			DWORD old;
			if (!VirtualProtect(&vtable[DispSlot], sizeof(void*), PAGE_READWRITE, &old))
				continue;

			Originals[i] = (DispFn)vtable[DispSlot];
			vtable[DispSlot] = Hooks[i];
			VirtualProtect(&vtable[DispSlot], sizeof(void*), old, &old);
		}
	}
}
