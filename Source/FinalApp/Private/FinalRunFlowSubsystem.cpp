#include "Subsystems/FinalRunFlowSubsystem.h"

#include "Facade/FinalRunSession.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalPlaceholderModalScreen.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"

namespace
{
FText BuildSnapshotFlowMessage(const FFinalRunSnapshot& Snapshot)
{
	if (Snapshot.PendingGrowthChoice.bHasPendingChoice)
	{
		return NSLOCTEXT("FinalRunFlow", "PendingGrowthChoice", "请选择当前角色的成长方向。");
	}

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

bool ShouldUseRewardEventFeedback(const EFinalRunEventType EventType)
{
	switch (EventType)
	{
	case EFinalRunEventType::PendingBattleRewardGenerated:
	case EFinalRunEventType::PendingBattleRewardClaimed:
	case EFinalRunEventType::PendingBattleRewardSkipped:
	case EFinalRunEventType::BattleResultApplied:
	case EFinalRunEventType::RewardNodeResolved:
	case EFinalRunEventType::EventNodeResolved:
	case EFinalRunEventType::ShopOfferPurchased:
	case EFinalRunEventType::GrowthChoiceApplied:
		return true;

	case EFinalRunEventType::Info:
	case EFinalRunEventType::RunInitialized:
	case EFinalRunEventType::BattleStartConfigured:
	case EFinalRunEventType::RunCommandAccepted:
	case EFinalRunEventType::RunCommandRejected:
	case EFinalRunEventType::NodeAdvanced:
	default:
		return false;
	}
}

FText BuildRunEventFlowMessage(const FFinalRunEvent& RunEvent)
{
	const bool bShouldUseRewardFeedback = ShouldUseRewardEventFeedback(RunEvent.EventType);
	const bool bHasRewardFeedback = bShouldUseRewardFeedback && (!RunEvent.RewardEntryViews.IsEmpty() || !RunEvent.RewardEntries.IsEmpty());
	const bool bHasAffectedCharacters = !RunEvent.AffectedCharacterResults.IsEmpty();

	if (!bHasRewardFeedback && !bHasAffectedCharacters)
	{
		return RunEvent.Message;
	}

	TArray<FString> Sections;

	if (!RunEvent.Message.IsEmpty())
	{
		Sections.Add(RunEvent.Message.ToString());
	}

	if (bHasRewardFeedback)
	{
		const FString RewardSummary = FinalRunFlowScreenUtils::BuildRewardPresentationSummaryString(
			RunEvent.RewardEntryViews,
			RunEvent.RewardEntries);
		if (!RewardSummary.IsEmpty())
		{
			Sections.Add(RewardSummary);
		}
	}

	if (bHasAffectedCharacters)
	{
		const FString CharacterResultSummary = FinalRunFlowScreenUtils::BuildCharacterResultsSummaryString(
			RunEvent.AffectedCharacterResults,
			NSLOCTEXT("FinalRunFlow", "AffectedCharacterResultsTitle", "角色结果"));
		if (!CharacterResultSummary.IsEmpty())
		{
			Sections.Add(CharacterResultSummary);
		}
	}

	if (Sections.Num() <= 0)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(FString::Join(Sections, TEXT("\n")));
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
		OnRunFlowStateChanged.Broadcast();
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
			LastFlowMessage = BuildRunEventFlowMessage(LastProcessedRunEvent);
		}

		LastSeenRunEventSequence = LatestRunEventSequence;
	}

	if (!bForce && !bHasNewEvents)
	{
		return;
	}

	CachedSnapshot = RunSession->GetSnapshot();
	const FText SnapshotFlowMessage = BuildSnapshotFlowMessage(CachedSnapshot);
	const bool bShouldPreserveRejectFeedback = bHasNewEvents && LastProcessedRunEvent.EventType == EFinalRunEventType::RunCommandRejected;
	if (CachedSnapshot.PendingGrowthChoice.bHasPendingChoice && !bShouldPreserveRejectFeedback)
	{
		LastFlowMessage = SnapshotFlowMessage;
	}
	else if (LastFlowMessage.IsEmpty())
	{
		LastFlowMessage = SnapshotFlowMessage;
	}

	if (UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr)
	{
		if (GameFlowSubsystem->TryAutoStartPreparedBattleFromRun())
		{
			return;
		}
	}

	ApplyPresentationForSnapshot(CachedSnapshot, bForce || bHasNewEvents);
	OnRunFlowStateChanged.Broadcast();
}

bool UFinalRunFlowSubsystem::ClaimPendingBattleReward()
{
	return ClaimPendingBattleRewardById(NAME_None);
}

bool UFinalRunFlowSubsystem::ClaimPendingBattleRewardById(const FName RewardId)
{
	UFinalRunSession* RunSession = ResolveRunSession();
	if (RunSession == nullptr)
	{
		LastFlowMessage = NSLOCTEXT("FinalRunFlow", "MissingRunSessionForReward", "当前无法访问 RunSession，无法领取待领奖励。");
		return false;
	}

	const bool bAccepted = RewardId.IsNone()
		? RunSession->ClaimPendingBattleReward()
		: RunSession->ClaimPendingBattleRewardById(RewardId);
	RefreshRunFlow(true);
	return bAccepted;
}

bool UFinalRunFlowSubsystem::SkipPendingBattleReward()
{
	UFinalRunSession* RunSession = ResolveRunSession();
	if (RunSession == nullptr)
	{
		LastFlowMessage = NSLOCTEXT("FinalRunFlow", "MissingRunSessionForSkipReward", "当前无法访问 RunSession，无法跳过待领奖励。");
		return false;
	}

	const bool bAccepted = RunSession->SkipPendingBattleReward();
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

bool UFinalRunFlowSubsystem::ResolveRewardNode()
{
	return SubmitRunCommand(
		EFinalRunCommandType::ResolveReward,
		NAME_None,
		NSLOCTEXT("FinalRunFlow", "MissingRunSessionForResolveReward", "当前无法访问 RunSession，无法确认奖励节点。"));
}

bool UFinalRunFlowSubsystem::ResolveEventOption(const FName OptionId)
{
	return SubmitRunCommand(
		EFinalRunCommandType::ResolveEvent,
		OptionId,
		NSLOCTEXT("FinalRunFlow", "MissingRunSessionForResolveEvent", "当前无法访问 RunSession，无法提交事件节点选项。"));
}

bool UFinalRunFlowSubsystem::ResolveShopOffer(const FName OfferId)
{
	return SubmitRunCommand(
		EFinalRunCommandType::ResolveShop,
		OfferId,
		NSLOCTEXT("FinalRunFlow", "MissingRunSessionForResolveShop", "当前无法访问 RunSession，无法提交商店节点购买请求。"));
}

bool UFinalRunFlowSubsystem::SelectGrowthChoice(const FName ChoiceInstanceId)
{
	return SubmitRunCommand(
		EFinalRunCommandType::SelectGrowthChoice,
		ChoiceInstanceId,
		NSLOCTEXT("FinalRunFlow", "MissingRunSessionForSelectGrowthChoice", "当前无法访问 RunSession，无法提交成长选择。"));
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

bool UFinalRunFlowSubsystem::SubmitRunCommand(const EFinalRunCommandType CommandType, const FName PayloadId, const FText& MissingSessionMessage)
{
	UFinalRunSession* RunSession = ResolveRunSession();
	if (RunSession == nullptr)
	{
		LastFlowMessage = MissingSessionMessage;
		return false;
	}

	FFinalCharacterId GrowthRefreshCharacterId;
	FName GrowthRefreshRunCardInstanceId = NAME_None;
	if (CommandType == EFinalRunCommandType::SelectGrowthChoice)
	{
		const FFinalRunPendingGrowthChoice& PendingGrowthChoice = RunSession->GetPendingGrowthChoice();
		if (PendingGrowthChoice.bIsValid)
		{
			if (const FFinalRunGrowthChoiceInstance* SelectedChoice = PendingGrowthChoice.Choices.FindByPredicate([&PayloadId](const FFinalRunGrowthChoiceInstance& Choice)
			{
				return Choice.ChoiceInstanceId == PayloadId;
			}))
			{
				if (SelectedChoice->ChoiceType == EFinalGrowthChoiceType::AttributeGrowth)
				{
					GrowthRefreshCharacterId = SelectedChoice->CharacterId;
				}
				else if (SelectedChoice->ChoiceType == EFinalGrowthChoiceType::CardEvolution)
				{
					GrowthRefreshRunCardInstanceId = SelectedChoice->TargetRunCardInstanceId;
				}
			}
		}
	}

	FFinalRunCommand Command;
	Command.CommandType = CommandType;
	Command.PayloadId = PayloadId;

	const bool bAccepted = RunSession->SubmitRunCommand(Command);
	RefreshRunFlow(true);
	if (bAccepted
		&& CommandType == EFinalRunCommandType::SelectGrowthChoice)
	{
		if (UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr)
		{
			if (GrowthRefreshCharacterId.IsValid())
			{
				GameFlowSubsystem->TryRefreshActiveBattleCharacterFromRunState(GrowthRefreshCharacterId);
			}

			if (!GrowthRefreshRunCardInstanceId.IsNone())
			{
				GameFlowSubsystem->TryRefreshActiveBattleCardFromRunState(GrowthRefreshRunCardInstanceId);
			}
		}
	}

	return bAccepted;
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
	case EFinalRunPresentedOverlay::GrowthChoice:
		UISubsystem->ShowRunGrowthChoiceOverlay();
		break;

	case EFinalRunPresentedOverlay::RunFlow:
		UISubsystem->ShowRunFlowOverlay();
		break;

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
	if (Snapshot.PendingGrowthChoice.bHasPendingChoice)
	{
		return EFinalRunPresentedOverlay::GrowthChoice;
	}

	if (Snapshot.Progression.FlowStage == EFinalRunFlowStage::PreparingBattle
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::None)
	{
		return EFinalRunPresentedOverlay::None;
	}

	if (Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return EFinalRunPresentedOverlay::BattleReward;
	}

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
	case EFinalRunFlowStage::PendingRewardNode:
	case EFinalRunFlowStage::RunEnded:
		return EFinalRunPresentedOverlay::RunFlow;

	case EFinalRunFlowStage::PendingEventNode:
		return EFinalRunPresentedOverlay::EventNode;

	case EFinalRunFlowStage::PendingShopNode:
		return EFinalRunPresentedOverlay::ShopNode;

	default:
		break;
	}

	if (Snapshot.Progression.bCurrentNodeNeedsResolution)
	{
		return EFinalRunPresentedOverlay::RunFlow;
	}

	if (Snapshot.Progression.bCanAdvanceToNextNode)
	{
		return EFinalRunPresentedOverlay::RunFlow;
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
