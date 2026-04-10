#include "Facade/FinalRunSession.h"

namespace
{
FFinalRunCharacterViewData MakeCharacterView(const FFinalRunPersistentCharacterState& CharacterState)
{
	FFinalRunCharacterViewData View;
	View.CharacterId = CharacterState.CharacterId;
	View.CurrentStress = CharacterState.CurrentStress;
	View.bCollapsed = CharacterState.bCollapsed;
	View.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
	View.CollapseCount = CharacterState.CollapseCount;
	return View;
}

FText GetBattleOutcomeText(const EFinalBattleOutcome Outcome)
{
	switch (Outcome)
	{
	case EFinalBattleOutcome::Victory:
		return FText::FromString(TEXT("Victory"));

	case EFinalBattleOutcome::Defeat:
		return FText::FromString(TEXT("Defeat"));

	case EFinalBattleOutcome::Escape:
		return FText::FromString(TEXT("Escape"));

	default:
		return FText::FromString(TEXT("Unknown"));
	}
}
}

void UFinalRunSession::InitializeRun()
{
	CurrentState = FFinalRunState{};
	CurrentState.TeamCurrentHP = 0;
	CurrentState.Gold = 0;
	CurrentState.bHasPendingBattleStart = false;
	RunLogEntries.Reset();
	ConfiguredRunNodes.Reset();
	CurrentNodeId = NAME_None;
	CurrentFlowStage = EFinalRunFlowStage::None;
	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardGold = 0;
	LastEventSequence = 0;

	FFinalRunEvent Event;
	Event.EventType = EFinalRunEventType::RunInitialized;
	Event.Message = FText::FromString(TEXT("Run session initialized."));
	AppendEvent(Event);
}

void UFinalRunSession::ConfigureRunNodeGraph(const TArray<FFinalRunNodeDefinition>& NodeDefinitions, const FName InCurrentNodeId)
{
	ConfiguredRunNodes = NodeDefinitions;
	CurrentNodeId = InCurrentNodeId;

	if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
	{
		if (CurrentNode->NodeType == EFinalRunNodeType::Battle)
		{
			if (!CurrentState.CurrentEncounterId.IsValid())
			{
				CurrentState.CurrentEncounterId = CurrentNode->EncounterId;
			}

			if (!CurrentState.CurrentRuleConfigId.IsValid())
			{
				CurrentState.CurrentRuleConfigId = CurrentNode->RuleConfigId;
			}
		}
	}

	if (CurrentState.bHasPendingBattleStart)
	{
		CurrentFlowStage = EFinalRunFlowStage::PreparingBattle;
	}
	else if (PendingRewardGold > 0)
	{
		CurrentFlowStage = EFinalRunFlowStage::PendingBattleReward;
	}
	else if (!CurrentNodeId.IsNone())
	{
		CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;
	}
}

void UFinalRunSession::ConfigureBattleStartState(const FFinalEncounterId& EncounterId, const FFinalRuleConfigId& RuleConfigId, const TArray<FFinalRunPersistentCharacterState>& PartyStates, const TArray<FFinalCardId>& DeckCardIds, int32 InTeamCurrentHP)
{
	CurrentState.CurrentEncounterId = EncounterId;
	CurrentState.CurrentRuleConfigId = RuleConfigId;
	CurrentState.Characters = PartyStates;
	CurrentState.RunDeck = DeckCardIds;
	CurrentState.TeamCurrentHP = InTeamCurrentHP;
	CurrentState.bHasPendingBattleStart = EncounterId.IsValid() && PartyStates.Num() > 0 && DeckCardIds.Num() > 0;
	CurrentFlowStage = CurrentState.bHasPendingBattleStart ? EFinalRunFlowStage::PreparingBattle : EFinalRunFlowStage::None;

	FFinalRunEvent Event;
	Event.EventType = EFinalRunEventType::BattleStartConfigured;
	Event.EncounterId = EncounterId;
	Event.RuleConfigId = RuleConfigId;
	Event.TeamCurrentHP = InTeamCurrentHP;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "BattleStartConfigured", "Configured battle start for {0} party members and {1} deck cards."),
		FText::AsNumber(PartyStates.Num()),
		FText::AsNumber(DeckCardIds.Num()));
	AppendEvent(Event);
}

bool UFinalRunSession::ClaimPendingBattleReward()
{
	FFinalRunCommand Command;
	Command.CommandType = EFinalRunCommandType::ClaimPendingBattleReward;
	return SubmitRunCommand(Command);
}

bool UFinalRunSession::AdvanceToNode(FName NodeId)
{
	FFinalRunCommand Command;
	Command.CommandType = EFinalRunCommandType::AdvanceToNode;
	Command.TargetNodeId = NodeId;
	return SubmitRunCommand(Command);
}

bool UFinalRunSession::SubmitRunCommand(const FFinalRunCommand& Command)
{
	FFinalRunEvent CommandEvent;
	CommandEvent.CommandType = Command.CommandType;
	CommandEvent.PayloadId = Command.PayloadId;
	CommandEvent.NodeId = !Command.TargetNodeId.IsNone() ? Command.TargetNodeId : Command.PayloadId;
	CommandEvent.TeamCurrentHP = CurrentState.TeamCurrentHP;

	FFinalRunEvent DetailEvent;
	EFinalRunCommandRejectReason RejectReason = EFinalRunCommandRejectReason::None;
	FText FailureMessage = FText::GetEmpty();
	bool bAccepted = false;

	switch (Command.CommandType)
	{
	case EFinalRunCommandType::AdvanceToNode:
		bAccepted = TryExecuteAdvanceToNode(CommandEvent.NodeId, DetailEvent, RejectReason, FailureMessage);
		break;

	case EFinalRunCommandType::ClaimPendingBattleReward:
		bAccepted = TryExecuteClaimPendingBattleReward(DetailEvent, RejectReason, FailureMessage);
		break;

	case EFinalRunCommandType::ResolveEvent:
	case EFinalRunCommandType::ResolveShop:
		RejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
		FailureMessage = FText::FromString(TEXT("This run command is not implemented in the current prototype."));
		break;

	default:
		RejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
		FailureMessage = FText::FromString(TEXT("Unknown run command."));
		break;
	}

	CommandEvent.EventType = bAccepted ? EFinalRunEventType::RunCommandAccepted : EFinalRunEventType::RunCommandRejected;
	CommandEvent.RejectReason = RejectReason;
	CommandEvent.Message = bAccepted
		? FText::FromString(TEXT("Run command accepted."))
		: FailureMessage;
	AppendEvent(CommandEvent);

	if (bAccepted && DetailEvent.EventType != EFinalRunEventType::Info)
	{
		DetailEvent.CommandType = Command.CommandType;
		DetailEvent.PayloadId = Command.PayloadId;
		if (DetailEvent.NodeId.IsNone())
		{
			DetailEvent.NodeId = CommandEvent.NodeId;
		}

		AppendEvent(DetailEvent);
	}

	return bAccepted;
}

bool UFinalRunSession::HasValidBattleStartState() const
{
	return CurrentState.bHasPendingBattleStart
		&& CurrentState.CurrentEncounterId.IsValid()
		&& CurrentState.CurrentRuleConfigId.IsValid()
		&& CurrentState.Characters.Num() > 0
		&& CurrentState.RunDeck.Num() > 0;
}

FFinalBattleStartRequest UFinalRunSession::BuildBattleStartRequest() const
{
	FFinalBattleStartRequest Request;
	Request.EncounterId = CurrentState.CurrentEncounterId;
	Request.RuleConfigId = CurrentState.CurrentRuleConfigId;
	Request.TeamCurrentHP = CurrentState.TeamCurrentHP;
	Request.DeckCardIds = CurrentState.RunDeck;
	Request.PartyStates = CurrentState.Characters;

	for (const FFinalRunPersistentCharacterState& CharacterState : CurrentState.Characters)
	{
		Request.PartyCharacterIds.Add(CharacterState.CharacterId);
	}

	return Request;
}

void UFinalRunSession::ApplyBattleResult(const FFinalBattleResult& Result)
{
	CurrentState.LastResolvedEncounterId = Result.EncounterId;
	CurrentState.LastBattleOutcome = Result.Outcome;
	CurrentState.LastBattleRewardGold = Result.RewardGold;
	CurrentState.TeamCurrentHP = Result.TeamCurrentHP;
	CurrentState.bHasPendingBattleStart = false;
	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardGold = 0;

	if (Result.UpdatedCharacterStates.Num() > 0)
	{
		CurrentState.Characters = Result.UpdatedCharacterStates;
	}

	if (Result.Outcome == EFinalBattleOutcome::Victory && Result.RewardGold > 0)
	{
		PendingRewardSourceNodeId = CurrentNodeId;
		PendingRewardSourceEncounterId = Result.EncounterId;
		PendingRewardBattleOutcome = Result.Outcome;
		PendingRewardGold = Result.RewardGold;
		CurrentFlowStage = EFinalRunFlowStage::PendingBattleReward;
	}
	else if (Result.Outcome == EFinalBattleOutcome::Victory)
	{
		CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;
	}
	else
	{
		CurrentFlowStage = EFinalRunFlowStage::RunEnded;
	}

	FFinalRunEvent Event;
	Event.EventType = EFinalRunEventType::BattleResultApplied;
	Event.EncounterId = Result.EncounterId;
	Event.RuleConfigId = CurrentState.CurrentRuleConfigId;
	Event.NodeId = CurrentNodeId;
	Event.BattleOutcome = Result.Outcome;
	Event.TeamCurrentHP = Result.TeamCurrentHP;
	Event.RewardGold = Result.RewardGold;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "BattleResultApplied", "Applied battle result: {0}."),
		GetBattleOutcomeText(Result.Outcome));
	AppendEvent(Event);

	if (PendingRewardGold > 0)
	{
		FFinalRunEvent RewardEvent;
		RewardEvent.EventType = EFinalRunEventType::PendingBattleRewardGenerated;
		RewardEvent.NodeId = CurrentNodeId;
		RewardEvent.SourceNodeId = CurrentNodeId;
		RewardEvent.EncounterId = Result.EncounterId;
		RewardEvent.BattleOutcome = Result.Outcome;
		RewardEvent.RewardGold = PendingRewardGold;
		RewardEvent.Message = FText::Format(
			NSLOCTEXT("FinalRunSession", "PendingBattleRewardGenerated", "Generated a pending battle reward worth {0} gold."),
			FText::AsNumber(PendingRewardGold));
		AppendEvent(RewardEvent);
	}
}

FFinalRunSnapshot UFinalRunSession::GetSnapshot() const
{
	FFinalRunSnapshot Snapshot;
	Snapshot.PendingBattle.bHasPendingBattleStart = CurrentState.bHasPendingBattleStart;
	Snapshot.PendingBattle.EncounterId = CurrentState.CurrentEncounterId;
	Snapshot.PendingBattle.RuleConfigId = CurrentState.CurrentRuleConfigId;
	Snapshot.PendingBattle.TeamCurrentHP = CurrentState.TeamCurrentHP;
	Snapshot.PendingBattle.PartyCount = CurrentState.Characters.Num();
	Snapshot.PendingBattle.DeckCount = CurrentState.RunDeck.Num();
	Snapshot.PendingBattleReward.bHasPendingReward = PendingRewardGold > 0;
	Snapshot.PendingBattleReward.SourceNodeId = PendingRewardSourceNodeId;
	Snapshot.PendingBattleReward.SourceEncounterId = PendingRewardSourceEncounterId;
	Snapshot.PendingBattleReward.SourceBattleOutcome = PendingRewardBattleOutcome;
	Snapshot.PendingBattleReward.RewardGold = PendingRewardGold;
	Snapshot.Progression.FlowStage = CurrentFlowStage;
	Snapshot.Progression.CurrentNodeId = CurrentNodeId;
	Snapshot.Progression.CurrentNodeType = GetCurrentNodeType();
	Snapshot.Progression.bCanClaimPendingBattleReward = PendingRewardGold > 0;
	Snapshot.Progression.AvailableNextNodes = BuildAvailableNextNodeViews();
	Snapshot.Progression.bCanAdvanceToNextNode =
		CurrentFlowStage != EFinalRunFlowStage::PendingBattleReward
		&& CurrentFlowStage != EFinalRunFlowStage::RunEnded
		&& Snapshot.Progression.AvailableNextNodes.Num() > 0;
	Snapshot.Gold = CurrentState.Gold;
	Snapshot.RelicCount = CurrentState.Relics.Num();
	Snapshot.DeckCount = CurrentState.RunDeck.Num();
	Snapshot.LastBattleOutcome = CurrentState.LastBattleOutcome;
	Snapshot.LastResolvedEncounterId = CurrentState.LastResolvedEncounterId;
	Snapshot.LastBattleRewardGold = CurrentState.LastBattleRewardGold;

	for (const FFinalRunPersistentCharacterState& CharacterState : CurrentState.Characters)
	{
		Snapshot.Characters.Add(MakeCharacterView(CharacterState));
	}

	return Snapshot;
}

FFinalRunState UFinalRunSession::GetRunState() const
{
	return CurrentState;
}

TArray<FFinalRunEvent> UFinalRunSession::GetRunLogEntries() const
{
	return RunLogEntries;
}

TArray<FFinalRunEvent> UFinalRunSession::GetRunEventsSince(const int32 LastSeenEventSequence) const
{
	TArray<FFinalRunEvent> Events;
	for (const FFinalRunEvent& Event : RunLogEntries)
	{
		if (Event.EventSequence > LastSeenEventSequence)
		{
			Events.Add(Event);
		}
	}

	return Events;
}

int32 UFinalRunSession::GetLatestRunEventSequence() const
{
	return LastEventSequence;
}

bool UFinalRunSession::TryExecuteClaimPendingBattleReward(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (PendingRewardGold <= 0)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPendingBattleReward;
		OutFailureMessage = FText::FromString(TEXT("There is no pending battle reward to claim."));
		return false;
	}

	const int32 ClaimedRewardGold = PendingRewardGold;
	CurrentState.Gold += ClaimedRewardGold;
	CurrentState.LastBattleRewardGold = ClaimedRewardGold;
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::PendingBattleRewardClaimed;
	OutDetailEvent.NodeId = PendingRewardSourceNodeId;
	OutDetailEvent.SourceNodeId = PendingRewardSourceNodeId;
	OutDetailEvent.EncounterId = PendingRewardSourceEncounterId;
	OutDetailEvent.BattleOutcome = PendingRewardBattleOutcome;
	OutDetailEvent.RewardGold = ClaimedRewardGold;
	OutDetailEvent.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "PendingBattleRewardClaimed", "Claimed the pending battle reward for {0} gold."),
		FText::AsNumber(ClaimedRewardGold));

	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardGold = 0;
	return true;
}

bool UFinalRunSession::TryExecuteAdvanceToNode(const FName& TargetNodeId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (CurrentFlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		OutRejectReason = EFinalRunCommandRejectReason::PendingBattleRewardMustBeClaimed;
		OutFailureMessage = FText::FromString(TEXT("Claim the pending battle reward before advancing to another node."));
		return false;
	}

	if (CurrentFlowStage == EFinalRunFlowStage::RunEnded)
	{
		OutRejectReason = EFinalRunCommandRejectReason::RunEnded;
		OutFailureMessage = FText::FromString(TEXT("The run can no longer advance."));
		return false;
	}

	if (TargetNodeId.IsNone())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingTargetNode;
		OutFailureMessage = FText::FromString(TEXT("AdvanceToNode requires a target node id."));
		return false;
	}

	const FFinalRunNodeDefinition* TargetNode = FindNodeDefinition(TargetNodeId);
	if (TargetNode == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownTargetNode;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "UnknownTargetNode", "Run node {0} is not defined."),
			FText::FromName(TargetNodeId));
		return false;
	}

	if (!CurrentNodeId.IsNone())
	{
		if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
		{
			if (!CurrentNode->NextNodeIds.Contains(TargetNodeId))
			{
				OutRejectReason = EFinalRunCommandRejectReason::TargetNodeNotReachable;
				OutFailureMessage = FText::Format(
					NSLOCTEXT("FinalRunSession", "TargetNodeNotReachable", "Run node {0} is not reachable from the current node."),
					FText::FromName(TargetNodeId));
				return false;
			}
		}
	}

	if (TargetNode->NodeType != EFinalRunNodeType::Battle)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedTargetNodeType;
		OutFailureMessage = FText::FromString(TEXT("Only battle nodes are supported by the current run progression prototype."));
		return false;
	}

	if (!TargetNode->EncounterId.IsValid() || !TargetNode->RuleConfigId.IsValid())
	{
		OutRejectReason = EFinalRunCommandRejectReason::TargetNodeMissingBattleConfig;
		OutFailureMessage = FText::FromString(TEXT("The target battle node is missing encounter or rule config data."));
		return false;
	}

	CurrentNodeId = TargetNodeId;
	ApplyBattleStartContextFromNode(*TargetNode);
	CurrentFlowStage = CurrentState.bHasPendingBattleStart ? EFinalRunFlowStage::PreparingBattle : EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::NodeAdvanced;
	OutDetailEvent.NodeId = TargetNodeId;
	OutDetailEvent.EncounterId = TargetNode->EncounterId;
	OutDetailEvent.RuleConfigId = TargetNode->RuleConfigId;
	OutDetailEvent.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "NodeAdvanced", "Advanced to run node {0}."),
		FText::FromName(TargetNodeId));
	return true;
}

const FFinalRunNodeDefinition* UFinalRunSession::FindNodeDefinition(const FName& NodeId) const
{
	return ConfiguredRunNodes.FindByPredicate([&NodeId](const FFinalRunNodeDefinition& Definition)
	{
		return Definition.NodeId == NodeId;
	});
}

TArray<FFinalRunNodeOptionViewData> UFinalRunSession::BuildAvailableNextNodeViews() const
{
	TArray<FFinalRunNodeOptionViewData> Views;

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr)
	{
		return Views;
	}

	for (const FName& NextNodeId : CurrentNode->NextNodeIds)
	{
		FFinalRunNodeOptionViewData View;
		View.NodeId = NextNodeId;

		if (const FFinalRunNodeDefinition* NextNode = FindNodeDefinition(NextNodeId))
		{
			View.NodeType = NextNode->NodeType;
			View.EncounterId = NextNode->EncounterId;
			View.RuleConfigId = NextNode->RuleConfigId;
		}

		Views.Add(View);
	}

	return Views;
}

void UFinalRunSession::ApplyBattleStartContextFromNode(const FFinalRunNodeDefinition& NodeDefinition)
{
	CurrentState.CurrentEncounterId = NodeDefinition.EncounterId;
	CurrentState.CurrentRuleConfigId = NodeDefinition.RuleConfigId;
	CurrentState.bHasPendingBattleStart =
		NodeDefinition.EncounterId.IsValid()
		&& NodeDefinition.RuleConfigId.IsValid()
		&& CurrentState.Characters.Num() > 0
		&& CurrentState.RunDeck.Num() > 0;
}

EFinalRunNodeType UFinalRunSession::GetCurrentNodeType() const
{
	if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
	{
		return CurrentNode->NodeType;
	}

	return EFinalRunNodeType::None;
}

void UFinalRunSession::AppendEvent(const FFinalRunEvent& Event)
{
	FFinalRunEvent EventToAppend = Event;
	EventToAppend.EventSequence = ++LastEventSequence;
	if (!EventToAppend.EncounterId.IsValid())
	{
		EventToAppend.EncounterId = CurrentState.CurrentEncounterId;
	}

	if (!EventToAppend.RuleConfigId.IsValid())
	{
		EventToAppend.RuleConfigId = CurrentState.CurrentRuleConfigId;
	}

	if (EventToAppend.TeamCurrentHP <= 0 && CurrentState.TeamCurrentHP > 0)
	{
		EventToAppend.TeamCurrentHP = CurrentState.TeamCurrentHP;
	}

	RunLogEntries.Add(MoveTemp(EventToAppend));
}
