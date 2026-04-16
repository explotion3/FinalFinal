#include "UI/Screens/Debug/FinalPrototypeRunDebugScreen.h"

#include "App/FinalGameInstance.h"
#include "BattleBridge/FinalBattleEventPresentationUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Queries/FinalDataRegistry.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Save/FinalSaveGameCoordinator.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

namespace
{
UTextBlock* CreatePrototypeLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize, const FLinearColor& Color = FLinearColor::White)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

FText FormatOptionalDisplayName(const FText& DisplayName, const FString& FallbackValue)
{
	return !DisplayName.IsEmpty() ? DisplayName : FText::FromString(FallbackValue);
}

FText GetPrototypeRelicEffectTypeText(const EFinalRelicBattleStartEffectType EffectType)
{
	switch (EffectType)
	{
	case EFinalRelicBattleStartEffectType::GainAP:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RelicEffectTypeGainAP", "GainAP");

	case EFinalRelicBattleStartEffectType::GainShield:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RelicEffectTypeGainShield", "GainShield");

	case EFinalRelicBattleStartEffectType::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RelicEffectTypeNone", "None");
	}
}

FText GetPrototypeTurnStartRelicEffectTypeText(const EFinalRelicPlayerTurnStartEffectType EffectType)
{
	switch (EffectType)
	{
	case EFinalRelicPlayerTurnStartEffectType::GainAP:
		return NSLOCTEXT("FinalPrototypeRunDebug", "TurnStartRelicEffectTypeGainAP", "GainAP");

	case EFinalRelicPlayerTurnStartEffectType::GainShield:
		return NSLOCTEXT("FinalPrototypeRunDebug", "TurnStartRelicEffectTypeGainShield", "GainShield");

	case EFinalRelicPlayerTurnStartEffectType::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "TurnStartRelicEffectTypeNone", "None");
	}
}

FString BuildBattleRelicEffectSummaryString(const TArray<FFinalBattleStartRelicEffectInput>& EffectInputs)
{
	if (EffectInputs.IsEmpty())
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoBattleRelicEffects", "No battle-start effects").ToString();
	}

	TArray<FString> Segments;
	Segments.Reserve(EffectInputs.Num());
	for (const FFinalBattleStartRelicEffectInput& EffectInput : EffectInputs)
	{
		const FString EffectLabel = GetPrototypeRelicEffectTypeText(EffectInput.EffectType).ToString();
		Segments.Add(FString::Printf(
			TEXT("%s +%d"),
			*EffectLabel,
			EffectInput.Value));
	}

	return FString::Join(Segments, TEXT(" | "));
}

FString BuildBattleRelicTurnStartEffectSummaryString(const TArray<FFinalBattlePlayerTurnStartRelicEffectInput>& EffectInputs)
{
	if (EffectInputs.IsEmpty())
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoTurnStartRelicEffects", "No player-turn-start effects").ToString();
	}

	TArray<FString> Segments;
	Segments.Reserve(EffectInputs.Num());
	for (const FFinalBattlePlayerTurnStartRelicEffectInput& EffectInput : EffectInputs)
	{
		const FString EffectLabel = GetPrototypeTurnStartRelicEffectTypeText(EffectInput.EffectType).ToString();
		Segments.Add(FString::Printf(
			TEXT("%s +%d"),
			*EffectLabel,
			EffectInput.Value));
	}

	return FString::Join(Segments, TEXT(" | "));
}

FText GetFlowStageText(const EFinalRunFlowStage FlowStage)
{
	switch (FlowStage)
	{
	case EFinalRunFlowStage::PreparingBattle:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPreparingBattle", "PreparingBattle");

	case EFinalRunFlowStage::PendingBattleReward:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingBattleReward", "PendingBattleReward");

	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowAwaitingNodeAdvance", "AwaitingNodeAdvance");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingRewardNode", "PendingRewardNode");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingEventNode", "PendingEventNode");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingShopNode", "PendingShopNode");

	case EFinalRunFlowStage::RunEnded:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowRunEnded", "RunEnded");

	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowNone", "None");
	}
}

FText GetNodeSummaryText(const FFinalRunProgressionViewData& Progression)
{
	const FText NodeName = !Progression.CurrentNodeDisplayName.IsEmpty()
		? Progression.CurrentNodeDisplayName
		: NSLOCTEXT("FinalPrototypeRunDebug", "NoCurrentNodeName", "No current node");

	return FText::Format(
		NSLOCTEXT("FinalPrototypeRunDebug", "NodeSummaryFormat", "{0} | Chapter {1} Floor {2}"),
		NodeName,
		FText::AsNumber(Progression.CurrentChapter),
		FText::AsNumber(Progression.CurrentFloor));
}

FString BuildDeckEntriesSummaryString(const TArray<FFinalRunDeckEntryViewData>& DeckEntries)
{
	if (DeckEntries.IsEmpty())
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoDeckEntries", "CurrentBuild 还没有公开任何牌库条目。").ToString();
	}

	TArray<FString> Lines;
	Lines.Reserve(DeckEntries.Num() + 1);
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "DeckSectionTitle", "Current Deck").ToString());

	for (const FFinalRunDeckEntryViewData& Entry : DeckEntries)
	{
		Lines.Add(FString::Printf(
			TEXT("- %s | CardId: %s | Count: %d"),
			*FormatOptionalDisplayName(Entry.DisplayName, Entry.CardId.ToString()).ToString(),
			*Entry.CardId.ToString(),
			Entry.Count));
	}

	return FString::Join(Lines, TEXT("\n"));
}

FString BuildRelicEntriesSummaryString(const TArray<FFinalRunRelicEntryViewData>& RelicEntries)
{
	if (RelicEntries.IsEmpty())
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoRelicEntries", "CurrentBuild 还没有公开任何遗物条目。").ToString();
	}

	TArray<FString> Lines;
	Lines.Reserve(RelicEntries.Num() + 1);
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "RelicSectionTitle", "Current Relics").ToString());

	for (const FFinalRunRelicEntryViewData& Entry : RelicEntries)
	{
		Lines.Add(FString::Printf(
			TEXT("- %s | DisplayId: %s | RelicId: %s | Count: %d"),
			*FormatOptionalDisplayName(Entry.DisplayName, Entry.RelicId.ToString()).ToString(),
			*Entry.DisplayId.ToString(),
			*Entry.RelicId.ToString(),
			Entry.Count));
	}

	return FString::Join(Lines, TEXT("\n"));
}

FString BuildBattleActiveRelicsSummaryString(const TArray<FFinalBattleStartRelicInput>& ActiveRelics)
{
	if (ActiveRelics.IsEmpty())
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoBattleActiveRelics", "Battle Active Relics\n当前战斗没有公开的激活遗物。").ToString();
	}

	TArray<FString> Lines;
	Lines.Reserve(ActiveRelics.Num() + 1);
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "BattleActiveRelicsTitle", "Battle Active Relics").ToString());

	for (const FFinalBattleStartRelicInput& RelicInput : ActiveRelics)
	{
		const FString RelicName = FormatOptionalDisplayName(
			RelicInput.DisplayName,
			RelicInput.RelicId.IsValid() ? RelicInput.RelicId.ToString() : RelicInput.DisplayId.ToString()).ToString();

		Lines.Add(FString::Printf(
			TEXT("- %s | DisplayId: %s | RelicId: %s | Start: %s | Turn: %s"),
			*RelicName,
			*RelicInput.DisplayId.ToString(),
			*RelicInput.RelicId.ToString(),
			*BuildBattleRelicEffectSummaryString(RelicInput.BattleStartEffects),
			*BuildBattleRelicTurnStartEffectSummaryString(RelicInput.PlayerTurnStartEffects)));
	}

	return FString::Join(Lines, TEXT("\n"));
}

bool TryFindLatestBattleEvent(const UFinalBattleFlowSubsystem* BattleFlowSubsystem, FFinalBattleEvent& OutBattleEvent)
{
	if (BattleFlowSubsystem == nullptr)
	{
		return false;
	}

	const TArray<FFinalBattleEvent> BattleLogEntries = BattleFlowSubsystem->GetBattleLogEntries();
	if (BattleLogEntries.IsEmpty())
	{
		return false;
	}

	OutBattleEvent = BattleLogEntries.Last();
	return true;
}

FString BuildLatestBattleEventSummaryString(
	const UFinalBattleFlowSubsystem* BattleFlowSubsystem,
	const FFinalBattleSnapshot& BattleSnapshot,
	const UFinalDataRegistry* DataRegistry)
{
	FFinalBattleEvent BattleEvent;
	if (!TryFindLatestBattleEvent(BattleFlowSubsystem, BattleEvent))
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoBattleEvent", "Latest Battle Event\n当前还没有公开的 BattleEvent。").ToString();
	}

	const FinalBattleEventPresentation::FEventPresentation EventPresentation =
		FinalBattleEventPresentation::BuildPresentation(BattleEvent, BattleSnapshot, DataRegistry);

	TArray<FString> Lines;
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "LatestBattleEventTitle", "Latest Battle Event").ToString());
	if (!EventPresentation.TitleText.IsEmpty())
	{
		Lines.Add(FString::Printf(TEXT("- Title: %s"), *EventPresentation.TitleText.ToString()));
	}
	if (!EventPresentation.SummaryText.IsEmpty())
	{
		Lines.Add(FString::Printf(TEXT("- Summary: %s"), *EventPresentation.SummaryText.ToString()));
	}
	if (!EventPresentation.DetailText.IsEmpty())
	{
		Lines.Add(FString::Printf(TEXT("- Detail: %s"), *EventPresentation.DetailText.ToString()));
	}
	return FString::Join(Lines, TEXT("\n"));
}

FText GetRewardTypeText(const EFinalRunRewardType RewardType)
{
	switch (RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeGold", "Gold");

	case EFinalRunRewardType::CardGrant:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeCardGrant", "CardGrant");

	case EFinalRunRewardType::RelicGrant:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeRelicGrant", "RelicGrant");

	case EFinalRunRewardType::RemoveCard:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeRemoveCard", "RemoveCard");

	case EFinalRunRewardType::UpgradeCard:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeUpgradeCard", "UpgradeCard");

	case EFinalRunRewardType::Growth:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeGrowth", "Growth");

	case EFinalRunRewardType::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTypeNone", "None");
	}
}

FText GetRewardPresentationKindText(const EFinalRunRewardPresentationKind PresentationKind)
{
	switch (PresentationKind)
	{
	case EFinalRunRewardPresentationKind::Gold:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardPresentationGold", "Gold");

	case EFinalRunRewardPresentationKind::Card:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardPresentationCard", "Card");

	case EFinalRunRewardPresentationKind::Relic:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardPresentationRelic", "Relic");

	case EFinalRunRewardPresentationKind::DeckEdit:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardPresentationDeckEdit", "DeckEdit");

	case EFinalRunRewardPresentationKind::Growth:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardPresentationGrowth", "Growth");

	case EFinalRunRewardPresentationKind::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardPresentationNone", "None");
	}
}

FText GetRewardVisualTierText(const EFinalRunRewardVisualTier VisualTier)
{
	switch (VisualTier)
	{
	case EFinalRunRewardVisualTier::Currency:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierCurrency", "Currency");

	case EFinalRunRewardVisualTier::Common:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierCommon", "Common");

	case EFinalRunRewardVisualTier::Rare:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierRare", "Rare");

	case EFinalRunRewardVisualTier::Epic:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierEpic", "Epic");

	case EFinalRunRewardVisualTier::Legendary:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierLegendary", "Legendary");

	case EFinalRunRewardVisualTier::Utility:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierUtility", "Utility");

	case EFinalRunRewardVisualTier::Progression:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierProgression", "Progression");

	case EFinalRunRewardVisualTier::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "RewardTierNone", "None");
	}
}

FText GetPrototypeRunGrowthEffectTypeText(const EFinalRunGrowthEffectType GrowthEffectType)
{
	switch (GrowthEffectType)
	{
	case EFinalRunGrowthEffectType::ReduceStress:
		return NSLOCTEXT("FinalPrototypeRunDebug", "GrowthEffectReduceStress", "ReduceStress");

	case EFinalRunGrowthEffectType::GainAwakenProgress:
		return NSLOCTEXT("FinalPrototypeRunDebug", "GrowthEffectGainAwakenProgress", "GainAwakenProgress");

	case EFinalRunGrowthEffectType::ReduceCollapseCount:
		return NSLOCTEXT("FinalPrototypeRunDebug", "GrowthEffectReduceCollapseCount", "ReduceCollapseCount");

	case EFinalRunGrowthEffectType::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "GrowthEffectNone", "None");
	}
}

FString BuildRewardEntryDebugDetailString(const FFinalRunRewardEntry& Entry)
{
	switch (Entry.RewardType)
	{
	case EFinalRunRewardType::Growth:
		return FString::Printf(
			TEXT(" | Target: %s | Effect: %s"),
			*Entry.GrowthTargetCharacterId.ToString(),
			*GetPrototypeRunGrowthEffectTypeText(Entry.GrowthEffectType).ToString());

	case EFinalRunRewardType::RemoveCard:
		return Entry.RemovedCardId.IsValid()
			? FString::Printf(TEXT(" | Remove: %s"), *Entry.RemovedCardId.ToString())
			: FString();

	case EFinalRunRewardType::UpgradeCard:
		return (Entry.UpgradeFromCardId.IsValid() && Entry.UpgradeToCardId.IsValid())
			? FString::Printf(TEXT(" | Upgrade: %s -> %s"), *Entry.UpgradeFromCardId.ToString(), *Entry.UpgradeToCardId.ToString())
			: FString();

	default:
		return FString();
	}
}

FText GetRewardViewPrimaryText(const FFinalRunRewardEntryViewData& EntryView)
{
	return !EntryView.PrimaryText.IsEmpty()
		? EntryView.PrimaryText
		: GetRewardTypeText(EntryView.RewardType);
}

FString BuildRewardEntryViewDebugDetailString(const FFinalRunRewardEntryViewData& EntryView)
{
	FString Detail;

	Detail += FString::Printf(
		TEXT(" | Kind: %s | Tier: %s"),
		*GetRewardPresentationKindText(EntryView.PresentationKind).ToString(),
		*GetRewardVisualTierText(EntryView.VisualTier).ToString());

	if (!EntryView.SecondaryText.IsEmpty())
	{
		Detail += FString::Printf(TEXT(" | Secondary: %s"), *EntryView.SecondaryText.ToString());
	}

	if (!EntryView.DetailText.IsEmpty())
	{
		Detail += FString::Printf(TEXT(" | Detail: %s"), *EntryView.DetailText.ToString());
	}

	if (EntryView.IconId != NAME_None)
	{
		Detail += FString::Printf(TEXT(" | IconId: %s"), *EntryView.IconId.ToString());
	}

	switch (EntryView.RewardType)
	{
	case EFinalRunRewardType::CardGrant:
	case EFinalRunRewardType::RemoveCard:
		if (EntryView.CardId.IsValid())
		{
			Detail += FString::Printf(TEXT(" | CardId: %s"), *EntryView.CardId.ToString());
		}
		return Detail;

	case EFinalRunRewardType::RelicGrant:
		if (EntryView.RelicId.IsValid())
		{
			Detail += FString::Printf(TEXT(" | RelicId: %s"), *EntryView.RelicId.ToString());
		}
		return Detail;

	case EFinalRunRewardType::UpgradeCard:
		if (EntryView.SourceCardId.IsValid() || EntryView.ResultCardId.IsValid())
		{
			Detail += FString::Printf(TEXT(" | Upgrade: %s -> %s"), *EntryView.SourceCardId.ToString(), *EntryView.ResultCardId.ToString());
		}
		return Detail;

	case EFinalRunRewardType::Growth:
		if (EntryView.TargetCharacterId.IsValid())
		{
			Detail += FString::Printf(TEXT(" | Target: %s"), *EntryView.TargetCharacterId.ToString());
		}
		return Detail;

	default:
		return Detail;
	}
}

void AppendRewardEntryViewCandidateLines(TArray<FString>& Lines, const FString& SourceLabel, const TArray<FFinalRunRewardEntryViewData>& RewardEntryViews)
{
	for (const FFinalRunRewardEntryViewData& EntryView : RewardEntryViews)
	{
		const FString DebugDetail = BuildRewardEntryViewDebugDetailString(EntryView);
		Lines.Add(FString::Printf(
			TEXT("- [%s] %s | Type: %s | Value: %d | Claimable: %s | Claimed: %s%s"),
			*SourceLabel,
			*GetRewardViewPrimaryText(EntryView).ToString(),
			*GetRewardTypeText(EntryView.RewardType).ToString(),
			EntryView.Value,
			TEXT("n/a"),
			TEXT("n/a"),
			*DebugDetail));
	}
}

void AppendRewardEntryCandidateLines(TArray<FString>& Lines, const FString& SourceLabel, const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		const FString DebugDetail = BuildRewardEntryDebugDetailString(Entry);
		Lines.Add(FString::Printf(
			TEXT("- [%s] %s | Type: %s | Value: %d | Claimable: %s | Claimed: %s%s"),
			*SourceLabel,
			*FormatOptionalDisplayName(Entry.DisplayName, TEXT("Unnamed Reward")).ToString(),
			*GetRewardTypeText(Entry.RewardType).ToString(),
			Entry.Value,
			Entry.bCanClaim ? TEXT("Yes") : TEXT("No"),
			Entry.bClaimed ? TEXT("Yes") : TEXT("No"),
			*DebugDetail));
	}
}

FString BuildCharacterStateSummaryString(const TArray<FFinalRunCharacterViewData>& Characters)
{
	if (Characters.IsEmpty())
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoCharacterStateEntries", "Persistent Characters\n当前没有公开的角色持久状态。").ToString();
	}

	TArray<FString> Lines;
	Lines.Reserve(Characters.Num() + 1);
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "CharacterStateTitle", "Persistent Characters").ToString());

	for (const FFinalRunCharacterViewData& Character : Characters)
	{
		const FString DisplayName = FormatOptionalDisplayName(Character.DisplayName, Character.CharacterId.ToString()).ToString();
		const FString StateSummary = FinalRunFlowScreenUtils::BuildCharacterViewStateSummaryString(Character);

		Lines.Add(FString::Printf(
			TEXT("- %s | IconId: %s | Summary: %s | CharacterId: %s | Stress: %d | Awaken: %d | Collapse: %d"),
			*DisplayName,
			Character.IconId != NAME_None ? *Character.IconId.ToString() : TEXT("None"),
			*StateSummary,
			*Character.CharacterId.ToString(),
			Character.CurrentStress,
			Character.CurrentAwakenCount,
			Character.CollapseCount));
	}

	return FString::Join(Lines, TEXT("\n"));
}

FString BuildLatestAffectedCharacterResultsSummaryString(const UFinalRunFlowSubsystem* RunFlowSubsystem)
{
	if (RunFlowSubsystem == nullptr)
	{
		return NSLOCTEXT("FinalPrototypeRunDebug", "NoRunFlowForAffectedCharacterResults", "Last Event Character Results\n当前无法访问 RunFlowSubsystem。").ToString();
	}

	const FFinalRunEvent LastRunEvent = RunFlowSubsystem->GetLastProcessedRunEvent();
	const FString CharacterResultSummary = FinalRunFlowScreenUtils::BuildCharacterResultsSummaryString(
		LastRunEvent.AffectedCharacterResults,
		NSLOCTEXT("FinalPrototypeRunDebug", "LastEventCharacterResultsTitle", "Last Event Character Results"));

	if (!CharacterResultSummary.IsEmpty())
	{
		return CharacterResultSummary;
	}

	return NSLOCTEXT("FinalPrototypeRunDebug", "NoAffectedCharacterResults", "Last Event Character Results\n当前最近 RunEvent 没有公开的角色结果摘要。").ToString();
}

FString BuildPendingRewardCandidatesSummary(const FFinalRunSnapshot& RunSnapshot)
{
	TArray<FString> Lines;
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "CandidateSectionTitle", "Visible Pending Reward Candidates").ToString());

	if (RunSnapshot.PendingBattleReward.RewardEntryViews.Num() > 0)
	{
		AppendRewardEntryViewCandidateLines(Lines, TEXT("BattleReward"), RunSnapshot.PendingBattleReward.RewardEntryViews);
	}
	else
	{
		AppendRewardEntryCandidateLines(Lines, TEXT("BattleReward"), RunSnapshot.PendingBattleReward.RewardEntries);
	}

	if (RunSnapshot.PendingRewardNode.RewardEntryViews.Num() > 0)
	{
		AppendRewardEntryViewCandidateLines(Lines, TEXT("RewardNode"), RunSnapshot.PendingRewardNode.RewardEntryViews);
	}
	else
	{
		AppendRewardEntryCandidateLines(Lines, TEXT("RewardNode"), RunSnapshot.PendingRewardNode.RewardEntries);
	}

	for (const FFinalRunEventOptionViewData& Option : RunSnapshot.PendingEventNode.Options)
	{
		const FString OptionLabel = !Option.DisplayText.IsEmpty()
			? Option.DisplayText.ToString()
			: Option.OptionId.ToString();
		if (Option.RewardEntryViews.Num() > 0)
		{
			AppendRewardEntryViewCandidateLines(Lines, FString::Printf(TEXT("Event:%s"), *OptionLabel), Option.RewardEntryViews);
		}
		else
		{
			AppendRewardEntryCandidateLines(Lines, FString::Printf(TEXT("Event:%s"), *OptionLabel), Option.RewardEntries);
		}
	}

	for (const FFinalRunShopOfferViewData& Offer : RunSnapshot.PendingShopNode.Offers)
	{
		const FString OfferLabel = !Offer.DisplayName.IsEmpty()
			? Offer.DisplayName.ToString()
			: Offer.OfferId.ToString();
		if (Offer.RewardEntryViews.Num() > 0)
		{
			AppendRewardEntryViewCandidateLines(Lines, FString::Printf(TEXT("Shop:%s"), *OfferLabel), Offer.RewardEntryViews);
		}
		else
		{
			AppendRewardEntryCandidateLines(Lines, FString::Printf(TEXT("Shop:%s"), *OfferLabel), Offer.RewardEntries);
		}
	}

	if (Lines.Num() == 1)
	{
		Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "NoCandidateEntries", "当前没有可见的 pending reward 候选。").ToString());
	}

	return FString::Join(Lines, TEXT("\n"));
}

FText GetLatestDebugMessage(
	const UFinalRunFlowSubsystem* RunFlowSubsystem,
	const UFinalGameFlowSubsystem* GameFlowSubsystem,
	const UFinalGameInstance* FinalGameInstance)
{
	if (RunFlowSubsystem != nullptr)
	{
		const FText LastFlowMessage = RunFlowSubsystem->GetLastFlowMessage();
		if (!LastFlowMessage.IsEmpty())
		{
			return LastFlowMessage;
		}

		const FFinalRunEvent LastRunEvent = RunFlowSubsystem->GetLastProcessedRunEvent();
		if (!LastRunEvent.Message.IsEmpty())
		{
			return LastRunEvent.Message;
		}
	}

	if (GameFlowSubsystem != nullptr)
	{
		const FText LastBattleFailure = GameFlowSubsystem->GetLastBattleFailureReason();
		if (!LastBattleFailure.IsEmpty())
		{
			return LastBattleFailure;
		}
	}

	if (FinalGameInstance != nullptr)
	{
		return FinalGameInstance->GetLastTestFailureReason();
	}

	return FText::GetEmpty();
}
}

void UFinalPrototypeRunDebugScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalPrototypeRunDebugScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->OnRunFlowStateChanged.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleRunFlowStateChanged);
	}

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleBattleSnapshotChanged);
		BattleFlowSubsystem->OnBattleEventBroadcast.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleBattleEventBroadcast);
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::NativeDestruct()
{
	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->OnRunFlowStateChanged.RemoveDynamic(this, &UFinalPrototypeRunDebugScreen::HandleRunFlowStateChanged);
	}

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &UFinalPrototypeRunDebugScreen::HandleBattleSnapshotChanged);
		BattleFlowSubsystem->OnBattleEventBroadcast.RemoveDynamic(this, &UFinalPrototypeRunDebugScreen::HandleBattleEventBroadcast);
	}

	Super::NativeDestruct();
}

void UFinalPrototypeRunDebugScreen::RefreshFromSubsystems()
{
	if (SummaryText == nullptr
		|| MessageText == nullptr
		|| SaveStatusText == nullptr
		|| BuildSummaryText == nullptr
		|| CharacterSummaryText == nullptr
		|| EventCharacterResultText == nullptr
		|| CandidateSummaryText == nullptr
		|| BattleRelicSummaryText == nullptr
		|| BattleEventSummaryText == nullptr)
	{
		return;
	}

	const UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	const UFinalGameFlowSubsystem* GameFlowSubsystem = ResolveGameFlowSubsystem();
	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem();
	const UFinalSaveGameCoordinator* SaveGameCoordinator = ResolveSaveGameCoordinator();
	const UFinalGameInstance* FinalGameInstance = ResolveFinalGameInstance();
	const UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;

	const FFinalRunSnapshot RunSnapshot = RunFlowSubsystem ? RunFlowSubsystem->GetCurrentRunSnapshot() : FFinalRunSnapshot{};
	const FFinalBattleSnapshot BattleSnapshot = GameFlowSubsystem ? GameFlowSubsystem->GetCurrentBattleSnapshot() : FFinalBattleSnapshot{};
	const bool bHasActiveBattleSession = GameFlowSubsystem && GameFlowSubsystem->GetActiveBattleSession() != nullptr;

	SummaryText->SetText(FText::Format(
		NSLOCTEXT("FinalPrototypeRunDebug", "SummaryFormat", "FlowStage: {0}\nNode: {1}\nGold {2} | Deck {3} | Relics {4}\nActiveBattleSession: {5}"),
		GetFlowStageText(RunSnapshot.Progression.FlowStage),
		GetNodeSummaryText(RunSnapshot.Progression),
		FText::AsNumber(RunSnapshot.Gold),
		FText::AsNumber(RunSnapshot.DeckCount),
		FText::AsNumber(RunSnapshot.RelicCount),
		bHasActiveBattleSession
			? (BattleSnapshot.bBattleEnded
				? NSLOCTEXT("FinalPrototypeRunDebug", "BattleEnded", "Yes (Resolved)")
				: NSLOCTEXT("FinalPrototypeRunDebug", "BattleActive", "Yes"))
			: NSLOCTEXT("FinalPrototypeRunDebug", "BattleInactive", "No")));

	const FText LatestMessage = GetLatestDebugMessage(RunFlowSubsystem, GameFlowSubsystem, FinalGameInstance);
	MessageText->SetText(!LatestMessage.IsEmpty()
		? LatestMessage
		: NSLOCTEXT("FinalPrototypeRunDebug", "NoLatestMessage", "No recent flow feedback."));

	SaveStatusText->SetText(SaveGameCoordinator != nullptr
		? SaveGameCoordinator->GetPrototypeRunSaveDebugText()
		: NSLOCTEXT("FinalPrototypeRunDebug", "SaveCoordinatorUnavailable", "Save Slot: FinalPrototypeRun\nSlot Exists: Unknown\nLast Status: FinalSaveGameCoordinator is unavailable."));

	const FString DeckSummary = BuildDeckEntriesSummaryString(RunSnapshot.CurrentBuild.DeckEntries);
	const FString RelicSummary = BuildRelicEntriesSummaryString(RunSnapshot.CurrentBuild.RelicEntries);
	BuildSummaryText->SetText(FText::FromString(FString::Printf(TEXT("%s\n\n%s"), *DeckSummary, *RelicSummary)));
	CharacterSummaryText->SetText(FText::FromString(BuildCharacterStateSummaryString(RunSnapshot.Characters)));
	EventCharacterResultText->SetText(FText::FromString(BuildLatestAffectedCharacterResultsSummaryString(RunFlowSubsystem)));

	CandidateSummaryText->SetText(FText::FromString(BuildPendingRewardCandidatesSummary(RunSnapshot)));
	BattleRelicSummaryText->SetText(FText::FromString(BuildBattleActiveRelicsSummaryString(BattleSnapshot.ActiveRelics)));
	BattleEventSummaryText->SetText(FText::FromString(BuildLatestBattleEventSummaryString(BattleFlowSubsystem, BattleSnapshot, DataRegistry)));

	if (RestartRunButton)
	{
		RestartRunButton->SetIsEnabled(true);
	}

	if (SaveRunButton)
	{
		SaveRunButton->SetIsEnabled(GameFlowSubsystem != nullptr && GameFlowSubsystem->GetRunSession() != nullptr && !bHasActiveBattleSession);
	}

	if (LoadRunButton)
	{
		LoadRunButton->SetIsEnabled(SaveGameCoordinator != nullptr && SaveGameCoordinator->DoesPrototypeRunSaveExist() && !bHasActiveBattleSession);
	}

	if (CompleteResolvedBattleButton)
	{
		CompleteResolvedBattleButton->SetIsEnabled(bHasActiveBattleSession && BattleSnapshot.bBattleEnded);
	}
}

void UFinalPrototypeRunDebugScreen::HandleRunFlowStateChanged()
{
	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent)
{
	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleRestartPrototypeRunClicked()
{
	if (UFinalGameInstance* FinalGameInstance = ResolveFinalGameInstance())
	{
		FinalGameInstance->StartTestBattle();
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleCompleteResolvedBattleClicked()
{
	if (UFinalGameFlowSubsystem* GameFlowSubsystem = ResolveGameFlowSubsystem())
	{
		GameFlowSubsystem->CompleteResolvedBattle();
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleSavePrototypeRunClicked()
{
	if (UFinalSaveGameCoordinator* SaveGameCoordinator = ResolveSaveGameCoordinator())
	{
		SaveGameCoordinator->SaveCurrentRunToPrototypeSlot();
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleLoadPrototypeRunClicked()
{
	if (UFinalSaveGameCoordinator* SaveGameCoordinator = ResolveSaveGameCoordinator())
	{
		SaveGameCoordinator->LoadRunFromPrototypeSlot();
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	ScreenLayer = EFinalUIScreenLayer::HUD;

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PrototypeRunDebugRoot"));
	WidgetTree->RootWidget = RootOverlay;

	PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PrototypeRunDebugPanel"));
	PanelBorder->SetBrushColor(FLinearColor(0.05f, 0.06f, 0.09f, 0.88f));
	PanelBorder->SetPadding(FMargin(12.0f));
	if (UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(PanelBorder))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Right);
		PanelSlot->SetVerticalAlignment(VAlign_Top);
		PanelSlot->SetPadding(FMargin(16.0f));
	}

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PrototypeRunDebugContent"));
	PanelBorder->SetContent(ContentBox);

	TitleText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugTitle"), 15, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f));
	TitleText->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "Title", "Prototype Run Summary"));
	ContentBox->AddChildToVerticalBox(TitleText);

	SummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugSummary"), 12);
	if (UVerticalBoxSlot* SummarySlot = ContentBox->AddChildToVerticalBox(SummaryText))
	{
		SummarySlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	MessageText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugMessage"), 11, FLinearColor(0.86f, 0.88f, 0.92f, 1.0f));
	if (UVerticalBoxSlot* MessageSlot = ContentBox->AddChildToVerticalBox(MessageText))
	{
		MessageSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	SaveStatusText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugSaveStatus"), 10, FLinearColor(0.80f, 0.92f, 0.84f, 1.0f));
	if (UVerticalBoxSlot* SaveStatusSlot = ContentBox->AddChildToVerticalBox(SaveStatusText))
	{
		SaveStatusSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	BuildSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugBuildSummary"), 11, FLinearColor(0.90f, 0.95f, 1.0f, 1.0f));
	if (UVerticalBoxSlot* BuildSlot = ContentBox->AddChildToVerticalBox(BuildSummaryText))
	{
		BuildSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	CharacterSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugCharacterSummary"), 10, FLinearColor(0.88f, 0.92f, 0.98f, 1.0f));
	if (UVerticalBoxSlot* CharacterSlot = ContentBox->AddChildToVerticalBox(CharacterSummaryText))
	{
		CharacterSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	EventCharacterResultText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugEventCharacterResult"), 10, FLinearColor(0.84f, 0.90f, 0.96f, 1.0f));
	if (UVerticalBoxSlot* EventCharacterResultSlot = ContentBox->AddChildToVerticalBox(EventCharacterResultText))
	{
		EventCharacterResultSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	CandidateSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugCandidateSummary"), 10, FLinearColor(0.78f, 0.82f, 0.88f, 1.0f));
	if (UVerticalBoxSlot* CandidateSlot = ContentBox->AddChildToVerticalBox(CandidateSummaryText))
	{
		CandidateSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	BattleRelicSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugBattleRelicSummary"), 10, FLinearColor(0.92f, 0.88f, 0.72f, 1.0f));
	if (UVerticalBoxSlot* BattleRelicSummarySlot = ContentBox->AddChildToVerticalBox(BattleRelicSummaryText))
	{
		BattleRelicSummarySlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	BattleEventSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugBattleEventSummary"), 10, FLinearColor(0.95f, 0.83f, 0.62f, 1.0f));
	if (UVerticalBoxSlot* BattleRelicEventSlot = ContentBox->AddChildToVerticalBox(BattleEventSummaryText))
	{
		BattleRelicEventSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	RestartRunButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrototypeRunDebugRestartButton"));
	RestartRunButton->OnClicked.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleRestartPrototypeRunClicked);
	RestartRunLabel = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugRestartLabel"), 12);
	RestartRunLabel->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "RestartRunButton", "Restart Prototype Run"));
	RestartRunButton->AddChild(RestartRunLabel);
	if (UVerticalBoxSlot* RestartSlot = ContentBox->AddChildToVerticalBox(RestartRunButton))
	{
		RestartSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}

	SaveRunButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrototypeRunDebugSaveButton"));
	SaveRunButton->OnClicked.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleSavePrototypeRunClicked);
	SaveRunLabel = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugSaveLabel"), 12);
	SaveRunLabel->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "SaveRunButton", "Save Prototype Run"));
	SaveRunButton->AddChild(SaveRunLabel);
	if (UVerticalBoxSlot* SaveSlot = ContentBox->AddChildToVerticalBox(SaveRunButton))
	{
		SaveSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	LoadRunButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrototypeRunDebugLoadButton"));
	LoadRunButton->OnClicked.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleLoadPrototypeRunClicked);
	LoadRunLabel = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugLoadLabel"), 12);
	LoadRunLabel->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "LoadRunButton", "Load Prototype Run"));
	LoadRunButton->AddChild(LoadRunLabel);
	if (UVerticalBoxSlot* LoadSlot = ContentBox->AddChildToVerticalBox(LoadRunButton))
	{
		LoadSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	CompleteResolvedBattleButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrototypeRunDebugCompleteResolvedBattleButton"));
	CompleteResolvedBattleButton->OnClicked.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleCompleteResolvedBattleClicked);
	CompleteResolvedBattleLabel = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugCompleteResolvedBattleLabel"), 12);
	CompleteResolvedBattleLabel->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "CompleteResolvedBattleButton", "Complete Resolved Battle"));
	CompleteResolvedBattleButton->AddChild(CompleteResolvedBattleLabel);
	ContentBox->AddChildToVerticalBox(CompleteResolvedBattleButton);
}

UFinalBattleFlowSubsystem* UFinalPrototypeRunDebugScreen::ResolveBattleFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
}

UFinalGameFlowSubsystem* UFinalPrototypeRunDebugScreen::ResolveGameFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
}

UFinalRunFlowSubsystem* UFinalPrototypeRunDebugScreen::ResolveRunFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr;
}

UFinalSaveGameCoordinator* UFinalPrototypeRunDebugScreen::ResolveSaveGameCoordinator() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalSaveGameCoordinator>() : nullptr;
}

UFinalGameInstance* UFinalPrototypeRunDebugScreen::ResolveFinalGameInstance() const
{
	return Cast<UFinalGameInstance>(GetGameInstance());
}
