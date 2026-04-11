#include "Facade/FinalRunSession.h"

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"
#include "Run/Definitions/FinalRelicDefinition.h"
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

int32 GetRewardEntryApplicationCount(const FFinalRunRewardEntry& RewardEntry)
{
	return RewardEntry.Value > 0 ? RewardEntry.Value : 1;
}

const UFinalCardDefinition* FindRewardCardDefinition(const FFinalCardId& CardId, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry == nullptr || !CardId.IsValid())
	{
		return nullptr;
	}

	return DataRegistry->FindCardDefinition(CardId);
}

const UFinalRelicDefinition* FindRewardEntryRelicDefinition(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry == nullptr || !RewardEntry.GrantedRelicId.IsValid())
	{
		return nullptr;
	}

	return DataRegistry->FindRelicDefinition(RewardEntry.GrantedRelicId);
}

const UFinalCardDefinition* FindRewardEntryDisplayCardDefinition(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::CardGrant:
		return FindRewardCardDefinition(RewardEntry.GrantedCardId, DataRegistry);

	case EFinalRunRewardType::RemoveCard:
		return FindRewardCardDefinition(RewardEntry.RemovedCardId, DataRegistry);

	case EFinalRunRewardType::UpgradeCard:
		if (const UFinalCardDefinition* UpgradedDefinition = FindRewardCardDefinition(RewardEntry.UpgradeToCardId, DataRegistry))
		{
			return UpgradedDefinition;
		}

		return FindRewardCardDefinition(RewardEntry.UpgradeFromCardId, DataRegistry);

	default:
		return nullptr;
	}
}

FName GetRewardEntryDefaultCardDisplayId(const FFinalRunRewardEntry& RewardEntry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::CardGrant:
		return RewardEntry.GrantedCardId.IsValid() ? RewardEntry.GrantedCardId.Value : NAME_None;

	case EFinalRunRewardType::RemoveCard:
		return RewardEntry.RemovedCardId.IsValid() ? RewardEntry.RemovedCardId.Value : NAME_None;

	case EFinalRunRewardType::UpgradeCard:
		if (RewardEntry.UpgradeToCardId.IsValid())
		{
			return RewardEntry.UpgradeToCardId.Value;
		}

		return RewardEntry.UpgradeFromCardId.IsValid() ? RewardEntry.UpgradeFromCardId.Value : NAME_None;

	case EFinalRunRewardType::Growth:
		return RewardEntry.GrowthTargetCharacterId.IsValid() ? RewardEntry.GrowthTargetCharacterId.Value : NAME_None;

	default:
		return NAME_None;
	}
}

FText GetRewardEntryDefaultCardDisplayName(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	if (const UFinalCardDefinition* CardDefinition = FindRewardEntryDisplayCardDefinition(RewardEntry, DataRegistry))
	{
		if (!CardDefinition->DisplayName.IsEmpty())
		{
			return CardDefinition->DisplayName;
		}
	}

	const FName CardDisplayId = GetRewardEntryDefaultCardDisplayId(RewardEntry);
	return CardDisplayId.IsNone()
		? GetDefaultRewardDisplayName(RewardEntry.RewardType)
		: FText::FromName(CardDisplayId);
}

int32 CountRunDeckCards(const TArray<FFinalCardId>& RunDeck, const FFinalCardId& CardId)
{
	int32 Count = 0;
	for (const FFinalCardId& DeckCardId : RunDeck)
	{
		if (DeckCardId == CardId)
		{
			++Count;
		}
	}

	return Count;
}

int32 FindRunDeckCardIndex(const TArray<FFinalCardId>& RunDeck, const FFinalCardId& CardId)
{
	return RunDeck.IndexOfByPredicate([&CardId](const FFinalCardId& DeckCardId)
	{
		return DeckCardId == CardId;
	});
}

int32 FindRunCharacterIndex(const TArray<FFinalRunPersistentCharacterState>& Characters, const FFinalCharacterId& CharacterId)
{
	return Characters.IndexOfByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& CharacterState)
	{
		return CharacterState.CharacterId == CharacterId;
	});
}

FName GetRewardEntryDefaultDisplayId(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::CardGrant:
	case EFinalRunRewardType::RemoveCard:
	case EFinalRunRewardType::UpgradeCard:
		return GetRewardEntryDefaultCardDisplayId(RewardEntry);

	case EFinalRunRewardType::RelicGrant:
		if (const UFinalRelicDefinition* RelicDefinition = FindRewardEntryRelicDefinition(RewardEntry, DataRegistry))
		{
			if (!RelicDefinition->DisplayId.IsNone())
			{
				return RelicDefinition->DisplayId;
			}
		}

		return RewardEntry.GrantedRelicId.IsValid() ? RewardEntry.GrantedRelicId.Value : NAME_None;

	default:
		return NAME_None;
	}
}

FText GetRewardEntryDefaultDisplayName(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::CardGrant:
	case EFinalRunRewardType::RemoveCard:
	case EFinalRunRewardType::UpgradeCard:
		return GetRewardEntryDefaultCardDisplayName(RewardEntry, DataRegistry);

	case EFinalRunRewardType::RelicGrant:
		if (const UFinalRelicDefinition* RelicDefinition = FindRewardEntryRelicDefinition(RewardEntry, DataRegistry))
		{
			if (!RelicDefinition->DisplayName.IsEmpty())
			{
				return RelicDefinition->DisplayName;
			}
		}

		return RewardEntry.GrantedRelicId.IsValid()
			? FText::FromName(RewardEntry.GrantedRelicId.Value)
			: GetDefaultRewardDisplayName(RewardEntry.RewardType);

	case EFinalRunRewardType::Growth:
		return RewardEntry.GrowthTargetCharacterId.IsValid()
			? FText::FromName(RewardEntry.GrowthTargetCharacterId.Value)
			: GetDefaultRewardDisplayName(RewardEntry.RewardType);

	default:
		return GetDefaultRewardDisplayName(RewardEntry.RewardType);
	}
}

void NormalizeRewardEntries(TArray<FFinalRunRewardEntry>& RewardEntries, const bool bClaimable, const UFinalDataRegistry* DataRegistry)
{
	for (FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (Entry.DisplayId.IsNone())
		{
			Entry.DisplayId = GetRewardEntryDefaultDisplayId(Entry, DataRegistry);
		}

		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = GetRewardEntryDefaultDisplayName(Entry, DataRegistry);
		}

		Entry.bCanClaim = bClaimable;
		Entry.bClaimed = !bClaimable;
	}
}

TArray<FFinalRunRewardEntry> MakeClaimedRewardEntries(const TArray<FFinalRunRewardEntry>& RewardEntries, const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntry> ClaimedEntries = RewardEntries;
	NormalizeRewardEntries(ClaimedEntries, false, DataRegistry);
	return ClaimedEntries;
}

TArray<FFinalRunRewardEntry> MakePreviewRewardEntries(const TArray<FFinalRunRewardEntry>& RewardEntries, const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntry> PreviewEntries = RewardEntries;
	NormalizeRewardEntries(PreviewEntries, true, DataRegistry);
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

FText ResolveRewardCharacterDisplayName(const FFinalCharacterId& CharacterId, const UFinalDataRegistry* DataRegistry)
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
		: NSLOCTEXT("FinalRunSession", "UnknownGrowthCharacter", "Unknown Character");
}

FText GetRewardEntryResolvedDisplayName(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	if (!RewardEntry.DisplayName.IsEmpty())
	{
		return RewardEntry.DisplayName;
	}

	return GetRewardEntryDefaultDisplayName(RewardEntry, DataRegistry);
}

FText GetGrowthEffectDisplayName(const EFinalRunGrowthEffectType GrowthEffectType)
{
	switch (GrowthEffectType)
	{
	case EFinalRunGrowthEffectType::ReduceStress:
		return NSLOCTEXT("FinalRunSession", "GrowthEffectReduceStress", "Reduce Stress");

	case EFinalRunGrowthEffectType::GainAwakenProgress:
		return NSLOCTEXT("FinalRunSession", "GrowthEffectGainAwakenProgress", "Gain Awaken Progress");

	case EFinalRunGrowthEffectType::ReduceCollapseCount:
		return NSLOCTEXT("FinalRunSession", "GrowthEffectReduceCollapseCount", "Reduce Collapse Count");

	default:
		return NSLOCTEXT("FinalRunSession", "GrowthEffectUnknown", "Growth");
	}
}

EFinalRunRewardPresentationKind GetRewardPresentationKind(const EFinalRunRewardType RewardType)
{
	switch (RewardType)
	{
	case EFinalRunRewardType::Gold:
		return EFinalRunRewardPresentationKind::Gold;

	case EFinalRunRewardType::CardGrant:
		return EFinalRunRewardPresentationKind::Card;

	case EFinalRunRewardType::RelicGrant:
		return EFinalRunRewardPresentationKind::Relic;

	case EFinalRunRewardType::RemoveCard:
	case EFinalRunRewardType::UpgradeCard:
		return EFinalRunRewardPresentationKind::DeckEdit;

	case EFinalRunRewardType::Growth:
		return EFinalRunRewardPresentationKind::Growth;

	default:
		return EFinalRunRewardPresentationKind::None;
	}
}

EFinalRunRewardVisualTier MapRarityToRewardVisualTier(const EFinalRarity Rarity)
{
	switch (Rarity)
	{
	case EFinalRarity::Common:
		return EFinalRunRewardVisualTier::Common;

	case EFinalRarity::Rare:
		return EFinalRunRewardVisualTier::Rare;

	case EFinalRarity::Epic:
		return EFinalRunRewardVisualTier::Epic;

	case EFinalRarity::Legendary:
		return EFinalRunRewardVisualTier::Legendary;

	default:
		return EFinalRunRewardVisualTier::None;
	}
}

FText BuildRelicEffectFallbackDetailText(const UFinalRelicDefinition* RelicDefinition)
{
	if (RelicDefinition == nullptr)
	{
		return FText::GetEmpty();
	}

	FString Summary;
	auto AppendSummaryPart = [&Summary](const FText& Part)
	{
		if (Part.IsEmpty())
		{
			return;
		}

		if (!Summary.IsEmpty())
		{
			Summary += TEXT("; ");
		}

		Summary += Part.ToString();
	};

	for (const FFinalRelicBattleStartEffectDefinition& EffectDefinition : RelicDefinition->BattleStartEffects)
	{
		switch (EffectDefinition.EffectType)
		{
		case EFinalRelicBattleStartEffectType::GainAP:
			AppendSummaryPart(FText::Format(
				NSLOCTEXT("FinalRunSession", "RelicBattleStartGainAPDetail", "Battle start: +{0} AP"),
				FText::AsNumber(EffectDefinition.Value)));
			break;

		case EFinalRelicBattleStartEffectType::GainShield:
			AppendSummaryPart(FText::Format(
				NSLOCTEXT("FinalRunSession", "RelicBattleStartGainShieldDetail", "Battle start: +{0} Shield"),
				FText::AsNumber(EffectDefinition.Value)));
			break;

		default:
			break;
		}
	}

	for (const FFinalRelicPlayerTurnStartEffectDefinition& EffectDefinition : RelicDefinition->PlayerTurnStartEffects)
	{
		switch (EffectDefinition.EffectType)
		{
		case EFinalRelicPlayerTurnStartEffectType::GainAP:
			AppendSummaryPart(FText::Format(
				NSLOCTEXT("FinalRunSession", "RelicPlayerTurnStartGainAPDetail", "Player turn start: +{0} AP"),
				FText::AsNumber(EffectDefinition.Value)));
			break;

		case EFinalRelicPlayerTurnStartEffectType::GainShield:
			AppendSummaryPart(FText::Format(
				NSLOCTEXT("FinalRunSession", "RelicPlayerTurnStartGainShieldDetail", "Player turn start: +{0} Shield"),
				FText::AsNumber(EffectDefinition.Value)));
			break;

		default:
			break;
		}
	}

	return Summary.IsEmpty() ? FText::GetEmpty() : FText::FromString(Summary);
}

FName BuildRewardEntryIconId(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return TEXT("Currency.Gold");

	case EFinalRunRewardType::CardGrant:
		return RewardEntry.GrantedCardId.IsValid() ? RewardEntry.GrantedCardId.Value : NAME_None;

	case EFinalRunRewardType::RelicGrant:
		return ResolveRelicEntryDisplayId(RewardEntry.GrantedRelicId, DataRegistry);

	case EFinalRunRewardType::RemoveCard:
		return RewardEntry.RemovedCardId.IsValid() ? RewardEntry.RemovedCardId.Value : NAME_None;

	case EFinalRunRewardType::UpgradeCard:
		if (RewardEntry.UpgradeToCardId.IsValid())
		{
			return RewardEntry.UpgradeToCardId.Value;
		}

		return RewardEntry.UpgradeFromCardId.IsValid() ? RewardEntry.UpgradeFromCardId.Value : NAME_None;

	case EFinalRunRewardType::Growth:
		return RewardEntry.GrowthTargetCharacterId.IsValid() ? RewardEntry.GrowthTargetCharacterId.Value : NAME_None;

	default:
		return RewardEntry.DisplayId;
	}
}

EFinalRunRewardVisualTier ResolveRewardEntryVisualTier(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return EFinalRunRewardVisualTier::Currency;

	case EFinalRunRewardType::CardGrant:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.GrantedCardId, DataRegistry))
		{
			return MapRarityToRewardVisualTier(CardDefinition->Rarity);
		}
		return EFinalRunRewardVisualTier::Common;

	case EFinalRunRewardType::RelicGrant:
		if (const UFinalRelicDefinition* RelicDefinition = FindRewardEntryRelicDefinition(RewardEntry, DataRegistry))
		{
			return MapRarityToRewardVisualTier(RelicDefinition->Rarity);
		}
		return EFinalRunRewardVisualTier::Rare;

	case EFinalRunRewardType::RemoveCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.RemovedCardId, DataRegistry))
		{
			return MapRarityToRewardVisualTier(CardDefinition->Rarity);
		}
		return EFinalRunRewardVisualTier::Utility;

	case EFinalRunRewardType::UpgradeCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.UpgradeToCardId, DataRegistry))
		{
			return MapRarityToRewardVisualTier(CardDefinition->Rarity);
		}

		if (const UFinalCardDefinition* SourceDefinition = FindRewardCardDefinition(RewardEntry.UpgradeFromCardId, DataRegistry))
		{
			return MapRarityToRewardVisualTier(SourceDefinition->Rarity);
		}
		return EFinalRunRewardVisualTier::Utility;

	case EFinalRunRewardType::Growth:
		return EFinalRunRewardVisualTier::Progression;

	default:
		return EFinalRunRewardVisualTier::None;
	}
}

FText BuildRewardEntryDetailText(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalRunSession", "RewardViewGoldDetail", "Spend gold on shop offers and other run rewards.");

	case EFinalRunRewardType::CardGrant:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.GrantedCardId, DataRegistry))
		{
			if (!CardDefinition->RulesText.IsEmpty())
			{
				return CardDefinition->RulesText;
			}
		}

		return NSLOCTEXT("FinalRunSession", "RewardViewCardGrantDetailFallback", "Add this card to the current deck.");

	case EFinalRunRewardType::RelicGrant:
		if (const UFinalRelicDefinition* RelicDefinition = FindRewardEntryRelicDefinition(RewardEntry, DataRegistry))
		{
			if (!RelicDefinition->Description.IsEmpty())
			{
				return RelicDefinition->Description;
			}

			const FText FallbackDetail = BuildRelicEffectFallbackDetailText(RelicDefinition);
			if (!FallbackDetail.IsEmpty())
			{
				return FallbackDetail;
			}
		}

		return NSLOCTEXT("FinalRunSession", "RewardViewRelicGrantDetailFallback", "Gain this relic for the rest of the run.");

	case EFinalRunRewardType::RemoveCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.RemovedCardId, DataRegistry))
		{
			if (!CardDefinition->RulesText.IsEmpty())
			{
				return CardDefinition->RulesText;
			}
		}

		return NSLOCTEXT("FinalRunSession", "RewardViewRemoveCardDetailFallback", "Remove this card from the current deck.");

	case EFinalRunRewardType::UpgradeCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.UpgradeToCardId, DataRegistry))
		{
			if (!CardDefinition->RulesText.IsEmpty())
			{
				return CardDefinition->RulesText;
			}
		}

		return FText::Format(
			NSLOCTEXT("FinalRunSession", "RewardViewUpgradeCardDetailFallback", "Replace {0} with {1} in the current deck."),
			ResolveDeckEntryDisplayName(RewardEntry.UpgradeFromCardId, DataRegistry),
			ResolveDeckEntryDisplayName(RewardEntry.UpgradeToCardId, DataRegistry));

	case EFinalRunRewardType::Growth:
		return FText::Format(
			NSLOCTEXT("FinalRunSession", "RewardViewGrowthDetail", "Apply {0} to {1}."),
			GetGrowthEffectDisplayName(RewardEntry.GrowthEffectType),
			ResolveRewardCharacterDisplayName(RewardEntry.GrowthTargetCharacterId, DataRegistry));

	default:
		return FText::GetEmpty();
	}
}

FText BuildRewardEntryPrimaryText(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return GetRewardEntryResolvedDisplayName(RewardEntry, DataRegistry);

	case EFinalRunRewardType::CardGrant:
		return ResolveDeckEntryDisplayName(RewardEntry.GrantedCardId, DataRegistry);

	case EFinalRunRewardType::RelicGrant:
		return ResolveRelicEntryDisplayName(RewardEntry.GrantedRelicId, DataRegistry);

	case EFinalRunRewardType::RemoveCard:
		return ResolveDeckEntryDisplayName(RewardEntry.RemovedCardId, DataRegistry);

	case EFinalRunRewardType::UpgradeCard:
		return ResolveDeckEntryDisplayName(RewardEntry.UpgradeFromCardId, DataRegistry);

	case EFinalRunRewardType::Growth:
		return ResolveRewardCharacterDisplayName(RewardEntry.GrowthTargetCharacterId, DataRegistry);

	default:
		return GetRewardEntryResolvedDisplayName(RewardEntry, DataRegistry);
	}
}

FText BuildRewardEntrySecondaryText(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	const int32 ApplicationCount = GetRewardEntryApplicationCount(RewardEntry);

	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return FText::Format(
			NSLOCTEXT("FinalRunSession", "RewardViewGoldSecondary", "+{0}"),
			FText::AsNumber(RewardEntry.Value));

	case EFinalRunRewardType::CardGrant:
		return ApplicationCount > 1
			? FText::Format(
				NSLOCTEXT("FinalRunSession", "RewardViewCardGrantSecondaryPlural", "Add {0} copies to deck"),
				FText::AsNumber(ApplicationCount))
			: NSLOCTEXT("FinalRunSession", "RewardViewCardGrantSecondarySingular", "Add to deck");

	case EFinalRunRewardType::RelicGrant:
		return ApplicationCount > 1
			? FText::Format(
				NSLOCTEXT("FinalRunSession", "RewardViewRelicGrantSecondaryPlural", "Gain {0} relic copies"),
				FText::AsNumber(ApplicationCount))
			: NSLOCTEXT("FinalRunSession", "RewardViewRelicGrantSecondarySingular", "Gain relic");

	case EFinalRunRewardType::RemoveCard:
		return ApplicationCount > 1
			? FText::Format(
				NSLOCTEXT("FinalRunSession", "RewardViewRemoveCardSecondaryPlural", "Remove {0} copies from deck"),
				FText::AsNumber(ApplicationCount))
			: NSLOCTEXT("FinalRunSession", "RewardViewRemoveCardSecondarySingular", "Remove from deck");

	case EFinalRunRewardType::UpgradeCard:
		return FText::Format(
			NSLOCTEXT("FinalRunSession", "RewardViewUpgradeCardSecondary", "Upgrade to {0}"),
			ResolveDeckEntryDisplayName(RewardEntry.UpgradeToCardId, DataRegistry));

	case EFinalRunRewardType::Growth:
		return FText::Format(
			NSLOCTEXT("FinalRunSession", "RewardViewGrowthSecondary", "{0} +{1}"),
			GetGrowthEffectDisplayName(RewardEntry.GrowthEffectType),
			FText::AsNumber(RewardEntry.Value));

	default:
		return FText::GetEmpty();
	}
}

FFinalRunRewardEntryViewData BuildRewardEntryViewData(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	FFinalRunRewardEntryViewData ViewData;
	ViewData.RewardType = RewardEntry.RewardType;
	ViewData.PrimaryText = BuildRewardEntryPrimaryText(RewardEntry, DataRegistry);
	ViewData.SecondaryText = BuildRewardEntrySecondaryText(RewardEntry, DataRegistry);
	ViewData.Value = RewardEntry.Value;
	ViewData.PresentationKind = GetRewardPresentationKind(RewardEntry.RewardType);
	ViewData.IconId = BuildRewardEntryIconId(RewardEntry, DataRegistry);
	ViewData.VisualTier = ResolveRewardEntryVisualTier(RewardEntry, DataRegistry);
	ViewData.DetailText = BuildRewardEntryDetailText(RewardEntry, DataRegistry);

	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::CardGrant:
		ViewData.CardId = RewardEntry.GrantedCardId;
		break;

	case EFinalRunRewardType::RelicGrant:
		ViewData.RelicId = RewardEntry.GrantedRelicId;
		break;

	case EFinalRunRewardType::RemoveCard:
		ViewData.CardId = RewardEntry.RemovedCardId;
		break;

	case EFinalRunRewardType::UpgradeCard:
		ViewData.SourceCardId = RewardEntry.UpgradeFromCardId;
		ViewData.ResultCardId = RewardEntry.UpgradeToCardId;
		break;

	case EFinalRunRewardType::Growth:
		ViewData.TargetCharacterId = RewardEntry.GrowthTargetCharacterId;
		break;

	default:
		break;
	}

	return ViewData;
}

TArray<FFinalRunRewardEntryViewData> BuildRewardEntryViews(const TArray<FFinalRunRewardEntry>& RewardEntries, const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntryViewData> ViewDataEntries;
	ViewDataEntries.Reserve(RewardEntries.Num());

	for (const FFinalRunRewardEntry& RewardEntry : RewardEntries)
	{
		ViewDataEntries.Add(BuildRewardEntryViewData(RewardEntry, DataRegistry));
	}

	return ViewDataEntries;
}

void PopulateRewardEventViewData(FFinalRunEvent& Event, const UFinalDataRegistry* DataRegistry)
{
	Event.RewardEntryViews = BuildRewardEntryViews(Event.RewardEntries, DataRegistry);
}

bool RewardEntriesContainGrowth(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (Entry.RewardType == EFinalRunRewardType::Growth && Entry.IsClaimable())
		{
			return true;
		}
	}

	return false;
}

void PopulateAffectedCharacterResults(FFinalRunEvent& Event, const TArray<FFinalRunRewardEntry>& AppliedRewardEntries, const FFinalRunState& RunState, const UFinalDataRegistry* DataRegistry)
{
	if (!RewardEntriesContainGrowth(AppliedRewardEntries))
	{
		return;
	}

	TSet<FName> AffectedCharacterIds;
	for (const FFinalRunRewardEntry& Entry : AppliedRewardEntries)
	{
		if (Entry.RewardType == EFinalRunRewardType::Growth && Entry.IsClaimable() && Entry.GrowthTargetCharacterId.IsValid())
		{
			AffectedCharacterIds.Add(Entry.GrowthTargetCharacterId.Value);
		}
	}

	for (const FFinalRunPersistentCharacterState& CharacterState : RunState.Characters)
	{
		if (AffectedCharacterIds.Contains(CharacterState.CharacterId.Value))
		{
			Event.AffectedCharacterResults.Add(MakeCharacterView(CharacterState, DataRegistry));
		}
	}
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

		if (RelicInput.BattleStartEffects.Num() > 0 || RelicInput.PlayerTurnStartEffects.Num() > 0)
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
	for (const FFinalCardId& CardId : RunState.RunDeck)
	{
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

void ApplyValidatedRewardEntriesToRunState(const TArray<FFinalRunRewardEntry>& RewardEntries, FFinalRunState& RunState);

bool ValidateRewardEntryForApplication(
	const FFinalRunRewardEntry& RewardEntry,
	const UFinalDataRegistry* DataRegistry,
	const FFinalRunState& RunState,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return true;

	case EFinalRunRewardType::CardGrant:
		if (!RewardEntry.GrantedCardId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingGrantedCardId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "MissingGrantedCardId", "Reward entry {0} is missing GrantedCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		if (DataRegistry == nullptr || DataRegistry->FindCardDefinition(RewardEntry.GrantedCardId) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "RewardCardDefinitionUnavailable", "Card reward {0} cannot be applied because card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.GrantedCardId.Value));
			return false;
		}

		return true;

	case EFinalRunRewardType::RelicGrant:
		if (!RewardEntry.GrantedRelicId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingGrantedRelicId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "MissingGrantedRelicId", "Reward entry {0} is missing GrantedRelicId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		if (DataRegistry == nullptr || DataRegistry->FindRelicDefinition(RewardEntry.GrantedRelicId) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardRelicDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "RewardRelicDefinitionUnavailable", "Relic reward {0} cannot be applied because relic definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.GrantedRelicId.Value));
			return false;
		}

		return true;

	case EFinalRunRewardType::RemoveCard:
		if (!RewardEntry.RemovedCardId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingRemovedCardId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "MissingRemovedCardId", "Reward entry {0} is missing RemovedCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		if (DataRegistry == nullptr || FindRewardCardDefinition(RewardEntry.RemovedCardId, DataRegistry) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "RemoveCardDefinitionUnavailable", "Remove-card reward {0} cannot be applied because card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.RemovedCardId.Value));
			return false;
		}

		if (CountRunDeckCards(RunState.RunDeck, RewardEntry.RemovedCardId) < GetRewardEntryApplicationCount(RewardEntry))
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardTargetCardNotInRunDeck;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "RemoveCardTargetMissing", "Remove-card reward {0} needs {1} copies of {2} in the RunDeck, but only {3} are available."),
				FText::FromName(RewardEntry.RewardId),
				FText::AsNumber(GetRewardEntryApplicationCount(RewardEntry)),
				FText::FromName(RewardEntry.RemovedCardId.Value),
				FText::AsNumber(CountRunDeckCards(RunState.RunDeck, RewardEntry.RemovedCardId)));
			return false;
		}

		return true;

	case EFinalRunRewardType::UpgradeCard:
		if (!RewardEntry.UpgradeFromCardId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingUpgradeFromCardId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "MissingUpgradeFromCardId", "Reward entry {0} is missing UpgradeFromCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		if (!RewardEntry.UpgradeToCardId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingUpgradeToCardId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "MissingUpgradeToCardId", "Reward entry {0} is missing UpgradeToCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		if (RewardEntry.UpgradeFromCardId == RewardEntry.UpgradeToCardId)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardUpgradeResultInvalid;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "UpgradeResultSameAsSource", "Upgrade reward {0} is invalid because UpgradeToCardId matches UpgradeFromCardId {1}."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.UpgradeFromCardId.Value));
			return false;
		}

		if (DataRegistry == nullptr || FindRewardCardDefinition(RewardEntry.UpgradeFromCardId, DataRegistry) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "UpgradeSourceDefinitionUnavailable", "Upgrade reward {0} cannot be applied because source card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.UpgradeFromCardId.Value));
			return false;
		}

		if (DataRegistry == nullptr || FindRewardCardDefinition(RewardEntry.UpgradeToCardId, DataRegistry) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "UpgradeResultDefinitionUnavailable", "Upgrade reward {0} cannot be applied because upgraded card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.UpgradeToCardId.Value));
			return false;
		}

		if (CountRunDeckCards(RunState.RunDeck, RewardEntry.UpgradeFromCardId) < GetRewardEntryApplicationCount(RewardEntry))
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardTargetCardNotInRunDeck;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "UpgradeSourceMissingFromDeck", "Upgrade reward {0} needs {1} copies of {2} in the RunDeck, but only {3} are available."),
				FText::FromName(RewardEntry.RewardId),
				FText::AsNumber(GetRewardEntryApplicationCount(RewardEntry)),
				FText::FromName(RewardEntry.UpgradeFromCardId.Value),
				FText::AsNumber(CountRunDeckCards(RunState.RunDeck, RewardEntry.UpgradeFromCardId)));
			return false;
		}

		return true;

	case EFinalRunRewardType::Growth:
		if (!RewardEntry.GrowthTargetCharacterId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingGrowthTargetCharacterId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "MissingGrowthTargetCharacterId", "Growth reward {0} is missing GrowthTargetCharacterId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		if (FindRunCharacterIndex(RunState.Characters, RewardEntry.GrowthTargetCharacterId) == INDEX_NONE)
		{
			OutRejectReason = EFinalRunCommandRejectReason::UnknownGrowthTargetCharacter;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "UnknownGrowthTargetCharacter", "Growth reward {0} targets character {1}, but that character is not present in the current Run state."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.GrowthTargetCharacterId.Value));
			return false;
		}

		if (RewardEntry.Value <= 0)
		{
			OutRejectReason = EFinalRunCommandRejectReason::InvalidGrowthValue;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "InvalidGrowthValue", "Growth reward {0} is invalid because Value must be greater than zero."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}

		switch (RewardEntry.GrowthEffectType)
		{
		case EFinalRunGrowthEffectType::ReduceStress:
		case EFinalRunGrowthEffectType::GainAwakenProgress:
		case EFinalRunGrowthEffectType::ReduceCollapseCount:
			return true;

		case EFinalRunGrowthEffectType::None:
		default:
			OutRejectReason = EFinalRunCommandRejectReason::UnsupportedGrowthEffectType;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRunSession", "UnsupportedGrowthEffectType", "Growth reward {0} uses unsupported GrowthEffectType {1}."),
				FText::FromName(RewardEntry.RewardId),
				FText::AsNumber(static_cast<int32>(RewardEntry.GrowthEffectType)));
			return false;
		}

	case EFinalRunRewardType::None:
	default:
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedRewardType;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunSession", "UnsupportedRewardType", "Reward entry {0} uses reward type {1}, which does not yet have RunState landing support."),
			FText::FromName(RewardEntry.RewardId),
			FText::AsNumber(static_cast<int32>(RewardEntry.RewardType)));
		return false;
	}
}

bool ValidateRewardEntriesForApplication(
	const TArray<FFinalRunRewardEntry>& RewardEntries,
	const UFinalDataRegistry* DataRegistry,
	const FFinalRunState& RunState,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage)
{
	FFinalRunState SimulatedRunState = RunState;

	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (!Entry.IsClaimable())
		{
			continue;
		}

		if (!ValidateRewardEntryForApplication(Entry, DataRegistry, SimulatedRunState, OutRejectReason, OutFailureMessage))
		{
			return false;
		}

		TArray<FFinalRunRewardEntry> SingleRewardEntry;
		SingleRewardEntry.Add(Entry);
		ApplyValidatedRewardEntriesToRunState(SingleRewardEntry, SimulatedRunState);
	}

	return true;
}

void ApplyValidatedRewardEntriesToRunState(const TArray<FFinalRunRewardEntry>& RewardEntries, FFinalRunState& RunState)
{
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (!Entry.IsClaimable())
		{
			continue;
		}

		switch (Entry.RewardType)
		{
		case EFinalRunRewardType::Gold:
			RunState.Gold += Entry.Value;
			break;

		case EFinalRunRewardType::CardGrant:
			for (int32 GrantIndex = 0; GrantIndex < GetRewardEntryApplicationCount(Entry); ++GrantIndex)
			{
				RunState.RunDeck.Add(Entry.GrantedCardId);
			}
			break;

		case EFinalRunRewardType::RelicGrant:
			for (int32 GrantIndex = 0; GrantIndex < GetRewardEntryApplicationCount(Entry); ++GrantIndex)
			{
				RunState.Relics.Add(Entry.GrantedRelicId);
			}
			break;

		case EFinalRunRewardType::RemoveCard:
			for (int32 RemovalIndex = 0; RemovalIndex < GetRewardEntryApplicationCount(Entry); ++RemovalIndex)
			{
				const int32 RunDeckIndex = FindRunDeckCardIndex(RunState.RunDeck, Entry.RemovedCardId);
				if (RunDeckIndex != INDEX_NONE)
				{
					RunState.RunDeck.RemoveAt(RunDeckIndex);
				}
			}
			break;

		case EFinalRunRewardType::UpgradeCard:
			for (int32 UpgradeIndex = 0; UpgradeIndex < GetRewardEntryApplicationCount(Entry); ++UpgradeIndex)
			{
				const int32 RunDeckIndex = FindRunDeckCardIndex(RunState.RunDeck, Entry.UpgradeFromCardId);
				if (RunDeckIndex != INDEX_NONE)
				{
					RunState.RunDeck[RunDeckIndex] = Entry.UpgradeToCardId;
				}
			}
			break;

		case EFinalRunRewardType::Growth:
		{
			const int32 CharacterIndex = FindRunCharacterIndex(RunState.Characters, Entry.GrowthTargetCharacterId);
			if (CharacterIndex == INDEX_NONE)
			{
				break;
			}

			FFinalRunPersistentCharacterState& CharacterState = RunState.Characters[CharacterIndex];
			switch (Entry.GrowthEffectType)
			{
			case EFinalRunGrowthEffectType::ReduceStress:
				CharacterState.CurrentStress = FMath::Max(0, CharacterState.CurrentStress - Entry.Value);
				break;

			case EFinalRunGrowthEffectType::GainAwakenProgress:
				CharacterState.CurrentAwakenCount += Entry.Value;
				break;

			case EFinalRunGrowthEffectType::ReduceCollapseCount:
				CharacterState.CollapseCount = FMath::Max(0, CharacterState.CollapseCount - Entry.Value);
				break;

			default:
				break;
			}
			break;
		}

		default:
			break;
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
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	Request.EncounterId = CurrentState.CurrentEncounterId;
	Request.RuleConfigId = CurrentState.CurrentRuleConfigId;
	Request.TeamCurrentHP = CurrentState.TeamCurrentHP;
	Request.DeckCardIds = CurrentState.RunDeck;
	Request.PartyStates = CurrentState.Characters;
	Request.BattleStartRelics = BuildBattleStartRelicInputs(CurrentState, DataRegistry);

	for (const FFinalRunPersistentCharacterState& CharacterState : CurrentState.Characters)
	{
		Request.PartyCharacterIds.Add(CharacterState.CharacterId);
	}

	return Request;
}

void UFinalRunSession::ApplyBattleResult(const FFinalBattleResult& Result)
{
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
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
	Event.RewardEntries = MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);
	PopulateRewardEventViewData(Event, DataRegistry);
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
		RewardEvent.RewardEntries = MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);
		PopulateRewardEventViewData(RewardEvent, DataRegistry);
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
	Snapshot.PendingBattleReward.RewardGold = GetPendingBattleRewardGold();
	Snapshot.PendingBattleReward.bCanClaim = HasPendingBattleReward();
	Snapshot.PendingBattleReward.RewardEntries = MakePreviewRewardEntries(PendingRewardEntries, DataRegistry);
	Snapshot.PendingBattleReward.RewardEntryViews = BuildRewardEntryViews(Snapshot.PendingBattleReward.RewardEntries, DataRegistry);

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

	Snapshot.CurrentBuild = BuildCurrentBuildViewData(CurrentState, DataRegistry);
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
	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry(this);
	if (!HasPendingBattleReward())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPendingBattleReward;
		OutFailureMessage = FText::FromString(TEXT("There is no pending battle reward to claim."));
		return false;
	}

	if (!ValidateRewardEntriesForApplication(PendingRewardEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	TArray<FFinalRunRewardEntry> ClaimedEntries = MakeClaimedRewardEntries(PendingRewardEntries, DataRegistry);
	ApplyValidatedRewardEntriesToRunState(PendingRewardEntries, CurrentState);

	CurrentState.LastBattleRewardGold = GetRewardGoldTotal(PendingRewardEntries);
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::PendingBattleRewardClaimed;
	OutDetailEvent.NodeId = PendingRewardSourceNodeId;
	OutDetailEvent.SourceNodeId = PendingRewardSourceNodeId;
	OutDetailEvent.EncounterId = PendingRewardSourceEncounterId;
	OutDetailEvent.BattleOutcome = PendingRewardBattleOutcome;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ClaimedEntries);
	OutDetailEvent.RewardEntries = ClaimedEntries;
	PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	PopulateAffectedCharacterResults(OutDetailEvent, ClaimedEntries, CurrentState, DataRegistry);
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

	const TArray<FFinalRunRewardEntry> PreviewEntries = MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = MakeClaimedRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry);
	if (!ValidateRewardEntriesForApplication(PreviewEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	ApplyValidatedRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::RewardNodeResolved;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	PopulateAffectedCharacterResults(OutDetailEvent, ResolvedEntries, CurrentState, DataRegistry);
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

	const TArray<FFinalRunRewardEntry> PreviewEntries = MakePreviewRewardEntries(SelectedOption->RewardEntries, DataRegistry);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = MakeClaimedRewardEntries(SelectedOption->RewardEntries, DataRegistry);
	if (!ValidateRewardEntriesForApplication(PreviewEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	ApplyValidatedRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::EventNodeResolved;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.PayloadId = OptionId;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	PopulateAffectedCharacterResults(OutDetailEvent, ResolvedEntries, CurrentState, DataRegistry);
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

	const TArray<FFinalRunRewardEntry> PreviewEntries = MakePreviewRewardEntries(SelectedOffer->RewardEntries, DataRegistry);
	const TArray<FFinalRunRewardEntry> ResolvedEntries = MakeClaimedRewardEntries(SelectedOffer->RewardEntries, DataRegistry);
	if (!ValidateRewardEntriesForApplication(PreviewEntries, DataRegistry, CurrentState, OutRejectReason, OutFailureMessage))
	{
		return false;
	}

	CurrentState.Gold -= SelectedOffer->Price;
	ApplyValidatedRewardEntriesToRunState(PreviewEntries, CurrentState);
	MarkCurrentNodeResolved();
	CurrentFlowStage = EFinalRunFlowStage::AwaitingNodeAdvance;

	OutDetailEvent.EventType = EFinalRunEventType::ShopOfferPurchased;
	OutDetailEvent.NodeId = CurrentNodeId;
	OutDetailEvent.PayloadId = OfferId;
	OutDetailEvent.SpentGold = SelectedOffer->Price;
	OutDetailEvent.RewardGold = GetRewardGoldTotal(ResolvedEntries);
	OutDetailEvent.RewardEntries = ResolvedEntries;
	PopulateRewardEventViewData(OutDetailEvent, DataRegistry);
	PopulateAffectedCharacterResults(OutDetailEvent, ResolvedEntries, CurrentState, DataRegistry);
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
		? MakeClaimedRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry)
		: MakePreviewRewardEntries(CurrentNode->RewardContent.RewardEntries, DataRegistry);
	View.RewardEntryViews = BuildRewardEntryViews(View.RewardEntries, DataRegistry);
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

	for (const FFinalRunEventOptionDefinition& Option : CurrentNode->EventContent.Options)
	{
		FFinalRunEventOptionViewData OptionView;
		OptionView.OptionId = Option.OptionId;
		OptionView.DisplayText = Option.DisplayText.IsEmpty()
			? FText::FromName(Option.OptionId)
			: Option.DisplayText;
		OptionView.OutcomeSummary = Option.OutcomeSummary;
		OptionView.RewardEntries = MakePreviewRewardEntries(Option.RewardEntries, DataRegistry);
		OptionView.RewardEntryViews = BuildRewardEntryViews(OptionView.RewardEntries, DataRegistry);
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
		OfferView.RewardEntries = MakePreviewRewardEntries(Offer.RewardEntries, DataRegistry);
		OfferView.RewardEntryViews = BuildRewardEntryViews(OfferView.RewardEntries, DataRegistry);

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
