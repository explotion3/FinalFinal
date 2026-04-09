#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleHUDViewModel::ApplySnapshot(const FFinalBattleSnapshot& InSnapshot)
{
	Snapshot = InSnapshot;
}

FFinalBattleSnapshot UFinalBattleHUDViewModel::GetSnapshot() const
{
	return Snapshot;
}
