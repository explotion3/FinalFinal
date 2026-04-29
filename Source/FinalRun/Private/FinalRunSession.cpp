#include "Facade/FinalRunSession.h"

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Events/FinalRunEventResolver.h"
#include "Growth/FinalGrowthResolver.h"
#include "Queries/FinalDataRegistry.h"
#include "Rewards/FinalRewardResolver.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"
#include "Run/Definitions/FinalCardEvolutionDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Shops/FinalShopResolver.h"
#include "Subsystems/GameInstanceSubsystem.h"

namespace
{
FText ResolveRunCharacterDisplayName(const FFinalCharacterId& CharacterId, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry != nullptr && CharacterId.IsValid())
	{
		if (const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(CharacterId))
		{
			if (!CharacterDefinition->DisplayName.IsEmpty())
			{
				return CharacterDefinition->DisplayName;
			}
		}
	}

	return CharacterId.IsValid()
		? FText::FromName(CharacterId.Value)
		: NSLOCTEXT("FinalRunSession", "UnknownRunCharacter", "Unknown Character");
}

FName ResolveRunCharacterIconId(const FFinalCharacterId& CharacterId)
{
	return CharacterId.IsValid() ? CharacterId.Value : NAME_None;
}

FText BuildRunCharacterStateSummaryText(const FFinalRunPersistentCharacterState& CharacterState)
{
	return FText::Format(
		NSLOCTEXT("FinalRunSession", "RunCharacterStateSummary", "Stress {0} | Awaken {1} | Collapse {2}"),
		FText::AsNumber(CharacterState.CurrentStress),
		FText::AsNumber(CharacterState.CurrentAwakenCount),
		FText::AsNumber(CharacterState.CollapseCount));
}

FFinalRunCharacterViewData MakeCharacterView(const FFinalRunPersistentCharacterState& CharacterState, const UFinalDataRegistry* DataRegistry)
{
	FFinalRunCharacterViewData View;
	View.CharacterId = CharacterState.CharacterId;
	View.DisplayName = ResolveRunCharacterDisplayName(CharacterState.CharacterId, DataRegistry);
	View.IconId = ResolveRunCharacterIconId(CharacterState.CharacterId);
	View.Level = CharacterState.Level;
	View.BreakthroughValue = CharacterState.BreakthroughValue;
	View.BreakthroughRequiredValue = CharacterState.BreakthroughRequiredValue;
	View.RootBone = CharacterState.RootBone;
	View.Insight = CharacterState.Insight;
	View.KillingIntent = CharacterState.KillingIntent;
	View.bHasPendingGrowthChoice = CharacterState.bHasPendingGrowthChoice;
	View.CurrentStress = CharacterState.CurrentStress;
	View.bCollapsed = CharacterState.bCollapsed;
	View.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
	View.CollapseCount = CharacterState.CollapseCount;
	View.StateSummaryText = BuildRunCharacterStateSummaryText(CharacterState);
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

bool DoesFlowStageBlockNodeAdvance(const EFinalRunFlowStage FlowStage)
{
	return FlowStage == EFinalRunFlowStage::PreparingBattle
		|| FlowStage == EFinalRunFlowStage::PendingBattleReward
		|| FlowStage == EFinalRunFlowStage::PendingRewardNode
		|| FlowStage == EFinalRunFlowStage::PendingEventNode
		|| FlowStage == EFinalRunFlowStage::PendingShopNode;
}

const UFinalCardDefinition* FindRewardCardDefinition(const FFinalCardId& CardId, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry == nullptr || !CardId.IsValid())
	{
		return nullptr;
	}

	return DataRegistry->FindCardDefinition(CardId);
}

int32 CountRunDeckCards(const TArray<FFinalRunCardInstance>& RunDeck, const FFinalCardId& CardId)
{
	int32 Count = 0;
	for (const FFinalRunCardInstance& DeckCardInstance : RunDeck)
	{
		if (DeckCardInstance.GetEffectiveCardId() == CardId)
		{
			++Count;
		}
	}

	return Count;
}

struct FFinalBattleCardRewardCandidate
{
	FFinalCardId CardId;
	int32 ExistingDeckCount = 0;
	int32 SourceOrder = 0;
};

TArray<FFinalRunRewardEntry> BuildBattleCardRewardEntries(
	const FFinalBattleResult& Result,
	const FFinalRunState& RunState,
	const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntry> RewardEntries;
	if (Result.Outcome != EFinalBattleOutcome::Victory || DataRegistry == nullptr)
	{
		return RewardEntries;
	}

	TArray<FFinalBattleCardRewardCandidate> Candidates;
	TSet<FName> SeenCardIds;
	int32 SourceOrder = 0;

	for (const FFinalRunPersistentCharacterState& CharacterState : RunState.Characters)
	{
		const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(CharacterState.CharacterId);
		if (CharacterDefinition == nullptr)
		{
			continue;
		}

		for (const FFinalCardId& CardId : CharacterDefinition->CharacterCardPoolIds)
		{
			if (!CardId.IsValid() || SeenCardIds.Contains(CardId.Value))
			{
				++SourceOrder;
				continue;
			}

			if (DataRegistry->FindCardDefinition(CardId) == nullptr)
			{
				++SourceOrder;
				continue;
			}

			SeenCardIds.Add(CardId.Value);

			FFinalBattleCardRewardCandidate Candidate;
			Candidate.CardId = CardId;
			Candidate.ExistingDeckCount = CountRunDeckCards(RunState.RunDeck, CardId);
			Candidate.SourceOrder = SourceOrder++;
			Candidates.Add(MoveTemp(Candidate));
		}
	}

	Candidates.StableSort([](const FFinalBattleCardRewardCandidate& Left, const FFinalBattleCardRewardCandidate& Right)
	{
		if (Left.ExistingDeckCount != Right.ExistingDeckCount)
		{
			return Left.ExistingDeckCount < Right.ExistingDeckCount;
		}

		return Left.SourceOrder < Right.SourceOrder;
	});

	const int32 RewardCount = FMath::Min(3, Candidates.Num());
	for (int32 RewardIndex = 0; RewardIndex < RewardCount; ++RewardIndex)
	{
		const FFinalCardId& CardId = Candidates[RewardIndex].CardId;
		FFinalRunRewardEntry CardEntry;
		CardEntry.RewardId = *FString::Printf(TEXT("BattleReward.Card.%d.%s"), RewardIndex, *CardId.Value.ToString());
		CardEntry.RewardType = EFinalRunRewardType::CardGrant;
		CardEntry.Value = 1;
		CardEntry.GrantedCardId = CardId;
		CardEntry.DisplayId = CardId.Value;
		CardEntry.bCanClaim = true;
		CardEntry.bClaimed = false;
		if (const UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(CardId))
		{
			CardEntry.DisplayName = CardDefinition->DisplayName;
		}
		RewardEntries.Add(MoveTemp(CardEntry));
	}

	return RewardEntries;
}

const UFinalDataRegistry* ResolveDataRegistry(const UFinalRunSession* RunSession)
{
	if (RunSession == nullptr)
	{
		return nullptr;
	}

	if (const UGameInstance* GameInstance = RunSession->GetTypedOuter<UGameInstance>())
	{
		return GameInstance->GetSubsystem<UFinalDataRegistry>();
	}

	if (const UGameInstanceSubsystem* GameInstanceSubsystem = RunSession->GetTypedOuter<UGameInstanceSubsystem>())
	{
		if (UGameInstance* GameInstance = GameInstanceSubsystem->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UFinalDataRegistry>();
		}
	}

	if (const UWorld* World = RunSession->GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UFinalDataRegistry>();
		}
	}

	return nullptr;
}

FText ResolveDeckEntryDisplayName(const FFinalCardId& CardId, const UFinalDataRegistry* DataRegistry)
{
	if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(CardId, DataRegistry))
	{
		if (!CardDefinition->DisplayName.IsEmpty())
		{
			return CardDefinition->DisplayName;
		}
	}

	return CardId.IsValid()
		? FText::FromName(CardId.Value)
		: NSLOCTEXT("FinalRunSession", "UnknownDeckCard", "Unknown Card");
}

FText GetGrowthAttributeDisplayName(const EFinalGrowthAttributeType AttributeType)
{
	switch (AttributeType)
	{
	case EFinalGrowthAttributeType::RootBone:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeRootBone", "Root Bone +1");

	case EFinalGrowthAttributeType::Insight:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeInsight", "Insight +1");

	case EFinalGrowthAttributeType::KillingIntent:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeKillingIntent", "Killing Intent +1");

	default:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeUnknown", "Growth +1");
	}
}

FText GetGrowthAttributeDescription(const EFinalGrowthAttributeType AttributeType)
{
	switch (AttributeType)
	{
	case EFinalGrowthAttributeType::RootBone:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeRootBoneDescription", "Increase vitality share, defense, and stress cap.");

	case EFinalGrowthAttributeType::Insight:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeInsightDescription", "Improve future breakthrough gain efficiency.");

	case EFinalGrowthAttributeType::KillingIntent:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeKillingIntentDescription", "Increase attack, critical chance, and critical damage.");

	default:
		return NSLOCTEXT("FinalRunSession", "GrowthAttributeUnknownDescription", "Increase a growth attribute.");
	}
}

FName BuildGrowthChoiceInstanceId(const FFinalCharacterId& CharacterId, const int32 CharacterLevel, const int32 ChoiceIndex)
{
	return FName(*FString::Printf(
		TEXT("GrowthChoice_%s_Lv%d_%d"),
		*CharacterId.Value.ToString(),
		CharacterLevel,
		ChoiceIndex));
}

FFinalRunGrowthChoiceInstance MakeAttributeGrowthChoice(
	const FFinalRunPersistentCharacterState& CharacterState,
	const EFinalGrowthAttributeType AttributeType,
	const int32 ChoiceIndex)
{
	FFinalRunGrowthChoiceInstance Choice;
	Choice.ChoiceInstanceId = BuildGrowthChoiceInstanceId(CharacterState.CharacterId, CharacterState.Level, ChoiceIndex);
	Choice.ChoiceType = EFinalGrowthChoiceType::AttributeGrowth;
	Choice.CharacterId = CharacterState.CharacterId;
	Choice.AttributeType = AttributeType;
	Choice.AttributeDelta = 1;
	Choice.DisplayName = GetGrowthAttributeDisplayName(AttributeType);
	Choice.Description = GetGrowthAttributeDescription(AttributeType);
	return Choice;
}

bool IsEligibleEvolutionChoice(
	const UFinalCardEvolutionDefinition& EvolutionDefinition,
	const FFinalRunCardInstance& CardInstance,
	const FFinalRunPersistentCharacterState& CharacterState)
{
	if (!EvolutionDefinition.bAllowAsLevelUpCandidate
		|| !CardInstance.OwnerCharacterId.IsValid()
		|| CardInstance.OwnerCharacterId != CharacterState.CharacterId
		|| CardInstance.CurrentCardId != EvolutionDefinition.FromCardId
		|| CardInstance.EvolutionStage != EvolutionDefinition.FromStage)
	{
		return false;
	}

	if (EvolutionDefinition.RequiredOwnerCharacterId.IsValid()
		&& EvolutionDefinition.RequiredOwnerCharacterId != CharacterState.CharacterId)
	{
		return false;
	}

	return EvolutionDefinition.RequiredCardTags.IsEmpty();
}

FFinalRunGrowthChoiceInstance MakeEvolutionGrowthChoice(
	const FFinalRunPersistentCharacterState& CharacterState,
	const FFinalRunCardInstance& CardInstance,
	const UFinalCardEvolutionDefinition& EvolutionDefinition,
	const int32 ChoiceIndex,
	const UFinalDataRegistry* DataRegistry)
{
	FFinalRunGrowthChoiceInstance Choice;
	Choice.ChoiceInstanceId = BuildGrowthChoiceInstanceId(CharacterState.CharacterId, CharacterState.Level, ChoiceIndex);
	Choice.ChoiceType = EFinalGrowthChoiceType::CardEvolution;
	Choice.CharacterId = CharacterState.CharacterId;
	Choice.TargetRunCardInstanceId = CardInstance.InstanceId;
	Choice.CardEvolutionId = EvolutionDefinition.EvolutionId;
	Choice.FromCardId = EvolutionDefinition.FromCardId;
	Choice.ToCardId = EvolutionDefinition.ToCardId;
	Choice.DisplayName = EvolutionDefinition.DisplayName;
	Choice.Description = EvolutionDefinition.Description;

	if (Choice.DisplayName.IsEmpty())
	{
		if (const UFinalCardDefinition* CardDefinition = DataRegistry != nullptr ? DataRegistry->FindCardDefinition(EvolutionDefinition.ToCardId) : nullptr)
		{
			Choice.DisplayName = CardDefinition->DisplayName;
		}

		if (Choice.DisplayName.IsEmpty())
		{
			Choice.DisplayName = EvolutionDefinition.ToCardId.IsValid()
				? FText::FromName(EvolutionDefinition.ToCardId.Value)
				: NSLOCTEXT("FinalRunSession", "GrowthEvolutionFallbackDisplayName", "Card Evolution");
		}
	}

	if (Choice.Description.IsEmpty())
	{
		Choice.Description = NSLOCTEXT("FinalRunSession", "GrowthEvolutionFallbackDescription", "Evolve this card into its stronger run form.");
	}

	return Choice;
}

FName ResolveRelicEntryDisplayId(const FFinalRelicId& RelicId, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry != nullptr && RelicId.IsValid())
	{
		if (const UFinalRelicDefinition* RelicDefinition = DataRegistry->FindRelicDefinition(RelicId))
		{
			if (!RelicDefinition->DisplayId.IsNone())
			{
				return RelicDefinition->DisplayId;
			}
		}
	}

	return RelicId.IsValid() ? RelicId.Value : NAME_None;
}

FText ResolveRelicEntryDisplayName(const FFinalRelicId& RelicId, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry != nullptr && RelicId.IsValid())
	{
		if (const UFinalRelicDefinition* RelicDefinition = DataRegistry->FindRelicDefinition(RelicId))
		{
			if (!RelicDefinition->DisplayName.IsEmpty())
			{
				return RelicDefinition->DisplayName;
			}
		}
	}

	const FName DisplayId = ResolveRelicEntryDisplayId(RelicId, DataRegistry);
	if (!DisplayId.IsNone())
	{
		return FText::FromName(DisplayId);
	}

	return RelicId.IsValid()
		? FText::FromName(RelicId.Value)
		: NSLOCTEXT("FinalRunSession", "UnknownRelic", "Unknown Relic");
}

TArray<FFinalBattleStartRelicInput> BuildBattleStartRelicInputs(const FFinalRunState& RunState, const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalBattleStartRelicInput> BattleStartRelics;
	if (DataRegistry == nullptr)
	{
		return BattleStartRelics;
	}

	for (const FFinalRelicId& RelicId : RunState.Relics)
	{
		if (!RelicId.IsValid())
		{
			continue;
		}

		const UFinalRelicDefinition* RelicDefinition = DataRegistry->FindRelicDefinition(RelicId);
		if (RelicDefinition == nullptr)
		{
			continue;
		}

		FFinalBattleStartRelicInput RelicInput;
		RelicInput.RelicId = RelicId;
		RelicInput.DisplayId = ResolveRelicEntryDisplayId(RelicId, DataRegistry);
		RelicInput.DisplayName = ResolveRelicEntryDisplayName(RelicId, DataRegistry);

		for (const FFinalRelicBattleStartEffectDefinition& EffectDefinition : RelicDefinition->BattleStartEffects)
		{
			if (EffectDefinition.EffectType == EFinalRelicBattleStartEffectType::None || EffectDefinition.Value <= 0)
			{
				continue;
			}

			FFinalBattleStartRelicEffectInput EffectInput;
			EffectInput.EffectType = EffectDefinition.EffectType;
			EffectInput.Value = EffectDefinition.Value;
			RelicInput.BattleStartEffects.Add(EffectInput);
		}

		for (const FFinalRelicPlayerTurnStartEffectDefinition& EffectDefinition : RelicDefinition->PlayerTurnStartEffects)
		{
			if (EffectDefinition.EffectType == EFinalRelicPlayerTurnStartEffectType::None || EffectDefinition.Value <= 0)
			{
				continue;
			}

			FFinalBattlePlayerTurnStartRelicEffectInput EffectInput;
			EffectInput.EffectType = EffectDefinition.EffectType;
			EffectInput.Value = EffectDefinition.Value;
			RelicInput.PlayerTurnStartEffects.Add(EffectInput);
		}

		for (const FFinalRuntimeTriggerDefinition& TriggerDefinition : RelicDefinition->RuntimeTriggers)
		{
			if (TriggerDefinition.Domain != EFinalRuntimeTriggerDomain::Battle
				|| TriggerDefinition.Window == EFinalRuntimeTriggerWindow::None
				|| TriggerDefinition.Effects.IsEmpty())
			{
				continue;
			}

			RelicInput.RuntimeTriggers.Add(TriggerDefinition);
		}

		if (RelicInput.BattleStartEffects.Num() > 0
			|| RelicInput.PlayerTurnStartEffects.Num() > 0
			|| RelicInput.RuntimeTriggers.Num() > 0)
		{
			BattleStartRelics.Add(MoveTemp(RelicInput));
		}
	}

	return BattleStartRelics;
}

FFinalRunCurrentBuildViewData BuildCurrentBuildViewData(const FFinalRunState& RunState, const UFinalDataRegistry* DataRegistry)
{
	FFinalRunCurrentBuildViewData ViewData;

	TMap<FName, int32> DeckCardCounts;
	TArray<FFinalCardId> OrderedDeckCardIds;
	for (const FFinalRunCardInstance& CardInstance : RunState.RunDeck)
	{
		const FFinalCardId CardId = CardInstance.GetEffectiveCardId();
		if (!CardId.IsValid())
		{
			continue;
		}

		int32& Count = DeckCardCounts.FindOrAdd(CardId.Value);
		if (Count == 0)
		{
			OrderedDeckCardIds.Add(CardId);
		}

		++Count;
	}

	for (const FFinalCardId& CardId : OrderedDeckCardIds)
	{
		FFinalRunDeckEntryViewData Entry;
		Entry.CardId = CardId;
		Entry.DisplayName = ResolveDeckEntryDisplayName(CardId, DataRegistry);
		Entry.Count = DeckCardCounts.FindRef(CardId.Value);
		ViewData.DeckEntries.Add(MoveTemp(Entry));
	}

	TMap<FName, int32> RelicCounts;
	TArray<FFinalRelicId> OrderedRelicIds;
	for (const FFinalRelicId& RelicId : RunState.Relics)
	{
		if (!RelicId.IsValid())
		{
			continue;
		}

		int32& Count = RelicCounts.FindOrAdd(RelicId.Value);
		if (Count == 0)
		{
			OrderedRelicIds.Add(RelicId);
		}

		++Count;
	}

	for (const FFinalRelicId& RelicId : OrderedRelicIds)
	{
		FFinalRunRelicEntryViewData Entry;
		Entry.RelicId = RelicId;
		Entry.DisplayId = ResolveRelicEntryDisplayId(RelicId, DataRegistry);
		Entry.DisplayName = ResolveRelicEntryDisplayName(RelicId, DataRegistry);
		Entry.Count = RelicCounts.FindRef(RelicId.Value);
		ViewData.RelicEntries.Add(MoveTemp(Entry));
	}

	return ViewData;
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
	ConfiguredRouteId = NAME_None;
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
	ConfiguredRouteId = NAME_None;
	ConfigureRunNodeGraphInternal(NodeDefinitions, InCurrentNodeId);
}

bool UFinalRunSession::ConfigureRunRouteById(const FName RouteId)
{
	if (RouteId.IsNone())
	{
		return false;
	}

	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	if (DataRegistry == nullptr)
	{
		return false;
	}

	const UFinalRunRouteDefinition* RouteDefinition = DataRegistry->FindRunRouteDefinition(RouteId);
	return RouteDefinition != nullptr && ConfigureRunRouteDefinitionInternal(*RouteDefinition);
}

void UFinalRunSession::ConfigureRunNodeGraphInternal(const TArray<FFinalRunNodeDefinition>& NodeDefinitions, const FName InCurrentNodeId)
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

bool UFinalRunSession::ConfigureRunRouteDefinitionInternal(const UFinalRunRouteDefinition& RouteDefinition)
{
	if (!RouteDefinition.IsValidDefinition())
	{
		return false;
	}

	ConfiguredRouteId = RouteDefinition.RouteId;
	ConfigureRunNodeGraphInternal(RouteDefinition.NodeDefinitions, RouteDefinition.EntryNodeId);
	return true;
}

void UFinalRunSession::ConfigureBattleStartState(const FFinalEncounterId& EncounterId, const FFinalRuleConfigId& RuleConfigId, const TArray<FFinalRunPersistentCharacterState>& PartyStates, const TArray<FFinalCardId>& DeckCardIds, int32 InTeamCurrentHP)
{
	CurrentState.CurrentEncounterId = EncounterId;
	CurrentState.CurrentRuleConfigId = RuleConfigId;
	CurrentState.Characters = PartyStates;
	CurrentState.RunDeck.Reset();
	CurrentState.NextRunCardInstanceSerial = 1;

	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	for (const FFinalCardId& CardId : DeckCardIds)
	{
		if (!CardId.IsValid())
		{
			continue;
		}

		FFinalCharacterId OwnerCharacterId;
		if (const UFinalCardDefinition* CardDefinition = DataRegistry != nullptr ? DataRegistry->FindCardDefinition(CardId) : nullptr)
		{
			if (!CardDefinition->OwnerUnitId.IsNone())
			{
				OwnerCharacterId = FFinalCharacterId(CardDefinition->OwnerUnitId);
			}
		}

		const FName InstanceId = FName(*FString::Printf(TEXT("RunCard_%d"), CurrentState.NextRunCardInstanceSerial++));
		CurrentState.RunDeck.Add(FFinalRunCardInstance::Make(InstanceId, CardId, OwnerCharacterId));
	}
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

bool UFinalRunSession::ClaimPendingBattleRewardById(const FName RewardId)
{
	FFinalRunCommand Command;
	Command.CommandType = EFinalRunCommandType::ClaimPendingBattleReward;
	Command.PayloadId = RewardId;
	return SubmitRunCommand(Command);
}

bool UFinalRunSession::SkipPendingBattleReward()
{
	FFinalRunCommand Command;
	Command.CommandType = EFinalRunCommandType::SkipPendingBattleReward;
	return SubmitRunCommand(Command);
}

bool UFinalRunSession::AdvanceToNode(FName NodeId)
{
	FFinalRunCommand Command;
	Command.CommandType = EFinalRunCommandType::AdvanceToNode;
	Command.TargetNodeId = NodeId;
	return SubmitRunCommand(Command);
}

bool UFinalRunSession::AddBreakthroughValue(const FFinalCharacterId& CharacterId, const int32 Amount)
{
	if (!CharacterId.IsValid() || Amount <= 0)
	{
		return false;
	}

	FFinalRunPersistentCharacterState* CharacterState = FindMutableCharacterState(CharacterId);
	if (CharacterState == nullptr)
	{
		return false;
	}

	CharacterState->BreakthroughValue += Amount;
	if (HasPendingGrowthChoice())
	{
		return true;
	}

	return CharacterState->BreakthroughRequiredValue > 0
		&& CharacterState->BreakthroughValue >= CharacterState->BreakthroughRequiredValue
		? TryLevelUpCharacter(*CharacterState)
		: true;
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
		bAccepted = TryExecuteClaimPendingBattleReward(Command.PayloadId, DetailEvent, RejectReason, FailureMessage);
		break;

	case EFinalRunCommandType::SkipPendingBattleReward:
		bAccepted = TryExecuteSkipPendingBattleReward(DetailEvent, RejectReason, FailureMessage);
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

	case EFinalRunCommandType::SelectGrowthChoice:
		bAccepted = TryExecuteSelectGrowthChoice(Command.PayloadId, DetailEvent, RejectReason, FailureMessage);
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

bool UFinalRunSession::HasPendingGrowthChoice() const
{
	return CurrentState.PendingGrowthChoice.bIsValid;
}

FFinalBattleStartRequest UFinalRunSession::BuildBattleStartRequest() const
{
	FFinalBattleStartRequest Request;
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	Request.EncounterId = CurrentState.CurrentEncounterId;
	Request.RuleConfigId = CurrentState.CurrentRuleConfigId;
	Request.TeamCurrentHP = CurrentState.TeamCurrentHP;
	Request.DeckCardIds.Reset();
	Request.DeckCardIds.Reserve(CurrentState.RunDeck.Num());
	for (const FFinalRunCardInstance& CardInstance : CurrentState.RunDeck)
	{
		const FFinalCardId EffectiveCardId = CardInstance.GetEffectiveCardId();
		if (EffectiveCardId.IsValid())
		{
			Request.DeckCardIds.Add(EffectiveCardId);
		}
	}
	Request.PartyStates = CurrentState.Characters;
	Request.BattleStartRelics = BuildBattleStartRelicInputs(CurrentState, DataRegistry);

	for (const FFinalRunPersistentCharacterState& CharacterState : CurrentState.Characters)
	{
		Request.PartyCharacterIds.Add(CharacterState.CharacterId);
	}

	return Request;
}

const FFinalRunPendingGrowthChoice& UFinalRunSession::GetPendingGrowthChoice() const
{
	return CurrentState.PendingGrowthChoice;
}

void UFinalRunSession::ApplyBattleResult(const FFinalBattleResult& Result)
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	const FFinalRuleConfigId ResolvedRuleConfigId = CurrentState.CurrentRuleConfigId;
	CurrentState.LastResolvedEncounterId = Result.EncounterId;
	CurrentState.LastBattleOutcome = Result.Outcome;
	CurrentState.LastBattleRewardGold = Result.RewardGold;
	CurrentState.TeamCurrentHP = Result.TeamCurrentHP;
	if (Result.Outcome == EFinalBattleOutcome::Victory && Result.RewardGold > 0)
	{
		CurrentState.Gold += Result.RewardGold;
	}
	ClearBattleStartContext();
	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardEntries.Reset();

	if (Result.UpdatedCharacterStates.Num() > 0)
	{
		CurrentState.Characters = Result.UpdatedCharacterStates;
	}

	if (Result.Outcome == EFinalBattleOutcome::Victory)
	{
		MarkCurrentNodeResolved();
	}

	PendingRewardEntries = BuildBattleCardRewardEntries(Result, CurrentState, DataRegistry);

	if (PendingRewardEntries.Num() > 0)
	{
		PendingRewardSourceNodeId = CurrentNodeId;
		PendingRewardSourceEncounterId = Result.EncounterId;
		PendingRewardBattleOutcome = Result.Outcome;
		CurrentFlowStage = EFinalRunFlowStage::PendingBattleReward;
	}
	else if (Result.Outcome == EFinalBattleOutcome::Victory)
	{
		SetFlowStageAfterPostBattleReward();
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
	Event.RewardGold = Result.Outcome == EFinalBattleOutcome::Victory ? Result.RewardGold : 0;
	Event.RewardEntries = FFinalRewardResolver::MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);
	FFinalRewardResolver::PopulateRewardEventViewData(Event, DataRegistry);
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
		RewardEvent.RewardGold = Result.Outcome == EFinalBattleOutcome::Victory ? Result.RewardGold : 0;
		RewardEvent.RewardEntries = FFinalRewardResolver::MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);
		FFinalRewardResolver::PopulateRewardEventViewData(RewardEvent, DataRegistry);
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
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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
	Snapshot.PendingBattleReward.RewardGold = HasPendingBattleReward() ? CurrentState.LastBattleRewardGold : 0;
	Snapshot.PendingBattleReward.bCanClaim = HasPendingBattleReward();
	Snapshot.PendingBattleReward.RewardEntries = FFinalRewardResolver::MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);
	Snapshot.PendingBattleReward.RewardEntryViews = FFinalRewardResolver::BuildRewardEntryViews(Snapshot.PendingBattleReward.RewardEntries, DataRegistry);

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
	Snapshot.Progression.bCurrentNodeResolved = !CurrentNodeId.IsNone() && ResolvedNodeIds.Contains(CurrentNodeId);
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

	Snapshot.CurrentBuild = BuildCurrentBuildViewData(CurrentState, DataRegistry);
	Snapshot.PendingGrowthChoice.bHasPendingChoice = CurrentState.PendingGrowthChoice.bIsValid;
	Snapshot.PendingGrowthChoice.CharacterId = CurrentState.PendingGrowthChoice.CharacterId;
	Snapshot.PendingGrowthChoice.Choices = CurrentState.PendingGrowthChoice.Choices;
	Snapshot.Gold = CurrentState.Gold;
	Snapshot.RelicCount = CurrentState.Relics.Num();
	Snapshot.DeckCount = CurrentState.RunDeck.Num();
	Snapshot.LastBattleOutcome = CurrentState.LastBattleOutcome;
	Snapshot.LastResolvedEncounterId = CurrentState.LastResolvedEncounterId;
	Snapshot.LastBattleRewardGold = CurrentState.LastBattleRewardGold;

	for (const FFinalRunPersistentCharacterState& CharacterState : CurrentState.Characters)
	{
		Snapshot.Characters.Add(MakeCharacterView(CharacterState, DataRegistry));
	}

	return Snapshot;
}

FFinalRunState UFinalRunSession::GetRunState() const
{
	return CurrentState;
}

FFinalRunPersistentCharacterState* UFinalRunSession::FindMutableCharacterState(const FFinalCharacterId& CharacterId)
{
	return CurrentState.Characters.FindByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& CharacterState)
	{
		return CharacterState.CharacterId == CharacterId;
	});
}

const FFinalRunPersistentCharacterState* UFinalRunSession::FindCharacterState(const FFinalCharacterId& CharacterId) const
{
	return CurrentState.Characters.FindByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& CharacterState)
	{
		return CharacterState.CharacterId == CharacterId;
	});
}

bool UFinalRunSession::TryLevelUpCharacter(FFinalRunPersistentCharacterState& CharacterState)
{
	if (HasPendingGrowthChoice()
		|| CharacterState.BreakthroughRequiredValue <= 0
		|| CharacterState.BreakthroughValue < CharacterState.BreakthroughRequiredValue)
	{
		return false;
	}

	CharacterState.BreakthroughValue -= CharacterState.BreakthroughRequiredValue;
	CharacterState.Level = FMath::Max(1, CharacterState.Level + 1);
	if (!GenerateGrowthChoicesForCharacter(CharacterState))
	{
		CharacterState.bHasPendingGrowthChoice = false;
		CurrentState.PendingGrowthChoice.Reset();
		return false;
	}

	CharacterState.bHasPendingGrowthChoice = true;
	return true;
}

bool UFinalRunSession::GenerateGrowthChoicesForCharacter(const FFinalRunPersistentCharacterState& CharacterState)
{
	TArray<FFinalRunGrowthChoiceInstance> GeneratedChoices;
	GeneratedChoices.Reserve(3);
	GeneratedChoices.Add(MakeAttributeGrowthChoice(CharacterState, EFinalGrowthAttributeType::RootBone, 1));
	GeneratedChoices.Add(MakeAttributeGrowthChoice(CharacterState, EFinalGrowthAttributeType::Insight, 2));

	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	bool bAddedEvolutionChoice = false;
	if (DataRegistry != nullptr)
	{
		TArray<const UFinalCardEvolutionDefinition*> EvolutionDefinitions;
		DataRegistry->GetAllCardEvolutionDefinitions(EvolutionDefinitions);

		for (const UFinalCardEvolutionDefinition* EvolutionDefinition : EvolutionDefinitions)
		{
			if (EvolutionDefinition == nullptr)
			{
				continue;
			}

			for (const FFinalRunCardInstance& CardInstance : CurrentState.RunDeck)
			{
				if (IsEligibleEvolutionChoice(*EvolutionDefinition, CardInstance, CharacterState))
				{
					GeneratedChoices.Add(MakeEvolutionGrowthChoice(CharacterState, CardInstance, *EvolutionDefinition, 3, DataRegistry));
					bAddedEvolutionChoice = true;
					break;
				}
			}

			if (bAddedEvolutionChoice)
			{
				break;
			}
		}
	}

	if (!bAddedEvolutionChoice)
	{
		GeneratedChoices.Add(MakeAttributeGrowthChoice(CharacterState, EFinalGrowthAttributeType::KillingIntent, 3));
	}

	if (GeneratedChoices.IsEmpty())
	{
		return false;
	}

	CurrentState.PendingGrowthChoice.Reset();
	CurrentState.PendingGrowthChoice.bIsValid = true;
	CurrentState.PendingGrowthChoice.CharacterId = CharacterState.CharacterId;
	CurrentState.PendingGrowthChoice.Choices = MoveTemp(GeneratedChoices);
	return true;
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

FFinalRunSaveData UFinalRunSession::ExportSaveData() const
{
	FFinalRunSaveData SaveData;
	SaveData.SaveVersion = FFinalRunSaveData::CurrentSaveVersion;
	SaveData.RunState = CurrentState;
	SaveData.RunLogEntries = RunLogEntries;
	SaveData.LastEventSequence = LastEventSequence;
	SaveData.ConfiguredRunNodes = ConfiguredRunNodes;
	SaveData.CurrentNodeId = CurrentNodeId;
	SaveData.VisitedNodeIds = VisitedNodeIds.Array();
	SaveData.ResolvedNodeIds = ResolvedNodeIds.Array();
	SaveData.CurrentFlowStage = CurrentFlowStage;
	SaveData.PendingRewardSourceNodeId = PendingRewardSourceNodeId;
	SaveData.PendingRewardSourceEncounterId = PendingRewardSourceEncounterId;
	SaveData.PendingRewardBattleOutcome = PendingRewardBattleOutcome;
	SaveData.PendingRewardEntries = PendingRewardEntries;
	return SaveData;
}

bool UFinalRunSession::RestoreFromSaveData(const FFinalRunSaveData& SaveData, FText& OutFailureReason)
{
	if (!SaveData.IsStructurallyValid(&OutFailureReason))
	{
		return false;
	}

	CurrentState = SaveData.RunState;
	RunLogEntries = SaveData.RunLogEntries;
	ConfiguredRunNodes = SaveData.ConfiguredRunNodes;
	CurrentNodeId = SaveData.CurrentNodeId;
	CurrentFlowStage = SaveData.CurrentFlowStage;
	PendingRewardSourceNodeId = SaveData.PendingRewardSourceNodeId;
	PendingRewardSourceEncounterId = SaveData.PendingRewardSourceEncounterId;
	PendingRewardBattleOutcome = SaveData.PendingRewardBattleOutcome;
	PendingRewardEntries = SaveData.PendingRewardEntries;

	VisitedNodeIds.Reset();
	for (const FName& NodeId : SaveData.VisitedNodeIds)
	{
		if (!NodeId.IsNone())
		{
			VisitedNodeIds.Add(NodeId);
		}
	}

	ResolvedNodeIds.Reset();
	for (const FName& NodeId : SaveData.ResolvedNodeIds)
	{
		if (!NodeId.IsNone())
		{
			ResolvedNodeIds.Add(NodeId);
		}
	}

	int32 HighestRestoredEventSequence = 0;
	for (const FFinalRunEvent& Event : RunLogEntries)
	{
		HighestRestoredEventSequence = FMath::Max(HighestRestoredEventSequence, Event.EventSequence);
	}

	LastEventSequence = FMath::Max(SaveData.LastEventSequence, HighestRestoredEventSequence);
	OutFailureReason = FText::GetEmpty();
	return true;
}

bool UFinalRunSession::TryExecuteClaimPendingBattleReward(const FName& RewardId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	if (!HasPendingBattleReward())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPendingBattleReward;
		OutFailureMessage = FText::FromString(TEXT("There is no pending battle reward to claim."));
		return false;
	}

	if (RewardId.IsNone() && PendingRewardEntries.Num() != 1)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPayloadId;
		OutFailureMessage = FText::FromString(TEXT("ClaimPendingBattleReward requires a reward id when multiple pending rewards are available."));
		return false;
	}

	const FFinalRunRewardEntry* SelectedEntry = RewardId.IsNone()
		? &PendingRewardEntries[0]
		: PendingRewardEntries.FindByPredicate([&RewardId](const FFinalRunRewardEntry& Entry)
		{
			return Entry.RewardId == RewardId;
		});

	if (SelectedEntry == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownPendingBattleReward;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "UnknownPendingBattleReward", "Pending battle reward {0} is not available."),
			FText::FromName(RewardId));
		return false;
	}

	TArray<FFinalRunRewardEntry> SelectedEntries;
	SelectedEntries.Add(*SelectedEntry);

	if (!FFinalRewardResolver::ValidateRewardEntriesForApplication(SelectedEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	TArray<FFinalRunRewardEntry> ClaimedEntries = FFinalRewardResolver::MakeClaimedRewardEntries(SelectedEntries, DataRegistry);
	FFinalRewardResolver::ApplyValidatedRewardEntriesToRunState(SelectedEntries, CurrentState);

	SetFlowStageAfterPostBattleReward();

	OutDetailEvent.EventType = EFinalRunEventType::PendingBattleRewardClaimed;
	OutDetailEvent.NodeId = PendingRewardSourceNodeId;
	OutDetailEvent.SourceNodeId = PendingRewardSourceNodeId;
	OutDetailEvent.EncounterId = PendingRewardSourceEncounterId;
	OutDetailEvent.BattleOutcome = PendingRewardBattleOutcome;
	OutDetailEvent.RewardGold = CurrentState.LastBattleRewardGold;
	OutDetailEvent.RewardEntries = ClaimedEntries;
	FFinalRewardResolver::PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	FFinalRewardResolver::PopulateAffectedCharacterResults(OutDetailEvent, ClaimedEntries, CurrentState, DataRegistry);
	if (const FFinalRunNodeDefinition* SourceNode = FindNodeDefinition(PendingRewardSourceNodeId))
	{
		PopulateNodeEventMetadata(OutDetailEvent, *SourceNode);
	}
	OutDetailEvent.Message = FText::Format(
		NSLOCTEXT("FinalRunSession", "PendingBattleRewardClaimed", "Claimed pending battle reward {0}."),
		FText::FromName(SelectedEntry->RewardId));

	ClearPendingBattleReward();
	return true;
}

bool UFinalRunSession::TryExecuteSkipPendingBattleReward(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	if (!HasPendingBattleReward())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPendingBattleReward;
		OutFailureMessage = FText::FromString(TEXT("There is no pending battle reward to skip."));
		return false;
	}

	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	const TArray<FFinalRunRewardEntry> SkippedEntries = FFinalRewardResolver::MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);

	SetFlowStageAfterPostBattleReward();

	OutDetailEvent.EventType = EFinalRunEventType::PendingBattleRewardSkipped;
	OutDetailEvent.NodeId = PendingRewardSourceNodeId;
	OutDetailEvent.SourceNodeId = PendingRewardSourceNodeId;
	OutDetailEvent.EncounterId = PendingRewardSourceEncounterId;
	OutDetailEvent.BattleOutcome = PendingRewardBattleOutcome;
	OutDetailEvent.RewardGold = CurrentState.LastBattleRewardGold;
	OutDetailEvent.RewardEntries = SkippedEntries;
	FFinalRewardResolver::PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	if (const FFinalRunNodeDefinition* SourceNode = FindNodeDefinition(PendingRewardSourceNodeId))
	{
		PopulateNodeEventMetadata(OutDetailEvent, *SourceNode);
	}
	OutDetailEvent.Message = NSLOCTEXT("FinalRunSession", "PendingBattleRewardSkipped", "Skipped pending battle card reward.");

	ClearPendingBattleReward();
	return true;
}

bool UFinalRunSession::TryExecuteResolveRewardNode(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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

	const TArray<FFinalRunRewardEntry> PreviewEntries = FFinalRewardResolver::MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = FFinalRewardResolver::MakeClaimedRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry);
	if (!FFinalRewardResolver::ValidateRewardEntriesForApplication(PreviewEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	FFinalRewardResolver::ApplyValidatedRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::RewardNodeResolved;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.RewardGold = FFinalRewardResolver::GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	FFinalRewardResolver::PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	FFinalRewardResolver::PopulateAffectedCharacterResults(OutDetailEvent, ResolvedEntries, CurrentState, DataRegistry);
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
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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

	FFinalResolvedEventOptionResult ResolvedOption;
	if (!FFinalRunEventResolver::TryResolveEventOption(*CurrentNode, OptionId, DataRegistry, OutRejectReason, OutFailureMessage, ResolvedOption))
	{
		return false;
	}

	if (!FFinalRewardResolver::ValidateRewardEntriesForApplication(ResolvedOption.PreviewEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	FFinalRewardResolver::ApplyValidatedRewardEntriesToRunState(ResolvedOption.PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::EventNodeResolved;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.PayloadId = OptionId;
	OutDetailEvent.RewardGold = FFinalRewardResolver::GetRewardGoldTotal(ResolvedOption.ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedOption.ResolvedEntries;
	FFinalRewardResolver::PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	FFinalRewardResolver::PopulateAffectedCharacterResults(OutDetailEvent, ResolvedOption.ResolvedEntries, CurrentState, DataRegistry);
	PopulateNodeEventMetadata(OutDetailEvent, *CurrentNode);
	OutDetailEvent.Message = ResolvedOption.OptionDefinition->OutcomeSummary.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalRunSession", "EventNodeResolved", "Resolved event node {0} with option {1}."),
			OutDetailEvent.NodeDisplayName,
			FText::FromName(OptionId))
		: ResolvedOption.OptionDefinition->OutcomeSummary;
	return true;
}

bool UFinalRunSession::TryExecuteResolveShopNode(const FName& OfferId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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

	FFinalResolvedShopOfferResult ResolvedOffer;
	if (!FFinalShopResolver::TryResolveShopOffer(*CurrentNode, OfferId, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage, ResolvedOffer))
	{
		return false;
	}

	if (!FFinalRewardResolver::ValidateRewardEntriesForApplication(ResolvedOffer.PreviewEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	CurrentState.Gold -= ResolvedOffer.SpentGold;
	FFinalRewardResolver::ApplyValidatedRewardEntriesToRunState(ResolvedOffer.PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::ShopOfferPurchased;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.PayloadId = OfferId;
	OutDetailEvent.SpentGold = ResolvedOffer.SpentGold;
	OutDetailEvent.RewardGold = FFinalRewardResolver::GetRewardGoldTotal(ResolvedOffer.ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedOffer.ResolvedEntries;
	FFinalRewardResolver::PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	FFinalRewardResolver::PopulateAffectedCharacterResults(OutDetailEvent, ResolvedOffer.ResolvedEntries, CurrentState, DataRegistry);
	PopulateNodeEventMetadata(OutDetailEvent, *CurrentNode);
	OutDetailEvent.Message = ResolvedOffer.OfferDefinition->Description.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalRunSession", "ShopOfferPurchased", "Purchased shop offer {0}."),
			ResolvedOffer.OfferDefinition->DisplayName.IsEmpty() ? FText::FromName(OfferId) : ResolvedOffer.OfferDefinition->DisplayName)
		: ResolvedOffer.OfferDefinition->Description;
	return true;
}

bool UFinalRunSession::TryExecuteSelectGrowthChoice(const FName& ChoiceInstanceId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage)
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	if (!CurrentState.PendingGrowthChoice.bIsValid)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPendingGrowthChoice;
		OutFailureMessage = FText::FromString(TEXT("SelectGrowthChoice requires a pending growth choice."));
		return false;
	}

	if (ChoiceInstanceId.IsNone())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPayloadId;
		OutFailureMessage = FText::FromString(TEXT("SelectGrowthChoice requires a choice instance id payload."));
		return false;
	}

	const FFinalRunGrowthChoiceInstance* SelectedChoice = CurrentState.PendingGrowthChoice.Choices.FindByPredicate([&ChoiceInstanceId](const FFinalRunGrowthChoiceInstance& Choice)
	{
		return Choice.ChoiceInstanceId == ChoiceInstanceId;
	});
	if (SelectedChoice == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownGrowthChoice;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "UnknownGrowthChoice", "Growth choice {0} is not present in the current pending choice set."),
			FText::FromName(ChoiceInstanceId));
		return false;
	}

	if (SelectedChoice->CharacterId != CurrentState.PendingGrowthChoice.CharacterId)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownGrowthTargetCharacter;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "GrowthChoiceCharacterMismatch", "Growth choice {0} does not belong to the current pending character."),
			FText::FromName(ChoiceInstanceId));
		return false;
	}

	const FFinalRunGrowthChoiceInstance AppliedChoice = *SelectedChoice;
	if (!FFinalGrowthResolver::ValidateAndApplyGrowthChoice(AppliedChoice, CurrentState, DataRegistry, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	if (FFinalRunPersistentCharacterState* CharacterState = FindMutableCharacterState(AppliedChoice.CharacterId))
	{
		CharacterState->bHasPendingGrowthChoice = false;
	}

	CurrentState.PendingGrowthChoice.Reset();

	OutDetailEvent.EventType = EFinalRunEventType::GrowthChoiceApplied;
	OutDetailEvent.PayloadId = ChoiceInstanceId;
	OutDetailEvent.TargetCharacterId = AppliedChoice.CharacterId;
	TArray<FFinalCharacterId> AffectedCharacterIds;
	AffectedCharacterIds.Add(AppliedChoice.CharacterId);
	OutDetailEvent.AffectedCharacterResults = FFinalGrowthResolver::BuildAffectedCharacterResults(
		AffectedCharacterIds,
		CurrentState,
		DataRegistry);
	OutDetailEvent.Message = AppliedChoice.DisplayName.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalRunSession", "GrowthChoiceApplied", "Applied growth choice {0}."),
			FText::FromName(ChoiceInstanceId))
		: FText::Format(
			NSLOCTEXT("FinalRunSession", "GrowthChoiceAppliedDisplayName", "Applied growth choice: {0}."),
			AppliedChoice.DisplayName);
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
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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
		? FFinalRewardResolver::MakeClaimedRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry)
		: FFinalRewardResolver::MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry);
	View.RewardEntryViews = FFinalRewardResolver::BuildRewardEntryViews(View.RewardEntries, DataRegistry);
	return View;
}

FFinalRunPendingEventNodeViewData UFinalRunSession::BuildPendingEventNodeView() const
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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

	FFinalRunEventResolver::BuildEventOptionViews(*CurrentNode, DataRegistry, bResolved, View.Options, View.bCanResolve);

	return View;
}

FFinalRunPendingShopNodeViewData UFinalRunSession::BuildPendingShopNodeView() const
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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

	FFinalShopResolver::BuildShopOfferViews(*CurrentNode, DataRegistry, CurrentState, bResolved, View.Offers, View.bCanResolve);

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

void UFinalRunSession::ClearPendingBattleReward()
{
	PendingRewardSourceNodeId = NAME_None;
	PendingRewardSourceEncounterId = FFinalEncounterId{};
	PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	PendingRewardEntries.Reset();
}

void UFinalRunSession::SetFlowStageAfterPostBattleReward()
{
	if (const FFinalRunNodeDefinition* CurrentNode = FindNodeDefinition(CurrentNodeId))
	{
		CurrentFlowStage = CurrentNode->NextNodeIds.Num() > 0
			? EFinalRunFlowStage::AwaitingNodeAdvance
			: EFinalRunFlowStage::RunEnded;
		return;
	}

	CurrentFlowStage = EFinalRunFlowStage::RunEnded;
}

bool UFinalRunSession::HasPendingBattleReward() const
{
	return PendingRewardEntries.Num() > 0;
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


