#include "Rewards/FinalRewardResolver.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Growth/FinalGrowthResolver.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalRelicDefinition.h"

namespace FinalRewardResolverLocal
{
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

FText GetDefaultRewardDisplayName(const EFinalRunRewardType RewardType)
{
	switch (RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayGold", "Gold");
	case EFinalRunRewardType::CardGrant:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayCard", "Card");
	case EFinalRunRewardType::RelicGrant:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayRelic", "Relic");
	case EFinalRunRewardType::RemoveCard:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayRemoveCard", "Remove Card");
	case EFinalRunRewardType::UpgradeCard:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayUpgradeCard", "Upgrade Card");
	case EFinalRunRewardType::Growth:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayGrowth", "Growth");
	default:
		return NSLOCTEXT("FinalRewardResolver", "RewardDisplayUnknown", "Reward");
	}
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
		: NSLOCTEXT("FinalRewardResolver", "UnknownGrowthCharacter", "Unknown Character");
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
		: NSLOCTEXT("FinalRewardResolver", "UnknownDeckCard", "Unknown Card");
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

			if (!RelicDefinition->DisplayId.IsNone())
			{
				return FText::FromName(RelicDefinition->DisplayId);
			}
		}
	}

	return RelicId.IsValid()
		? FText::FromName(RelicId.Value)
		: NSLOCTEXT("FinalRewardResolver", "UnknownRelic", "Unknown Relic");
}

FText GetGrowthEffectDisplayName(const EFinalRunGrowthEffectType GrowthEffectType)
{
	switch (GrowthEffectType)
	{
	case EFinalRunGrowthEffectType::ReduceStress:
		return NSLOCTEXT("FinalRewardResolver", "GrowthEffectReduceStress", "Reduce Stress");
	case EFinalRunGrowthEffectType::GainAwakenProgress:
		return NSLOCTEXT("FinalRewardResolver", "GrowthEffectGainAwakenProgress", "Gain Awaken Progress");
	case EFinalRunGrowthEffectType::ReduceCollapseCount:
		return NSLOCTEXT("FinalRewardResolver", "GrowthEffectReduceCollapseCount", "Reduce Collapse Count");
	default:
		return NSLOCTEXT("FinalRewardResolver", "GrowthEffectUnknown", "Growth");
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
			? ResolveRewardCharacterDisplayName(RewardEntry.GrowthTargetCharacterId, DataRegistry)
			: GetDefaultRewardDisplayName(RewardEntry.RewardType);

	default:
		return GetDefaultRewardDisplayName(RewardEntry.RewardType);
	}
}

FText GetRewardEntryResolvedDisplayName(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	if (!RewardEntry.DisplayName.IsEmpty())
	{
		return RewardEntry.DisplayName;
	}

	return GetRewardEntryDefaultDisplayName(RewardEntry, DataRegistry);
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
				NSLOCTEXT("FinalRewardResolver", "RelicBattleStartGainAPDetail", "Battle start: +{0} AP"),
				FText::AsNumber(EffectDefinition.Value)));
			break;
		case EFinalRelicBattleStartEffectType::GainShield:
			AppendSummaryPart(FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RelicBattleStartGainShieldDetail", "Battle start: +{0} Shield"),
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
				NSLOCTEXT("FinalRewardResolver", "RelicTurnStartGainAPDetail", "Turn start: +{0} AP"),
				FText::AsNumber(EffectDefinition.Value)));
			break;
		case EFinalRelicPlayerTurnStartEffectType::GainShield:
			AppendSummaryPart(FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RelicTurnStartGainShieldDetail", "Turn start: +{0} Shield"),
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
		return RewardEntry.GrantedCardId.IsValid() ? RewardEntry.GrantedCardId.Value : GetRewardEntryDefaultDisplayId(RewardEntry, DataRegistry);
	case EFinalRunRewardType::RelicGrant:
		return GetRewardEntryDefaultDisplayId(RewardEntry, DataRegistry);
	case EFinalRunRewardType::RemoveCard:
		return RewardEntry.RemovedCardId.IsValid() ? RewardEntry.RemovedCardId.Value : GetRewardEntryDefaultDisplayId(RewardEntry, DataRegistry);
	case EFinalRunRewardType::UpgradeCard:
		if (RewardEntry.UpgradeToCardId.IsValid())
		{
			return RewardEntry.UpgradeToCardId.Value;
		}
		return RewardEntry.UpgradeFromCardId.IsValid() ? RewardEntry.UpgradeFromCardId.Value : GetRewardEntryDefaultDisplayId(RewardEntry, DataRegistry);
	case EFinalRunRewardType::Growth:
		return RewardEntry.GrowthTargetCharacterId.IsValid() ? RewardEntry.GrowthTargetCharacterId.Value : NAME_None;
	default:
		return NAME_None;
	}
}

EFinalRunRewardVisualTier ResolveRewardEntryVisualTier(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return EFinalRunRewardVisualTier::Currency;

	case EFinalRunRewardType::CardGrant:
	case EFinalRunRewardType::RemoveCard:
	case EFinalRunRewardType::UpgradeCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardEntryDisplayCardDefinition(RewardEntry, DataRegistry))
		{
			return MapRarityToRewardVisualTier(CardDefinition->Rarity);
		}
		return EFinalRunRewardVisualTier::Utility;

	case EFinalRunRewardType::RelicGrant:
		if (const UFinalRelicDefinition* RelicDefinition = FindRewardEntryRelicDefinition(RewardEntry, DataRegistry))
		{
			return MapRarityToRewardVisualTier(RelicDefinition->Rarity);
		}
		return EFinalRunRewardVisualTier::Utility;

	case EFinalRunRewardType::Growth:
		return EFinalRunRewardVisualTier::Progression;

	default:
		return EFinalRunRewardVisualTier::None;
	}
}

FText BuildUpgradeCardDetailText(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	FFinalRunRewardEntry SourceEntry;
	SourceEntry.RewardType = EFinalRunRewardType::RemoveCard;
	SourceEntry.RemovedCardId = RewardEntry.UpgradeFromCardId;

	FFinalRunRewardEntry ResultEntry;
	ResultEntry.RewardType = EFinalRunRewardType::CardGrant;
	ResultEntry.GrantedCardId = RewardEntry.UpgradeToCardId;

	return FText::Format(
		NSLOCTEXT("FinalRewardResolver", "RewardViewUpgradeCardDetail", "Replace {0} with {1} in the current deck."),
		GetRewardEntryDefaultCardDisplayName(SourceEntry, DataRegistry),
		GetRewardEntryDefaultCardDisplayName(ResultEntry, DataRegistry));
}

FText BuildRewardEntryDetailText(const FFinalRunRewardEntry& RewardEntry, const UFinalDataRegistry* DataRegistry)
{
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalRewardResolver", "RewardViewGoldDetail", "Spend gold on shop offers and other run rewards.");

	case EFinalRunRewardType::CardGrant:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.GrantedCardId, DataRegistry))
		{
			if (!CardDefinition->RulesText.IsEmpty())
			{
				return CardDefinition->RulesText;
			}
		}
		return FText::GetEmpty();

	case EFinalRunRewardType::RelicGrant:
		if (const UFinalRelicDefinition* RelicDefinition = FindRewardEntryRelicDefinition(RewardEntry, DataRegistry))
		{
			if (!RelicDefinition->Description.IsEmpty())
			{
				return RelicDefinition->Description;
			}
			return BuildRelicEffectFallbackDetailText(RelicDefinition);
		}
		return FText::GetEmpty();

	case EFinalRunRewardType::RemoveCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.RemovedCardId, DataRegistry))
		{
			if (!CardDefinition->RulesText.IsEmpty())
			{
				return CardDefinition->RulesText;
			}
		}
		return NSLOCTEXT("FinalRewardResolver", "RewardViewRemoveCardDetail", "Remove this card from the current deck.");

	case EFinalRunRewardType::UpgradeCard:
		if (const UFinalCardDefinition* CardDefinition = FindRewardCardDefinition(RewardEntry.UpgradeToCardId, DataRegistry))
		{
			if (!CardDefinition->RulesText.IsEmpty())
			{
				return CardDefinition->RulesText;
			}
		}
		return BuildUpgradeCardDetailText(RewardEntry, DataRegistry);

	case EFinalRunRewardType::Growth:
		return FText::Format(
			NSLOCTEXT("FinalRewardResolver", "RewardViewGrowthDetail", "Apply {0} to {1}."),
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
	switch (RewardEntry.RewardType)
	{
	case EFinalRunRewardType::Gold:
		return FText::Format(
			NSLOCTEXT("FinalRewardResolver", "RewardViewGoldSecondary", "+{0}"),
			FText::AsNumber(RewardEntry.Value));

	case EFinalRunRewardType::CardGrant:
		return GetRewardEntryApplicationCount(RewardEntry) > 1
			? FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RewardViewAddCardCopiesSecondary", "Add {0} copies to deck"),
				FText::AsNumber(GetRewardEntryApplicationCount(RewardEntry)))
			: NSLOCTEXT("FinalRewardResolver", "RewardViewAddCardSecondary", "Add to deck");

	case EFinalRunRewardType::RelicGrant:
		return GetRewardEntryApplicationCount(RewardEntry) > 1
			? FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RewardViewGainRelicCopiesSecondary", "Gain {0} relic copies"),
				FText::AsNumber(GetRewardEntryApplicationCount(RewardEntry)))
			: NSLOCTEXT("FinalRewardResolver", "RewardViewGainRelicSecondary", "Gain relic");

	case EFinalRunRewardType::RemoveCard:
		return GetRewardEntryApplicationCount(RewardEntry) > 1
			? FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RewardViewRemoveCardCopiesSecondary", "Remove {0} copies from deck"),
				FText::AsNumber(GetRewardEntryApplicationCount(RewardEntry)))
			: NSLOCTEXT("FinalRewardResolver", "RewardViewRemoveCardSecondary", "Remove from deck");

	case EFinalRunRewardType::UpgradeCard:
		return FText::Format(
			NSLOCTEXT("FinalRewardResolver", "RewardViewUpgradeCardSecondary", "Upgrade to {0}"),
			GetRewardEntryDefaultCardDisplayName(RewardEntry, DataRegistry));

	case EFinalRunRewardType::Growth:
		return FText::Format(
			NSLOCTEXT("FinalRewardResolver", "RewardViewGrowthSecondary", "{0} +{1}"),
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
	ViewData.TargetCharacterId = RewardEntry.GrowthTargetCharacterId;
	ViewData.CardId = RewardEntry.GrantedCardId;
	ViewData.SourceCardId = RewardEntry.UpgradeFromCardId;
	ViewData.ResultCardId = RewardEntry.UpgradeToCardId;
	ViewData.RelicId = RewardEntry.GrantedRelicId;
	return ViewData;
}

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
				NSLOCTEXT("FinalRewardResolver", "MissingGrantedCardId", "Reward entry {0} is missing GrantedCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}
		if (DataRegistry == nullptr || DataRegistry->FindCardDefinition(RewardEntry.GrantedCardId) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RewardCardDefinitionUnavailable", "Card reward {0} cannot be applied because card definition {1} is unavailable."),
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
				NSLOCTEXT("FinalRewardResolver", "MissingGrantedRelicId", "Reward entry {0} is missing GrantedRelicId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}
		if (DataRegistry == nullptr || DataRegistry->FindRelicDefinition(RewardEntry.GrantedRelicId) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardRelicDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RewardRelicDefinitionUnavailable", "Relic reward {0} cannot be applied because relic definition {1} is unavailable."),
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
				NSLOCTEXT("FinalRewardResolver", "MissingRemovedCardId", "Reward entry {0} is missing RemovedCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}
		if (DataRegistry == nullptr || FindRewardCardDefinition(RewardEntry.RemovedCardId, DataRegistry) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RemoveCardDefinitionUnavailable", "Remove-card reward {0} cannot be applied because card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.RemovedCardId.Value));
			return false;
		}
		if (CountRunDeckCards(RunState.RunDeck, RewardEntry.RemovedCardId) < GetRewardEntryApplicationCount(RewardEntry))
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardTargetCardNotInRunDeck;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "RemoveCardTargetMissing", "Remove-card reward {0} needs {1} copies of {2} in the RunDeck, but only {3} are available."),
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
				NSLOCTEXT("FinalRewardResolver", "MissingUpgradeFromCardId", "Reward entry {0} is missing UpgradeFromCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}
		if (!RewardEntry.UpgradeToCardId.IsValid())
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingUpgradeToCardId;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "MissingUpgradeToCardId", "Reward entry {0} is missing UpgradeToCardId."),
				FText::FromName(RewardEntry.RewardId));
			return false;
		}
		if (RewardEntry.UpgradeFromCardId == RewardEntry.UpgradeToCardId)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardUpgradeResultInvalid;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "UpgradeResultSameAsSource", "Upgrade reward {0} is invalid because UpgradeToCardId matches UpgradeFromCardId {1}."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.UpgradeFromCardId.Value));
			return false;
		}
		if (DataRegistry == nullptr || FindRewardCardDefinition(RewardEntry.UpgradeFromCardId, DataRegistry) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "UpgradeSourceDefinitionUnavailable", "Upgrade reward {0} cannot be applied because source card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.UpgradeFromCardId.Value));
			return false;
		}
		if (DataRegistry == nullptr || FindRewardCardDefinition(RewardEntry.UpgradeToCardId, DataRegistry) == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardCardDefinitionUnavailable;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "UpgradeResultDefinitionUnavailable", "Upgrade reward {0} cannot be applied because upgraded card definition {1} is unavailable."),
				FText::FromName(RewardEntry.RewardId),
				FText::FromName(RewardEntry.UpgradeToCardId.Value));
			return false;
		}
		if (CountRunDeckCards(RunState.RunDeck, RewardEntry.UpgradeFromCardId) < GetRewardEntryApplicationCount(RewardEntry))
		{
			OutRejectReason = EFinalRunCommandRejectReason::RewardTargetCardNotInRunDeck;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalRewardResolver", "UpgradeSourceMissingFromDeck", "Upgrade reward {0} needs {1} copies of {2} in the RunDeck, but only {3} are available."),
				FText::FromName(RewardEntry.RewardId),
				FText::AsNumber(GetRewardEntryApplicationCount(RewardEntry)),
				FText::FromName(RewardEntry.UpgradeFromCardId.Value),
				FText::AsNumber(CountRunDeckCards(RunState.RunDeck, RewardEntry.UpgradeFromCardId)));
			return false;
		}
		return true;

	case EFinalRunRewardType::Growth:
		return FFinalGrowthResolver::ValidateGrowthReward(RewardEntry, RunState, OutRejectReason, OutFailureMessage);

	case EFinalRunRewardType::None:
	default:
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedRewardType;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRewardResolver", "UnsupportedRewardType", "Reward entry {0} uses reward type {1}, which does not yet have RunState landing support."),
			FText::FromName(RewardEntry.RewardId),
			FText::AsNumber(static_cast<int32>(RewardEntry.RewardType)));
		return false;
	}
}
}

int32 FFinalRewardResolver::GetRewardGoldTotal(const TArray<FFinalRunRewardEntry>& RewardEntries)
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

TArray<FFinalRunRewardEntry> FFinalRewardResolver::MakeClaimedRewardEntries(
	const TArray<FFinalRunRewardEntry>& RewardEntries,
	const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntry> ClaimedEntries = RewardEntries;
	FinalRewardResolverLocal::NormalizeRewardEntries(ClaimedEntries, false, DataRegistry);
	return ClaimedEntries;
}

TArray<FFinalRunRewardEntry> FFinalRewardResolver::MakePreviewRewardEntries(
	const TArray<FFinalRunRewardEntry>& RewardEntries,
	const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntry> PreviewEntries = RewardEntries;
	FinalRewardResolverLocal::NormalizeRewardEntries(PreviewEntries, true, DataRegistry);
	return PreviewEntries;
}

TArray<FFinalRunRewardEntryViewData> FFinalRewardResolver::BuildRewardEntryViews(
	const TArray<FFinalRunRewardEntry>& RewardEntries,
	const UFinalDataRegistry* DataRegistry)
{
	TArray<FFinalRunRewardEntryViewData> ViewData;
	ViewData.Reserve(RewardEntries.Num());
	for (const FFinalRunRewardEntry& RewardEntry : RewardEntries)
	{
		ViewData.Add(FinalRewardResolverLocal::BuildRewardEntryViewData(RewardEntry, DataRegistry));
	}
	return ViewData;
}

void FFinalRewardResolver::PopulateRewardEventViewData(
	FFinalRunEvent& Event,
	const UFinalDataRegistry* DataRegistry)
{
	Event.RewardEntryViews = BuildRewardEntryViews(Event.RewardEntries, DataRegistry);
}

void FFinalRewardResolver::PopulateAffectedCharacterResults(
	FFinalRunEvent& Event,
	const TArray<FFinalRunRewardEntry>& AppliedRewardEntries,
	const FFinalRunState& RunState,
	const UFinalDataRegistry* DataRegistry)
{
	Event.AffectedCharacterResults = FFinalGrowthResolver::BuildAffectedCharacterResults(AppliedRewardEntries, RunState, DataRegistry);
}

bool FFinalRewardResolver::ValidateRewardEntriesForApplication(
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

		if (!FinalRewardResolverLocal::ValidateRewardEntryForApplication(Entry, DataRegistry, SimulatedRunState, OutRejectReason, OutFailureMessage))
		{
			return false;
		}

		TArray<FFinalRunRewardEntry> SingleRewardEntry;
		SingleRewardEntry.Add(Entry);
		ApplyValidatedRewardEntriesToRunState(SingleRewardEntry, SimulatedRunState);
	}

	return true;
}

void FFinalRewardResolver::ApplyValidatedRewardEntriesToRunState(
	const TArray<FFinalRunRewardEntry>& RewardEntries,
	FFinalRunState& RunState)
{
	using namespace FinalRewardResolverLocal;

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
			FFinalGrowthResolver::ApplyGrowthReward(Entry, RunState);
			break;

		default:
			break;
		}
	}
}
