#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleHUDViewModel::ApplySnapshot(const FFinalBattleSnapshot& InSnapshot)
{
	Snapshot = InSnapshot;
}

void UFinalBattleHUDViewModel::ApplyBattleEvent(const FFinalBattleEvent& BattleEvent)
{
	if (BattleEvent.EventType != EFinalBattleEventType::PhaseChanged)
	{
		return;
	}

	LatestPhaseChangedEvent = BattleEvent;
	OnPhaseChangedPresentation.Broadcast(BattleEvent);
}

FFinalBattleSnapshot UFinalBattleHUDViewModel::GetSnapshot() const
{
	return Snapshot;
}

FFinalBattleEvent UFinalBattleHUDViewModel::GetLatestPhaseChangedEvent() const
{
	return LatestPhaseChangedEvent;
}

void UFinalBattleHUDViewModel::ApplyPresentation(const FFinalBattleHUDPresentationData& InPresentation)
{
	Presentation = InPresentation;
	BroadcastViewModelChanged();
}

FFinalBattleHUDPresentationData UFinalBattleHUDViewModel::GetPresentation() const
{
	return Presentation;
}
