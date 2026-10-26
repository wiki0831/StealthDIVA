#pragma once
#include <stdint.h>

// NOTE: Skips the draw of heavy PV tasks (stage, movie, characters, stage objects) to save GPU power.
// Task vtables are found at runtime from their RTTI names, so no per-version addresses are needed.
namespace scene_skip
{
	void Init(int32_t hotkey, bool startEnabled);

	// NOTE: Call every frame, toggles on a key press (not while held)
	void Poll();

	bool IsSkipping();

	// NOTE: Call with true when the caller runs Poll() once per frame, so the task hooks skip it
	void SetExternalPoll(bool external);

	// NOTE: While forced, the scene is always skipped and the hotkey is ignored
	void SetForced(bool forced);
}
