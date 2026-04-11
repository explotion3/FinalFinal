#include "UI/Screens/Debug/FinalPrototypeRunDebugScreen.h"

#include "App/FinalGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"

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

void AppendRewardEntryCandidateLines(TArray<FString>& Lines, const FString& SourceLabel, const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		Lines.Add(FString::Printf(
			TEXT("- [%s] %s | Type: %s | Value: %d | Claimable: %s | Claimed: %s"),
			*SourceLabel,
			*FormatOptionalDisplayName(Entry.DisplayName, TEXT("Unnamed Reward")).ToString(),
			*GetRewardTypeText(Entry.RewardType).ToString(),
			Entry.Value,
			Entry.bCanClaim ? TEXT("Yes") : TEXT("No"),
			Entry.bClaimed ? TEXT("Yes") : TEXT("No")));
	}
}

FString BuildPendingRewardCandidatesSummary(const FFinalRunSnapshot& RunSnapshot)
{
	TArray<FString> Lines;
	Lines.Add(NSLOCTEXT("FinalPrototypeRunDebug", "CandidateSectionTitle", "Visible Pending Reward Candidates").ToString());

	AppendRewardEntryCandidateLines(Lines, TEXT("BattleReward"), RunSnapshot.PendingBattleReward.RewardEntries);
	AppendRewardEntryCandidateLines(Lines, TEXT("RewardNode"), RunSnapshot.PendingRewardNode.RewardEntries);

	for (const FFinalRunEventOptionViewData& Option : RunSnapshot.PendingEventNode.Options)
	{
		const FString OptionLabel = !Option.DisplayText.IsEmpty()
			? Option.DisplayText.ToString()
			: Option.OptionId.ToString();
		AppendRewardEntryCandidateLines(Lines, FString::Printf(TEXT("Event:%s"), *OptionLabel), Option.RewardEntries);
	}

	for (const FFinalRunShopOfferViewData& Offer : RunSnapshot.PendingShopNode.Offers)
	{
		const FString OfferLabel = !Offer.DisplayName.IsEmpty()
			? Offer.DisplayName.ToString()
			: Offer.OfferId.ToString();
		AppendRewardEntryCandidateLines(Lines, FString::Printf(TEXT("Shop:%s"), *OfferLabel), Offer.RewardEntries);
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
	}

	Super::NativeDestruct();
}

void UFinalPrototypeRunDebugScreen::RefreshFromSubsystems()
{
	if (SummaryText == nullptr || MessageText == nullptr || BuildSummaryText == nullptr || CandidateSummaryText == nullptr)
	{
		return;
	}

	const UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	const UFinalGameFlowSubsystem* GameFlowSubsystem = ResolveGameFlowSubsystem();
	const UFinalGameInstance* FinalGameInstance = ResolveFinalGameInstance();

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

	const FString DeckSummary = BuildDeckEntriesSummaryString(RunSnapshot.CurrentBuild.DeckEntries);
	const FString RelicSummary = BuildRelicEntriesSummaryString(RunSnapshot.CurrentBuild.RelicEntries);
	BuildSummaryText->SetText(FText::FromString(FString::Printf(TEXT("%s\n\n%s"), *DeckSummary, *RelicSummary)));

	CandidateSummaryText->SetText(FText::FromString(BuildPendingRewardCandidatesSummary(RunSnapshot)));

	if (RestartRunButton)
	{
		RestartRunButton->SetIsEnabled(true);
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

	BuildSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugBuildSummary"), 11, FLinearColor(0.90f, 0.95f, 1.0f, 1.0f));
	if (UVerticalBoxSlot* BuildSlot = ContentBox->AddChildToVerticalBox(BuildSummaryText))
	{
		BuildSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	CandidateSummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugCandidateSummary"), 10, FLinearColor(0.78f, 0.82f, 0.88f, 1.0f));
	if (UVerticalBoxSlot* CandidateSlot = ContentBox->AddChildToVerticalBox(CandidateSummaryText))
	{
		CandidateSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
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

UFinalGameInstance* UFinalPrototypeRunDebugScreen::ResolveFinalGameInstance() const
{
	return Cast<UFinalGameInstance>(GetGameInstance());
}
