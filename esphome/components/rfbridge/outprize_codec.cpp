#include "outprize_codec.h"

// Compatibility translation unit.
//
// Older RF Bridge revisions shipped bridge-coupled Outprize method definitions
// in this file. The production codec is now header-only and pure protocol code.
// Keeping this intentionally empty source file ensures ESPHome/Git worktrees
// overwrite any stale tracked implementation during a clean rebuild.
