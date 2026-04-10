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

bool IsBattleNodeType(const EFinalRunNodeType NodeType)
{
	return NodeType == EFinalRunNodeType::Battle
		|| NodeType == EFinalRunNodeType::EliteBattle
		|| NodeType == EFinalRunNodeType::BossBattle;
}

bool HasImplementedNodeResolver(const EFinalRunNodeType NodeType)
{
	return IsBattleNodeType(NodeType);
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

FText GetDefaultNodeDisplayName(const EFinalRunNodeType NodeType)
{
	switch (NodeType)
	{
	case EFinalRunNodeType::Battle:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayBattle", "Battle");

	case EFinalRunNodeType::EliteBattle:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayEliteBattle", "Elite Battle");

	case EFinalRunNodeType::BossBattle:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayBossBattle", "Boss Battle");

	case EFinalRunNodeType::Event:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayEvent", "Event");

	case EFinalRunNodeType::Shop:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayShop", "Shop");

	case EFinalRunNodeType::Reward:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayReward", "Reward");

	default:
		return NSLOCTEXT("FinalRunSession", "NodeDisplayUnknown", "Unknown Node");
	}
}

FName GetDefaultNodeDisplayLabel(const EFinalRunNodeType NodeType)
{
	switch (NodeType)
	{
	case EFinalRunNodeType::Battle:
		return TEXT("RunNode.Battle");

	case EFinalRunNodeType::EliteBattle:
		return TEXT("RunNode.EliteBattle");

	case EFinalRunNodeType::BossBattle:
		return TEXT("RunNode.BossBattle");

	case EFinalRunNodeType::Event:
		return TEXT("RunNode.Event");

	case EFinalRunNodeType::Shop:
		return TEXT("RunNode.Shop");

	case EFinalRunNodeType::Reward:
		return TEXT("RunNode.Reward");

	default:
		return TEXT("RunNode.Unknown");
	}
}

bool DoesFlowStageBlockNodeAdvance(const EFinalRunFlowStage FlowStage)
{
	return FlowStage == EFinalRunFlowStage::PreparingBattle
		|| FlowStage == EFinalRunFlowStage::PendingBattleReward
		|| FlowStage == EFinalRunFlowStage::PendingRewardNode
		|| FlowStage == EFinalRunFlowStage::PendingEventNode
		|| FlowStage == EFinalRunFlowStage::PendingShopNode;
}

int32 GetRewardGoldTotal(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	int32 GoldTotal = 0;
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (Entry.RewardType == EFinalRunRewardType::Gold)
		{
			GoldTotal += Entry.Value;
		}
	}

	return GoldTotal;
}

TArray<FFinalRunRewardEntry> MakeClaimedRewardEntries(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	TArray<FFinalRunRewardEntry> ClaimedEntries = RewardEntries;
	for (FFinalRunRewardEntry& Entry : ClaimedEntries)
	{
		Entry.bCanClaim = false;
		Entry.bClaimed = true;
	}

	return ClaimedEntries;
}

TArray<FFinalRunRewardEntry> BuildBattleRewardEntries(const FFinalBattleResult& Result)
{
	TArray<FFinalRunRewardEntry> RewardEntries;

	if (Result.Outcome == EFinalBattleOutcome::Victory && Result.RewardGold > 0)
	{
		FFinalRunRewardEntry GoldEntry;
		GoldEntry.RewardId = TEXT("BattleReward.Gold");
		GoldEntry.RewardType = EFinalRunRewardType::Gold;
		GoldEntry.Value = Result.RewardGold;
		GoldEntry.DisplayId = TEXT("Currency.Gold");
		GoldEntry.DisplayName = NSLOCTEXT("FinalRunSession", "RewardDisplayGold", "Gold");
		GoldEntry.bCanClaim = true;
		GoldEntry.bClaimed = false;
		RewardEntries.Add(GoldEntry);
	}

	return RewardEntries;
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
	VisitedNodeIds.Reset();
	CurrentNodeId = NAME_None;
	CurrentFlowStage = EFinalRunFlowStage::None;
	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardEntries.Reset();
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

	if (!CurrentNodeId.IsNone())
	{
		VisitedNodeIds.Add(CurrentNodeId);
	}

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode != nullptr)
	{
		if (CurrentNode->IsBattleNode() && !HasPendingBattleReward())
		{
			ApplyNodeContextFromNode(*CurrentNode);
		}
		else if (!CurrentNode->IsBattleNode())
		{
			ClearBattleStartContext();
		}
	}

	if (CurrentState.bHasPendingBattleStart)
	{
		CurrentFlowStage = EFinalRunFlowStage::PreparingBattle;
	}
	else if (HasPendingBattleReward())
	{
		CurrentFlowStage = EFinalRunFlowStage::PendingBattleReward;
	}
	else if (CurrentNode != nullptr)
	{
		if (IsBattleNodeType(CurrentNode->NodeType))
		{
			CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;
		}
		else
		{
			switch (CurrentNode->NodeType)
			{
			case EFinalRunNodeType::Reward:
				CurrentFlowStage = EFinalRunFlowStage::PendingRewardNode;
				break;

			case EFinalRunNodeType::Event:
				CurrentFlowStage = EFinalRunFlowStage::PendingEventNode;
				break;

			case EFinalRunNodeType::Shop:
				CurrentFlowStage = EFinalRunFlowStage::PendingShopNode;
				break;

			default:
				CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;
				break;
			}
		}
	}
	else
	{
		CurrentFlowStage = EFinalRunFlowStage::None;
	}
}

void UFinalRunSession::ConfigureBattleStartState(const FFinalEncounterId& EncounterId, const FFinalRuleConfigId& RuleConfigId, const TArray<FFinalRunPersistentCharacterState>& PartyStates, const TArray<FFinalCardId>& DeckCardIds, int32 InTeamCurrentHP)
{
	CurrentState.CurrentEncounterId = EncounterId;
	CurrentState.CurrentRuleConfigId = RuleConfigId;
	CurrentState.Characters = PartyStates;
	CurrentState.RunDeck = DeckCardIds;
	CurrentState.TeamCurrentHP = InTeamCurrentHP;
	CurrentState.bHasPendingBattleStart = EncounterId.IsValid()
		&& RuleConfigId.IsValid()
		&& PartyStates.Num() > 0
		&& DeckCardIds.Num() > 0;
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
		if (CurrentFlowStage == EFinalRunFlowStage::PendingEventNode)
		{
			RejectReason = EFinalRunCommandRejectReason::EventNodeResolutionNotImplemented;
			FailureMessage = FText::FromString(TEXT("Event node resolution is not implemented in the current prototype."));
		}
		else
		{
			RejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
			FailureMessage = FText::FromString(TEXT("ResolveEvent is only valid while the run is on an event node."));
		}
		break;

	case EFinalRunCommandType::ResolveShop:
		if (CurrentFlowStage == EFinalRunFlowStage::PendingShopNode)
		{
			RejectReason = EFinalRunCommandRejectReason::ShopNodeResolutionNotImplemented;
			FailureMessage = FText::FromString(TEXT("Shop node resolution is not implemented in the current prototype."));
		}
		else
		{
			RejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
			FailureMessage = FText::FromString(TEXT("ResolveShop is only valid while the run is on a shop node."));
		}
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
	const FFinalRuleConfigId ResolvedRuleConfigId = CurrentState.CurrentRuleConfigId;
	CurrentState.LastResolvedEncounterId = Result.EncounterId;
	CurrentState.LastBattleOutcome = Result.Outcome;
	CurrentState.LastBattleRewardGold = Result.RewardGold;
	CurrentState.TeamCurrentHP = Result.TeamCurrentHP;
	ClearBattleStartContext();
	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardEntries.Reset();

	if (Result.UpdatedCharacterStates.Num() > 0)
	{
		CurrentState.Characters = Result.UpdatedCharacterStates;
	}

	PendingRewardEntries = BuildBattleRewardEntries(Result);

	if (PendingRewardEntries.Num() > 0)
	{
		PendingRewardSourceNodeId = CurrentNodeId;
		PendingRewardSourceEncounterId = Result.EncounterId;
		PendingRewardBattleOutcome = Result.Outcome;
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
	Event.RuleConfigId = ResolvedRuleConfigId;
	Event.NodeId = CurrentNodeId;
	Event.BattleOutcome = Result.Outcome;
	Event.TeamCurrentHP = Result.TeamCurrentHP;
	Event.RewardGold = GetRewardGoldTotal(PendingRewardEntries);
	Event.RewardEntries = PendingRewardEntries;
	if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
	{
		PopulateNodeEventMetadata(Event, *CurrentNode);
	}
	Event.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "BattleResultApplied", "Applied battle result: {0}."),
		GetBattleOutcomeText(Result.Outcome));
	AppendEvent(Event);

	if (HasPendingBattleReward())
	{
		FFinalRunEvent RewardEvent;
		RewardEvent.EventType = EFinalRunEventType::PendingBattleRewardGenerated;
		RewardEvent.NodeId = CurrentNodeId;
		RewardEvent.SourceNodeId = CurrentNodeId;
		RewardEvent.EncounterId = Result.EncounterId;
		RewardEvent.BattleOutcome = Result.Outcome;
		RewardEvent.RewardGold = GetPendingBattleRewardGold();
		RewardEvent.RewardEntries = PendingRewardEntries;
		if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
		{
			PopulateNodeEventMetadata(RewardEvent, *CurrentNode);
		}
		RewardEvent.Message = FText::Format(
			NSLOCTEXT("FinalRunSession", "PendingBattleRewardGenerated", "Generated {0} pending battle reward entries."),
			FText::AsNumber(PendingRewardEntries.Num()));
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

	Snapshot.PendingBattleReward.bHasPendingReward = HasPendingBattleReward();
	Snapshot.PendingBattleReward.SourceNodeId = PendingRewardSourceNodeId;
	Snapshot.PendingBattleReward.SourceEncounterId = PendingRewardSourceEncounterId;
	Snapshot.PendingBattleReward.SourceBattleOutcome = PendingRewardBattleOutcome;
	Snapshot.PendingBattleReward.RewardGold = GetPendingBattleRewardGold();
	Snapshot.PendingBattleReward.bCanClaim = HasPendingBattleReward();
	Snapshot.PendingBattleReward.RewardEntries = PendingRewardEntries;

	if (const FFinalRunNodeDefinition* SourceNode = FindNodeDefinition(PendingRewardSourceNodeId))
	{
		Snapshot.PendingBattleReward.SourceNodeType = SourceNode->NodeType;
		Snapshot.PendingBattleReward.SourceNodeDisplayName = SourceNode->DisplayName.IsEmpty()
			? GetDefaultNodeDisplayName(SourceNode->NodeType)
			: SourceNode->DisplayName;
		Snapshot.PendingBattleReward.SourceNodeDisplayLabel = SourceNode->DisplayLabel.IsNone()
			? GetDefaultNodeDisplayLabel(SourceNode->NodeType)
			: SourceNode->DisplayLabel;
	}

	Snapshot.Progression.FlowStage = CurrentFlowStage;
	Snapshot.Progression.CurrentNodeId = CurrentNodeId;
	Snapshot.Progression.CurrentNodeType = GetCurrentNodeType();
	Snapshot.Progression.bCurrentNodeVisited = !CurrentNodeId.IsNone() && VisitedNodeIds.Contains(CurrentNodeId);
	Snapshot.Progression.bCurrentNodeNeedsResolution =
		CurrentFlowStage != EFinalRunFlowStage::None
		&& CurrentFlowStage != EFinalRunFlowStage::AwaitingNodeAdvance
		&& CurrentFlowStage != EFinalRunFlowStage::RunEnded;
	Snapshot.Progression.bCurrentNodeHasImplementedResolver = HasImplementedNodeResolver(Snapshot.Progression.CurrentNodeType);
	Snapshot.Progression.CurrentNodeStateMessage = GetCurrentNodeStateMessage();
	Snapshot.Progression.bCanClaimPendingBattleReward = HasPendingBattleReward();
	Snapshot.Progression.AvailableNextNodes = BuildAvailableNextNodeViews();

	if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
	{
		Snapshot.Progression.CurrentNodeDisplayName = CurrentNode->DisplayName.IsEmpty()
			? GetDefaultNodeDisplayName(CurrentNode->NodeType)
			: CurrentNode->DisplayName;
		Snapshot.Progression.CurrentNodeDisplayLabel = CurrentNode->DisplayLabel.IsNone()
			? GetDefaultNodeDisplayLabel(CurrentNode->NodeType)
			: CurrentNode->DisplayLabel;
		Snapshot.Progression.CurrentChapter = CurrentNode->ChapterIndex;
		Snapshot.Progression.CurrentFloor = CurrentNode->FloorIndex;
	}

	int32 UnlockedNextNodeCount = 0;
	for (const FFinalRunNodeOptionViewData& NextNode : Snapshot.Progression.AvailableNextNodes)
	{
		if (!NextNode.bLocked)
		{
			++UnlockedNextNodeCount;
		}
	}

	Snapshot.Progression.bCanAdvanceToNextNode =
		CurrentFlowStage == EFinalRunFlowStage::AwaitingNodeAdvance
		&& UnlockedNextNodeCount > 0;

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
	if (!HasPendingBattleReward())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPendingBattleReward;
		OutFailureMessage = FText::FromString(TEXT("There is no pending battle reward to claim."));
		return false;
	}

	TArray<FFinalRunRewardEntry> ClaimedEntries = MakeClaimedRewardEntries(PendingRewardEntries);
	for (const FFinalRunRewardEntry& Entry : PendingRewardEntries)
	{
		if (!Entry.IsClaimable())
		{
			continue;
		}

		if (Entry.RewardType == EFinalRunRewardType::Gold)
		{
			CurrentState.Gold += Entry.Value;
		}
	}

	CurrentState.LastBattleRewardGold = GetRewardGoldTotal(PendingRewardEntries);
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::PendingBattleRewardClaimed;
	OutDetailEvent.NodeId = PendingRewardSourceNodeId;
	OutDetailEvent.SourceNodeId = PendingRewardSourceNodeId;
	OutDetailEvent.EncounterId = PendingRewardSourceEncounterId;
	OutDetailEvent.BattleOutcome = PendingRewardBattleOutcome;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ClaimedEntries);
	OutDetailEvent.RewardEntries = ClaimedEntries;
	if (const FFinalRunNodeDefinition* SourceNode = FindNodeDefinition(PendingRewardSourceNodeId))
	{
		PopulateNodeEventMetadata(OutDetailEvent, *SourceNode);
	}
	OutDetailEvent.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "PendingBattleRewardClaimed", "Claimed {0} pending battle reward entries."),
		FText::AsNumber(ClaimedEntries.Num()));

	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardEntries.Reset();
	return true;
}

bool UFinalRunSession::TryExecuteAdvanceToNode(const FName& TargetNodeId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (HasPendingBattleReward())
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

	if (DoesFlowStageBlockNodeAdvance(CurrentFlowStage))
	{
		OutRejectReason = EFinalRunCommandRejectReason::CurrentNodeRequiresResolution;
		OutFailureMessage = GetCurrentNodeStateMessage();
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

	if (TargetNode->bStartsLocked)
	{
		OutRejectReason = EFinalRunCommandRejectReason::TargetNodeLocked;
		OutFailureMessage = TargetNode->LockedReason.IsEmpty()
			? FText::FromString(TEXT("The target run node is locked."))
			: TargetNode->LockedReason;
		return false;
	}

	if (IsBattleNodeType(TargetNode->NodeType) && (!TargetNode->EncounterId.IsValid() || !TargetNode->RuleConfigId.IsValid()))
	{
		OutRejectReason = EFinalRunCommandRejectReason::TargetNodeMissingBattleConfig;
		OutFailureMessage = FText::FromString(TEXT("The target battle node is missing encounter or rule config data."));
		return false;
	}

	if (TargetNode->NodeType == EFinalRunNodeType::None)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedTargetNodeType;
		OutFailureMessage = FText::FromString(TEXT("The target run node does not have a supported node type."));
		return false;
	}

	const FName PreviousNodeId = CurrentNodeId;
	CurrentNodeId = TargetNodeId;
	VisitedNodeIds.Add(TargetNodeId);
	ApplyNodeContextFromNode(*TargetNode);

	if (IsBattleNodeType(TargetNode->NodeType))
	{
		CurrentFlowStage = CurrentState.bHasPendingBattleStart ? EFinalRunFlowStage::PreparingBattle : EFinalRunFlowStage::AwaitingNodeAdvance;
	}
	else
	{
		switch (TargetNode->NodeType)
		{
		case EFinalRunNodeType::Reward:
			CurrentFlowStage = EFinalRunFlowStage::PendingRewardNode;
			break;

		case EFinalRunNodeType::Event:
			CurrentFlowStage = EFinalRunFlowStage::PendingEventNode;
			break;

		case EFinalRunNodeType::Shop:
			CurrentFlowStage = EFinalRunFlowStage::PendingShopNode;
			break;

		default:
			CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;
			break;
		}
	}

	OutDetailEvent.EventType = EFinalRunEventType::NodeAdvanced;
	OutDetailEvent.NodeId = TargetNodeId;
	OutDetailEvent.SourceNodeId = PreviousNodeId;
	PopulateNodeEventMetadata(OutDetailEvent, *TargetNode);
	OutDetailEvent.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "NodeAdvanced", "Advanced to run node {0}."),
		TargetNode->DisplayName.IsEmpty()
			? GetDefaultNodeDisplayName(TargetNode->NodeType)
			: TargetNode->DisplayName);
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

	const bool bMustClaimBattleReward = HasPendingBattleReward();
	const bool bCurrentNodeBlocksAdvance = DoesFlowStageBlockNodeAdvance(CurrentFlowStage);
	const bool bRunEnded = CurrentFlowStage == EFinalRunFlowStage::RunEnded;
	const FText CurrentNodeStateMessage = GetCurrentNodeStateMessage();

	for (const FName& NextNodeId : CurrentNode->NextNodeIds)
	{
		FFinalRunNodeOptionViewData View;
		View.NodeId = NextNodeId;
		View.bVisited = VisitedNodeIds.Contains(NextNodeId);

		if (const FFinalRunNodeDefinition* NextNode = FindNodeDefinition(NextNodeId))
		{
			PopulateNodeViewMetadata(View, *NextNode);

			if (bRunEnded)
			{
				View.bLocked = true;
				View.AvailabilityReason = EFinalRunNodeAvailabilityReason::RunEnded;
				View.AvailabilityMessage = FText::FromString(TEXT("The run can no longer advance."));
			}
			else if (bMustClaimBattleReward)
			{
				View.bLocked = true;
				View.AvailabilityReason = EFinalRunNodeAvailabilityReason::PendingBattleRewardMustBeClaimed;
				View.AvailabilityMessage = FText::FromString(TEXT("Claim the pending battle reward before selecting another node."));
			}
			else if (bCurrentNodeBlocksAdvance)
			{
				View.bLocked = true;
				View.AvailabilityReason = EFinalRunNodeAvailabilityReason::CurrentNodeRequiresResolution;
				View.AvailabilityMessage = CurrentNodeStateMessage;
			}
			else if (NextNode->bStartsLocked)
			{
				View.bLocked = true;
				View.AvailabilityReason = EFinalRunNodeAvailabilityReason::DefinitionLocked;
				View.AvailabilityMessage = NextNode->LockedReason.IsEmpty()
					? FText::FromString(TEXT("This node is locked."))
					: NextNode->LockedReason;
			}
			else if (IsBattleNodeType(NextNode->NodeType) && (!NextNode->EncounterId.IsValid() || !NextNode->RuleConfigId.IsValid()))
			{
				View.bLocked = true;
				View.AvailabilityReason = EFinalRunNodeAvailabilityReason::MissingBattleConfig;
				View.AvailabilityMessage = FText::FromString(TEXT("This battle node is missing encounter or rule config data."));
			}
		}

		Views.Add(View);
	}

	return Views;
}

void UFinalRunSession::ApplyNodeContextFromNode(const FFinalRunNodeDefinition& NodeDefinition)
{
	if (NodeDefinition.IsBattleNode())
	{
		CurrentState.CurrentEncounterId = NodeDefinition.EncounterId;
		CurrentState.CurrentRuleConfigId = NodeDefinition.RuleConfigId;
		CurrentState.bHasPendingBattleStart =
			NodeDefinition.EncounterId.IsValid()
			&& NodeDefinition.RuleConfigId.IsValid()
			&& CurrentState.Characters.Num() > 0
			&& CurrentState.RunDeck.Num() > 0;
	}
	else
	{
		ClearBattleStartContext();
	}
}

void UFinalRunSession::ClearBattleStartContext()
{
	CurrentState.CurrentEncounterId = FFinalEncounterId{};
	CurrentState.CurrentRuleConfigId = FFinalRuleConfigId{};
	CurrentState.bHasPendingBattleStart = false;
}

void UFinalRunSession::PopulateNodeEventMetadata(FFinalRunEvent& Event, const FFinalRunNodeDefinition& NodeDefinition) const
{
	Event.NodeType = NodeDefinition.NodeType;
	Event.NodeDisplayName = NodeDefinition.DisplayName.IsEmpty()
		? GetDefaultNodeDisplayName(NodeDefinition.NodeType)
		: NodeDefinition.DisplayName;
	Event.NodeDisplayLabel = NodeDefinition.DisplayLabel.IsNone()
		? GetDefaultNodeDisplayLabel(NodeDefinition.NodeType)
		: NodeDefinition.DisplayLabel;
	Event.ChapterIndex = NodeDefinition.ChapterIndex;
	Event.FloorIndex = NodeDefinition.FloorIndex;
	Event.EncounterId = NodeDefinition.EncounterId.IsValid() ? NodeDefinition.EncounterId : Event.EncounterId;
	Event.RuleConfigId = NodeDefinition.RuleConfigId.IsValid() ? NodeDefinition.RuleConfigId : Event.RuleConfigId;
}

void UFinalRunSession::PopulateNodeViewMetadata(FFinalRunNodeOptionViewData& View, const FFinalRunNodeDefinition& NodeDefinition) const
{
	View.NodeType = NodeDefinition.NodeType;
	View.DisplayName = NodeDefinition.DisplayName.IsEmpty()
		? GetDefaultNodeDisplayName(NodeDefinition.NodeType)
		: NodeDefinition.DisplayName;
	View.DisplayLabel = NodeDefinition.DisplayLabel.IsNone()
		? GetDefaultNodeDisplayLabel(NodeDefinition.NodeType)
		: NodeDefinition.DisplayLabel;
	View.ChapterIndex = NodeDefinition.ChapterIndex;
	View.FloorIndex = NodeDefinition.FloorIndex;
	View.EncounterId = NodeDefinition.EncounterId;
	View.RuleConfigId = NodeDefinition.RuleConfigId;
	View.bHasImplementedResolver = HasImplementedNodeResolver(NodeDefinition.NodeType);
}

bool UFinalRunSession::HasPendingBattleReward() const
{
	return PendingRewardEntries.Num() > 0;
}

int32 UFinalRunSession::GetPendingBattleRewardGold() const
{
	return GetRewardGoldTotal(PendingRewardEntries);
}

FText UFinalRunSession::GetCurrentNodeStateMessage() const
{
	switch (CurrentFlowStage)
	{
	case EFinalRunFlowStage::PreparingBattle:
		return FText::FromString(TEXT("Finish the current battle node before selecting another node."));

	case EFinalRunFlowStage::PendingBattleReward:
		return FText::FromString(TEXT("Claim the pending battle reward before selecting another node."));

	case EFinalRunFlowStage::PendingRewardNode:
		return FText::FromString(TEXT("Reward nodes are not implemented in the current prototype."));

	case EFinalRunFlowStage::PendingEventNode:
		return FText::FromString(TEXT("Event nodes are not implemented in the current prototype."));

	case EFinalRunFlowStage::PendingShopNode:
		return FText::FromString(TEXT("Shop nodes are not implemented in the current prototype."));

	case EFinalRunFlowStage::RunEnded:
		return FText::FromString(TEXT("The run can no longer advance."));

	default:
		return FText::GetEmpty();
	}
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
