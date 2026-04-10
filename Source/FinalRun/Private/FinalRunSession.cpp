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
	return IsBattleNodeType(NodeType)
		|| NodeType == EFinalRunNodeType::Reward
		|| NodeType == EFinalRunNodeType::Event
		|| NodeType == EFinalRunNodeType::Shop;
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

FText GetDefaultNodeSummary(const EFinalRunNodeType NodeType)
{
	switch (NodeType)
	{
	case EFinalRunNodeType::Reward:
		return NSLOCTEXT("FinalRunSession", "RewardNodeSummaryDefault", "Choose and confirm this node's configured rewards.");

	case EFinalRunNodeType::Event:
		return NSLOCTEXT("FinalRunSession", "EventNodeSummaryDefault", "Review the event text and resolve one option.");

	case EFinalRunNodeType::Shop:
		return NSLOCTEXT("FinalRunSession", "ShopNodeSummaryDefault", "Inspect the current shop offers and resolve one purchase.");

	default:
		return FText::GetEmpty();
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

FText GetDefaultRewardDisplayName(const EFinalRunRewardType RewardType)
{
	switch (RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayGold", "Gold");

	case EFinalRunRewardType::CardGrant:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayCard", "Card");

	case EFinalRunRewardType::RelicGrant:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayRelic", "Relic");

	case EFinalRunRewardType::RemoveCard:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayRemoveCard", "Remove Card");

	case EFinalRunRewardType::UpgradeCard:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayUpgradeCard", "Upgrade Card");

	case EFinalRunRewardType::Growth:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayGrowth", "Growth");

	default:
		return NSLOCTEXT("FinalRunSession", "RewardDisplayUnknown", "Reward");
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

void NormalizeRewardEntries(TArray<FFinalRunRewardEntry>& RewardEntries, const bool bClaimable)
{
	for (FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = GetDefaultRewardDisplayName(Entry.RewardType);
		}

		Entry.bCanClaim = bClaimable;
		Entry.bClaimed = !bClaimable;
	}
}

TArray<FFinalRunRewardEntry> MakeClaimedRewardEntries(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	TArray<FFinalRunRewardEntry> ClaimedEntries = RewardEntries;
	NormalizeRewardEntries(ClaimedEntries, false);
	return ClaimedEntries;
}

TArray<FFinalRunRewardEntry> MakePreviewRewardEntries(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	TArray<FFinalRunRewardEntry> PreviewEntries = RewardEntries;
	NormalizeRewardEntries(PreviewEntries, true);
	return PreviewEntries;
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

void ApplyRewardEntriesToRunState(const TArray<FFinalRunRewardEntry>& RewardEntries, FFinalRunState& RunState)
{
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (!Entry.IsClaimable())
		{
			continue;
		}

		if (Entry.RewardType == EFinalRunRewardType::Gold)
		{
			RunState.Gold += Entry.Value;
		}
	}
}

EFinalRunFlowStage GetFlowStageForNode(const FFinalRunNodeDefinition& NodeDefinition, const bool bNodeResolved, const bool bHasPendingBattleStart)
{
	if (NodeDefinition.IsBattleNode())
	{
		return bHasPendingBattleStart ? EFinalRunFlowStage::PreparingBattle : EFinalRunFlowStage::AwaitingNodeAdvance;
	}

	if (bNodeResolved)
	{
		return EFinalRunFlowStage::AwaitingNodeAdvance;
	}

	switch (NodeDefinition.NodeType)
	{
	case EFinalRunNodeType::Reward:
		return EFinalRunFlowStage::PendingRewardNode;

	case EFinalRunNodeType::Event:
		return EFinalRunFlowStage::PendingEventNode;

	case EFinalRunNodeType::Shop:
		return EFinalRunFlowStage::PendingShopNode;

	default:
		return EFinalRunFlowStage::AwaitingNodeAdvance;
	}
}

const FFinalRunEventOptionDefinition* FindEventOptionDefinition(const FFinalRunNodeDefinition& NodeDefinition, const FName OptionId)
{
	return NodeDefinition.EventContent.Options.FindByPredicate([&OptionId](const FFinalRunEventOptionDefinition& Option)
	{
		return Option.OptionId == OptionId;
	});
}

const FFinalRunShopOfferDefinition* FindShopOfferDefinition(const FFinalRunNodeDefinition& NodeDefinition, const FName OfferId)
{
	return NodeDefinition.ShopContent.Offers.FindByPredicate([&OfferId](const FFinalRunShopOfferDefinition& Offer)
	{
		return Offer.OfferId == OfferId;
	});
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
	ResolvedNodeIds.Reset();
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
		CurrentFlowStage = GetFlowStageForNode(*CurrentNode, ResolvedNodeIds.Contains(CurrentNodeId), CurrentState.bHasPendingBattleStart);
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
	CommandEvent.NodeId = !Command.TargetNodeId.IsNone() ? Command.TargetNodeId : CurrentNodeId;
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

	case EFinalRunCommandType::ResolveReward:
		bAccepted = TryExecuteResolveRewardNode(DetailEvent, RejectReason, FailureMessage);
		break;

	case EFinalRunCommandType::ResolveEvent:
		bAccepted = TryExecuteResolveEventNode(Command.PayloadId, DetailEvent, RejectReason, FailureMessage);
		break;

	case EFinalRunCommandType::ResolveShop:
		bAccepted = TryExecuteResolveShopNode(Command.PayloadId, DetailEvent, RejectReason, FailureMessage);
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

	Snapshot.PendingRewardNode = BuildPendingRewardNodeView();
	Snapshot.PendingEventNode = BuildPendingEventNodeView();
	Snapshot.PendingShopNode = BuildPendingShopNodeView();

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
	ApplyRewardEntriesToRunState(PendingRewardEntries, CurrentState);

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

bool UFinalRunSession::TryExecuteResolveRewardNode(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (CurrentFlowStage != EFinalRunFlowStage::PendingRewardNode)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
		OutFailureMessage = FText::FromString(TEXT("ResolveReward is only valid while the run is on a reward node."));
		return false;
	}

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr || CurrentNode->NodeType != EFinalRunNodeType::Reward)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingRewardNodeContent;
		OutFailureMessage = FText::FromString(TEXT("The current reward node does not provide reward content."));
		return false;
	}

	const TArray<FFinalRunRewardEntry> PreviewEntries = MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = MakeClaimedRewardEntries(PreviewEntries);
	ApplyRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::RewardNodeResolved;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	PopulateNodeEventMetadata(OutDetailEvent, *CurrentNode);
	OutDetailEvent.Message = CurrentNode->RewardContent.Summary.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalRunSession", "RewardNodeResolved", "Resolved reward node {0}."),
			OutDetailEvent.NodeDisplayName)
		: CurrentNode->RewardContent.Summary;
	return true;
}

bool UFinalRunSession::TryExecuteResolveEventNode(const FName& OptionId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (CurrentFlowStage != EFinalRunFlowStage::PendingEventNode)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
		OutFailureMessage = FText::FromString(TEXT("ResolveEvent is only valid while the run is on an event node."));
		return false;
	}

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr || CurrentNode->NodeType != EFinalRunNodeType::Event)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingEventNodeContent;
		OutFailureMessage = FText::FromString(TEXT("The current event node does not provide event content."));
		return false;
	}

	if (OptionId.IsNone())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPayloadId;
		OutFailureMessage = FText::FromString(TEXT("ResolveEvent requires an event option id in PayloadId."));
		return false;
	}

	const FFinalRunEventOptionDefinition* SelectedOption = FindEventOptionDefinition(*CurrentNode, OptionId);
	if (SelectedOption == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownEventOption;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "UnknownEventOption", "Event option {0} is not defined on the current node."),
			FText::FromName(OptionId));
		return false;
	}

	if (SelectedOption->bStartsDisabled)
	{
		OutRejectReason = EFinalRunCommandRejectReason::EventOptionDisabled;
		OutFailureMessage = SelectedOption->DisabledReason.IsEmpty()
			? FText::FromString(TEXT("The selected event option is currently disabled."))
			: SelectedOption->DisabledReason;
		return false;
	}

	const TArray<FFinalRunRewardEntry> PreviewEntries = MakePreviewRewardEntries(SelectedOption->RewardEntries);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = MakeClaimedRewardEntries(PreviewEntries);
	ApplyRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::EventNodeResolved;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.PayloadId = OptionId;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	PopulateNodeEventMetadata(OutDetailEvent, *CurrentNode);
	OutDetailEvent.Message = SelectedOption->OutcomeSummary.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalRunSession", "EventNodeResolved", "Resolved event node {0} with option {1}."),
			OutDetailEvent.NodeDisplayName,
			FText::FromName(OptionId))
		: SelectedOption->OutcomeSummary;
	return true;
}

bool UFinalRunSession::TryExecuteResolveShopNode(const FName& OfferId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (CurrentFlowStage != EFinalRunFlowStage::PendingShopNode)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedCommand;
		OutFailureMessage = FText::FromString(TEXT("ResolveShop is only valid while the run is on a shop node."));
		return false;
	}

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr || CurrentNode->NodeType != EFinalRunNodeType::Shop)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingShopNodeContent;
		OutFailureMessage = FText::FromString(TEXT("The current shop node does not provide shop content."));
		return false;
	}

	if (OfferId.IsNone())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPayloadId;
		OutFailureMessage = FText::FromString(TEXT("ResolveShop requires a shop offer id in PayloadId."));
		return false;
	}

	const FFinalRunShopOfferDefinition* SelectedOffer = FindShopOfferDefinition(*CurrentNode, OfferId);
	if (SelectedOffer == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownShopOffer;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "UnknownShopOffer", "Shop offer {0} is not defined on the current node."),
			FText::FromName(OfferId));
		return false;
	}

	if (SelectedOffer->bStartsUnavailable)
	{
		OutRejectReason = EFinalRunCommandRejectReason::ShopOfferUnavailable;
		OutFailureMessage = SelectedOffer->UnavailableReason.IsEmpty()
			? FText::FromString(TEXT("The selected shop offer is currently unavailable."))
			: SelectedOffer->UnavailableReason;
		return false;
	}

	if (CurrentState.Gold < SelectedOffer->Price)
	{
		OutRejectReason = EFinalRunCommandRejectReason::InsufficientGold;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "InsufficientGold", "The selected shop offer costs {0} gold, but the run only has {1}."),
			FText::AsNumber(SelectedOffer->Price),
			FText::AsNumber(CurrentState.Gold));
		return false;
	}

	const TArray<FFinalRunRewardEntry> PreviewEntries = MakePreviewRewardEntries(SelectedOffer->RewardEntries);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = MakeClaimedRewardEntries(PreviewEntries);
	CurrentState.Gold -= SelectedOffer->Price;
	ApplyRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::ShopOfferPurchased;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.PayloadId = OfferId;
	OutDetailEvent.SpentGold = SelectedOffer->Price;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	PopulateNodeEventMetadata(OutDetailEvent, *CurrentNode);
	OutDetailEvent.Message = SelectedOffer->Description.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalRunSession", "ShopOfferPurchased", "Purchased shop offer {0}."),
			SelectedOffer->DisplayName.IsEmpty() ? FText::FromName(OfferId) : SelectedOffer->DisplayName)
		: SelectedOffer->Description;
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
	CurrentFlowStage = GetFlowStageForNode(*TargetNode, ResolvedNodeIds.Contains(TargetNodeId), CurrentState.bHasPendingBattleStart);

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

FFinalRunPendingRewardNodeViewData UFinalRunSession::BuildPendingRewardNodeView() const
{
	FFinalRunPendingRewardNodeViewData View;

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr || CurrentNode->NodeType != EFinalRunNodeType::Reward)
	{
		return View;
	}

	const bool bResolved = ResolvedNodeIds.Contains(CurrentNodeId);
	View.bHasPendingContent = true;
	View.NodeId = CurrentNodeId;
	View.Title = CurrentNode->RewardContent.Title.IsEmpty()
		? (CurrentNode->DisplayName.IsEmpty() ? GetDefaultNodeDisplayName(CurrentNode->NodeType) : CurrentNode->DisplayName)
		: CurrentNode->RewardContent.Title;
	View.Summary = CurrentNode->RewardContent.Summary.IsEmpty()
		? GetDefaultNodeSummary(CurrentNode->NodeType)
		: CurrentNode->RewardContent.Summary;
	View.bCanResolve = CurrentFlowStage == EFinalRunFlowStage::PendingRewardNode;
	View.bResolved = bResolved;
	View.RewardEntries = bResolved
		? MakeClaimedRewardEntries(MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries))
		: MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries);
	return View;
}

FFinalRunPendingEventNodeViewData UFinalRunSession::BuildPendingEventNodeView() const
{
	FFinalRunPendingEventNodeViewData View;

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr || CurrentNode->NodeType != EFinalRunNodeType::Event)
	{
		return View;
	}

	const bool bResolved = ResolvedNodeIds.Contains(CurrentNodeId);
	View.bHasPendingContent = true;
	View.NodeId = CurrentNodeId;
	View.Title = CurrentNode->EventContent.Title.IsEmpty()
		? (CurrentNode->DisplayName.IsEmpty() ? GetDefaultNodeDisplayName(CurrentNode->NodeType) : CurrentNode->DisplayName)
		: CurrentNode->EventContent.Title;
	View.Summary = CurrentNode->EventContent.Summary.IsEmpty()
		? GetDefaultNodeSummary(CurrentNode->NodeType)
		: CurrentNode->EventContent.Summary;
	View.bResolved = bResolved;

	for (const FFinalRunEventOptionDefinition& Option : CurrentNode->EventContent.Options)
	{
		FFinalRunEventOptionViewData OptionView;
		OptionView.OptionId = Option.OptionId;
		OptionView.DisplayText = Option.DisplayText.IsEmpty()
			? FText::FromName(Option.OptionId)
			: Option.DisplayText;
		OptionView.OutcomeSummary = Option.OutcomeSummary;
		OptionView.RewardEntries = MakePreviewRewardEntries(Option.RewardEntries);
		OptionView.bSelectable = !bResolved && !Option.bStartsDisabled;
		OptionView.AvailabilityMessage = Option.bStartsDisabled
			? (Option.DisabledReason.IsEmpty() ? FText::FromString(TEXT("This option is currently unavailable.")) : Option.DisabledReason)
			: FText::GetEmpty();
		if (bResolved)
		{
			OptionView.bSelectable = false;
			OptionView.AvailabilityMessage = FText::FromString(TEXT("This event node has already been resolved."));
		}

		if (OptionView.bSelectable)
		{
			View.bCanResolve = true;
		}

		View.Options.Add(OptionView);
	}

	return View;
}

FFinalRunPendingShopNodeViewData UFinalRunSession::BuildPendingShopNodeView() const
{
	FFinalRunPendingShopNodeViewData View;

	const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId);
	if (CurrentNode == nullptr || CurrentNode->NodeType != EFinalRunNodeType::Shop)
	{
		return View;
	}

	const bool bResolved = ResolvedNodeIds.Contains(CurrentNodeId);
	View.bHasPendingContent = true;
	View.NodeId = CurrentNodeId;
	View.Title = CurrentNode->ShopContent.Title.IsEmpty()
		? (CurrentNode->DisplayName.IsEmpty() ? GetDefaultNodeDisplayName(CurrentNode->NodeType) : CurrentNode->DisplayName)
		: CurrentNode->ShopContent.Title;
	View.Summary = CurrentNode->ShopContent.Summary.IsEmpty()
		? GetDefaultNodeSummary(CurrentNode->NodeType)
		: CurrentNode->ShopContent.Summary;
	View.bResolved = bResolved;

	for (const FFinalRunShopOfferDefinition& Offer : CurrentNode->ShopContent.Offers)
	{
		FFinalRunShopOfferViewData OfferView;
		OfferView.OfferId = Offer.OfferId;
		OfferView.DisplayId = Offer.DisplayId;
		OfferView.DisplayName = Offer.DisplayName.IsEmpty()
			? FText::FromName(Offer.OfferId)
			: Offer.DisplayName;
		OfferView.Description = Offer.Description;
		OfferView.Price = Offer.Price;
		OfferView.bPurchased = bResolved;
		OfferView.RewardEntries = MakePreviewRewardEntries(Offer.RewardEntries);

		if (bResolved)
		{
			OfferView.bPurchasable = false;
			OfferView.AvailabilityMessage = FText::FromString(TEXT("This shop node has already been resolved."));
		}
		else if (Offer.bStartsUnavailable)
		{
			OfferView.bPurchasable = false;
			OfferView.AvailabilityMessage = Offer.UnavailableReason.IsEmpty()
				? FText::FromString(TEXT("This offer is currently unavailable."))
				: Offer.UnavailableReason;
		}
		else if (CurrentState.Gold < Offer.Price)
		{
			OfferView.bPurchasable = false;
			OfferView.AvailabilityMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "ShopOfferNeedsMoreGold", "Requires {0} gold."),
				FText::AsNumber(Offer.Price));
		}
		else
		{
			OfferView.bPurchasable = true;
			View.bCanResolve = true;
		}

		View.Offers.Add(OfferView);
	}

	return View;
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

void UFinalRunSession::MarkCurrentNodeResolved()
{
	if (!CurrentNodeId.IsNone())
	{
		ResolvedNodeIds.Add(CurrentNodeId);
	}
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
		return FText::FromString(TEXT("Resolve the current reward node before selecting another node."));

	case EFinalRunFlowStage::PendingEventNode:
		return FText::FromString(TEXT("Resolve the current event node before selecting another node."));

	case EFinalRunFlowStage::PendingShopNode:
		return FText::FromString(TEXT("Resolve the current shop node before selecting another node."));

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
