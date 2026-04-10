#include "Subsystems/FinalRunFlowSubsystem.h"

#include "Facade/FinalRunSession.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalPlaceholderModalScreen.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"

namespace
{
FText BuildSnapshotFlowMessage(const FFinalRunSnapshot& Snapshot)
{
	if (!Snapshot.Progression.CurrentNodeStateMessage.IsEmpty()
		&& (Snapshot.PendingBattleReward.bHasPendingReward
			|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::AwaitingNodeAdvance
			|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingRewardNode
			|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingEventNode
			|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingShopNode
			|| Snapshot.Progression.bCurrentNodeNeedsResolution))
	{
		return Snapshot.Progression.CurrentNodeStateMessage;
	}

	if (Snapshot.PendingBattleReward.bHasPendingReward && !Snapshot.PendingBattleReward.bCanClaim)
	{
		return NSLOCTEXT("FinalRunFlow", "PendingRewardNotClaimable", "当前待领奖励已生成，但尚未满足领取条件。");
	}

	return FText::GetEmpty();
}
}

void UFinalRunFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetFlowState();
}

void UFinalRunFlowSubsystem::Deinitialize()
{
	ResetFlowState();
	Super::Deinitialize();
}

void UFinalRunFlowSubsystem::HandleRunSessionChanged()
{
	ResetFlowState();
	RefreshRunFlow(true);
}

void UFinalRunFlowSubsystem::RefreshRunFlow(const bool bForce)
{
	UFinalRunSession* RunSession = ResolveRunSession();
	if (RunSession == nullptr)
	{
		ResetFlowState();
		CloseActiveFlowModal();
		CloseActiveFlowOverlay();
		return;
	}

	const int32 LatestRunEventSequence = RunSession->GetLatestRunEventSequence();
	const bool bHasNewEvents = LatestRunEventSequence > LastSeenRunEventSequence;
	if (bHasNewEvents)
	{
		const TArray<FFinalRunEvent> NewRunEvents = RunSession->GetRunEventsSince(LastSeenRunEventSequence);
		if (NewRunEvents.Num() > 0)
		{
			LastProcessedRunEvent = NewRunEvents.Last();
			LastFlowMessage = LastProcessedRunEvent.Message;
		}

		LastSeenRunEventSequence = LatestRunEventSequence;
	}

	if (!bForce && !bHasNewEvents)
	{
		return;
	}

	CachedSnapshot = RunSession->GetSnapshot();
	if (LastFlowMessage.IsEmpty())
	{
		LastFlowMessage = BuildSnapshotFlowMessage(CachedSnapshot);
	}
	ApplyPresentationForSnapshot(CachedSnapshot, bForce || bHasNewEvents);
}

bool UFinalRunFlowSubsystem::ClaimPendingBattleReward()
{
	UFinalRunSession* RunSession = ResolveRunSession();
	if (RunSession == nullptr)
	{
		LastFlowMessage = NSLOCTEXT("FinalRunFlow", "MissingRunSessionForReward", "当前无法访问 RunSession，无法领取待领奖励。");
		return false;
	}

	const bool bAccepted = RunSession->ClaimPendingBattleReward();
	RefreshRunFlow(true);
	return bAccepted;
}

bool UFinalRunFlowSubsystem::AdvanceToNode(const FName NodeId)
{
	UFinalRunSession* RunSession = ResolveRunSession();
	if (RunSession == nullptr)
	{
		LastFlowMessage = NSLOCTEXT("FinalRunFlow", "MissingRunSessionForAdvance", "当前无法访问 RunSession，无法推进节点。");
		return false;
	}

	const bool bAccepted = RunSession->AdvanceToNode(NodeId);
	RefreshRunFlow(true);
	return bAccepted;
}

FFinalRunSnapshot UFinalRunFlowSubsystem::GetCurrentRunSnapshot() const
{
	if (const UFinalRunSession* RunSession = ResolveRunSession())
	{
		return RunSession->GetSnapshot();
	}

	return CachedSnapshot;
}

FFinalRunEvent UFinalRunFlowSubsystem::GetLastProcessedRunEvent() const
{
	return LastProcessedRunEvent;
}

FText UFinalRunFlowSubsystem::GetLastFlowMessage() const
{
	return LastFlowMessage;
}

void UFinalRunFlowSubsystem::ResetFlowState()
{
	CachedSnapshot = FFinalRunSnapshot{};
	LastProcessedRunEvent = FFinalRunEvent{};
	LastFlowMessage = FText::GetEmpty();
	LastSeenRunEventSequence = 0;
	PresentedOverlay = EFinalRunPresentedOverlay::None;
}

void UFinalRunFlowSubsystem::ApplyPresentationForSnapshot(const FFinalRunSnapshot& Snapshot, const bool bForce)
{
	UFinalUISubsystem* UISubsystem = ResolveUISubsystem();
	if (UISubsystem == nullptr)
	{
		return;
	}

	const EFinalRunPresentedOverlay DesiredOverlay = DetermineDesiredOverlay(Snapshot);
	if (DesiredOverlay != PresentedOverlay)
	{
		CloseActiveFlowModal();
	}

	switch (DesiredOverlay)
	{
	case EFinalRunPresentedOverlay::BattleReward:
		UISubsystem->ShowBattleRewardOverlayPlaceholder();
		break;

	case EFinalRunPresentedOverlay::NodeSelect:
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
		break;

	case EFinalRunPresentedOverlay::RewardNode:
		UISubsystem->ShowRewardNodeOverlayPlaceholder();
		break;

	case EFinalRunPresentedOverlay::EventNode:
		UISubsystem->ShowEventNodeOverlayPlaceholder();
		break;

	case EFinalRunPresentedOverlay::ShopNode:
		UISubsystem->ShowShopNodeOverlayPlaceholder();
		break;

	case EFinalRunPresentedOverlay::None:
	default:
		CloseActiveFlowModal();
		CloseActiveFlowOverlay();
		break;
	}

	if (DesiredOverlay == EFinalRunPresentedOverlay::None && !bForce)
	{
		CloseActiveFlowModal();
	}

	PresentedOverlay = DesiredOverlay;
}

void UFinalRunFlowSubsystem::CloseActiveFlowModal() const
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		if (UFinalScreenBase* ActiveModalScreen = UISubsystem->GetActiveModalScreen())
		{
			if (Cast<UFinalPlaceholderModalScreen>(ActiveModalScreen) != nullptr)
			{
				UISubsystem->CloseModalScreen(ActiveModalScreen);
			}
		}
	}
}

void UFinalRunFlowSubsystem::CloseActiveFlowOverlay() const
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		if (UFinalScreenBase* ActiveOverlayScreen = UISubsystem->GetActiveOverlayScreen())
		{
			if (Cast<UFinalRunStageOverlayScreenBase>(ActiveOverlayScreen) != nullptr)
			{
				UISubsystem->CloseOverlayScreen(ActiveOverlayScreen);
			}
		}
	}
}

EFinalRunPresentedOverlay UFinalRunFlowSubsystem::DetermineDesiredOverlay(const FFinalRunSnapshot& Snapshot) const
{
	if (Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return EFinalRunPresentedOverlay::BattleReward;
	}

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return EFinalRunPresentedOverlay::NodeSelect;

	case EFinalRunFlowStage::PendingRewardNode:
		return EFinalRunPresentedOverlay::RewardNode;

	case EFinalRunFlowStage::PendingEventNode:
		return EFinalRunPresentedOverlay::EventNode;

	case EFinalRunFlowStage::PendingShopNode:
		return EFinalRunPresentedOverlay::ShopNode;

	default:
		break;
	}

	if (Snapshot.Progression.bCurrentNodeNeedsResolution)
	{
		switch (Snapshot.Progression.CurrentNodeType)
		{
		case EFinalRunNodeType::Reward:
			return EFinalRunPresentedOverlay::RewardNode;

		case EFinalRunNodeType::Event:
			return EFinalRunPresentedOverlay::EventNode;

		case EFinalRunNodeType::Shop:
			return EFinalRunPresentedOverlay::ShopNode;

		default:
			return EFinalRunPresentedOverlay::NodeSelect;
		}
	}

	if (Snapshot.Progression.bCanAdvanceToNextNode)
	{
		return EFinalRunPresentedOverlay::NodeSelect;
	}

	return EFinalRunPresentedOverlay::None;
}

UFinalRunSession* UFinalRunFlowSubsystem::ResolveRunSession() const
{
	const UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	return GameFlowSubsystem ? GameFlowSubsystem->GetRunSession() : nullptr;
}

UFinalUISubsystem* UFinalRunFlowSubsystem::ResolveUISubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalUISubsystem>() : nullptr;
}
