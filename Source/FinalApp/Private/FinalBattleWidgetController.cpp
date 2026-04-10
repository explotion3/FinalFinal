#include "Controllers/FinalBattleWidgetController.h"

#include "Facade/FinalBattleSession.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleWidgetController::Initialize(UFinalBattleHUDViewModel* InViewModel)
{
	ViewModel = InViewModel;
}

void UFinalBattleWidgetController::BindToBattleFlow(UFinalBattleFlowSubsystem* InBattleFlowSubsystem)
{
	if (BattleFlowSubsystem == InBattleFlowSubsystem)
	{
		return;
	}

	UnbindFromBattleFlow();
	BattleFlowSubsystem = InBattleFlowSubsystem;

	if (BattleFlowSubsystem == nullptr)
	{
		return;
	}

	BattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &UFinalBattleWidgetController::HandleBattleSnapshotChanged);
	BattleFlowSubsystem->OnBattleEventBroadcast.AddDynamic(this, &UFinalBattleWidgetController::HandleBattleEventBroadcast);

	HandleBattleSnapshotChanged(BattleFlowSubsystem->GetCurrentSnapshot());
	for (const FFinalBattleEvent& BattleEvent : BattleFlowSubsystem->GetBattleLogEntries())
	{
		HandleBattleEventBroadcast(BattleEvent);
	}
}

void UFinalBattleWidgetController::UnbindFromBattleFlow()
{
	if (BattleFlowSubsystem)
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &UFinalBattleWidgetController::HandleBattleSnapshotChanged);
		BattleFlowSubsystem->OnBattleEventBroadcast.RemoveDynamic(this, &UFinalBattleWidgetController::HandleBattleEventBroadcast);
	}

	BattleFlowSubsystem = nullptr;
}

void UFinalBattleWidgetController::RefreshFromSession(UFinalBattleSession* Session)
{
	if (!ViewModel || !Session)
	{
		return;
	}

	ViewModel->ApplySnapshot(Session->GetSnapshot());
	for (const FFinalBattleEvent& BattleEvent : Session->GetBattleLogEntries())
	{
		ViewModel->ApplyBattleEvent(BattleEvent);
	}
}

UFinalBattleHUDViewModel* UFinalBattleWidgetController::GetViewModel() const
{
	return ViewModel;
}

void UFinalBattleWidgetController::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	if (ViewModel == nullptr)
	{
		return;
	}

	ViewModel->ApplySnapshot(Snapshot);
}

void UFinalBattleWidgetController::HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent)
{
	if (ViewModel == nullptr)
	{
		return;
	}

	ViewModel->ApplyBattleEvent(BattleEvent);
}
