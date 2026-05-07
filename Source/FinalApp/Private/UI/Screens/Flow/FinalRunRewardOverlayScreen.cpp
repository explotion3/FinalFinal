#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "Styling/CoreStyle.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
FText BuildRewardCandidateMetaText(const FFinalRunRewardEntryViewData& EntryView)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RewardCandidateMetaFromView", "{0} | {1} | 数值 {2}"),
		FormatRewardPresentationKindText(EntryView.PresentationKind),
		FormatRewardVisualTierText(EntryView.VisualTier),
		FText::AsNumber(EntryView.Value));
}

FText BuildRewardCandidateMetaText(const FFinalRunRewardEntry& Entry)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RewardCandidateMetaFromEntry", "{0} | 数值 {1}"),
		FormatRewardTypeText(Entry.RewardType),
		FText::AsNumber(Entry.Value));
}
}

void UFinalRunRewardCandidateEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UFinalRunRewardCandidateEntryWidget::HandleClicked);
	}

	RefreshBoundWidgets();
}

void UFinalRunRewardCandidateEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	if (SelectedVisual)
	{
		SelectedVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UFinalRunRewardCandidateEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	if (SelectedVisual)
	{
		SelectedVisual->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalRunRewardCandidateEntryWidget::ApplyCandidateView(const FFinalRunRewardCandidateEntryViewData& InViewData)
{
	CachedViewData = InViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnCandidateViewApplied(CachedViewData);
}

void UFinalRunRewardCandidateEntryWidget::HandleClicked()
{
	OnCandidateClicked.Broadcast(this);
}

UWidget* UFinalRunRewardCandidateEntryWidget::GetFocusTarget() const
{
	return OptionButton;
}

void UFinalRunRewardCandidateEntryWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	OptionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OptionButton"));
	WidgetTree->RootWidget = OptionButton;

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RewardCandidateRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.09f, 0.10f, 0.08f, 0.92f));
	RootBorder->SetPadding(FMargin(8.0f));
	OptionButton->AddChild(RootBorder);

	UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardCandidateTextBox"));
	RootBorder->SetContent(TextBox);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetAutoWrapText(true);
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 14));
	TextBox->AddChildToVerticalBox(TitleText);

	SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubtitleText"));
	SubtitleText->SetAutoWrapText(true);
	SubtitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(SubtitleText);

	DetailText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailText"));
	DetailText->SetAutoWrapText(true);
	DetailText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(DetailText);

	MetaText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MetaText"));
	MetaText->SetAutoWrapText(true);
	MetaText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(MetaText);

	StateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
	StateText->SetAutoWrapText(true);
	StateText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(StateText);
}

void UFinalRunRewardCandidateEntryWidget::RefreshBoundWidgets()
{
	if (TitleText)
	{
		TitleText->SetText(CachedViewData.Title);
	}
	if (SubtitleText)
	{
		SubtitleText->SetText(CachedViewData.Subtitle);
		SubtitleText->SetVisibility(CachedViewData.Subtitle.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (DetailText)
	{
		DetailText->SetText(CachedViewData.Detail);
		DetailText->SetVisibility(CachedViewData.Detail.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (MetaText)
	{
		MetaText->SetText(CachedViewData.Meta);
		MetaText->SetVisibility(CachedViewData.Meta.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (StateText)
	{
		StateText->SetText(CachedViewData.State);
	}
	if (OptionButton)
	{
		OptionButton->SetIsEnabled(CachedViewData.bEnabled);
	}
	if (IconImage)
	{
		IconImage->SetVisibility(CachedViewData.IconId.IsNone() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (TierVisual)
	{
		TierVisual->SetVisibility(CachedViewData.VisualTier == EFinalRunRewardVisualTier::None ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (SelectedVisual)
	{
		SelectedVisual->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalRunRewardOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (ClaimRewardButton)
	{
		ClaimRewardButton->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardClicked);
	}
	if (ClaimRewardOption0Button)
	{
		ClaimRewardOption0Button->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardOption0Clicked);
	}
	if (ClaimRewardOption1Button)
	{
		ClaimRewardOption1Button->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardOption1Clicked);
	}
	if (ClaimRewardOption2Button)
	{
		ClaimRewardOption2Button->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardOption2Clicked);
	}
	if (SkipRewardButton)
	{
		SkipRewardButton->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleSkipRewardClicked);
	}
	if (OpenNodePageButton)
	{
		OpenNodePageButton->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked);
	}
	if (OpenModalButton)
	{
		OpenModalButton->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenModalClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UFinalRunRewardOverlayScreen::HandleCloseClicked);
	}
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardClicked()
{
	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	if (PendingReward.RewardEntries.Num() == 1)
	{
		HandleClaimRewardOptionClicked(0);
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystem", "当前无法访问 RunFlowSubsystem，无法领取待领奖励。"));
		RebuildVisual();
		return;
	}

	const bool bClaimed = RunFlowSubsystem->ClaimPendingBattleReward();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bClaimed
			? NSLOCTEXT("FinalFlowUI", "RewardClaimSucceeded", "已转发 ClaimPendingBattleReward。")
			: NSLOCTEXT("FinalFlowUI", "RewardClaimFailed", "ClaimPendingBattleReward 执行失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOption0Clicked()
{
	HandleClaimRewardOptionClicked(0);
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOption1Clicked()
{
	HandleClaimRewardOptionClicked(1);
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOption2Clicked()
{
	HandleClaimRewardOptionClicked(2);
}

void UFinalRunRewardOverlayScreen::HandleSkipRewardClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystemForSkip", "当前无法访问 RunFlowSubsystem，无法跳过待领奖励。"));
		RebuildVisual();
		return;
	}

	const bool bSkipped = RunFlowSubsystem->SkipPendingBattleReward();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bSkipped
			? NSLOCTEXT("FinalFlowUI", "RewardSkipSucceeded", "已跳过战后卡牌奖励。")
			: NSLOCTEXT("FinalFlowUI", "RewardSkipFailed", "跳过战后卡牌奖励失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
	}
}

void UFinalRunRewardOverlayScreen::HandleCloseClicked()
{
	RequestCloseOverlay();
}

void UFinalRunRewardOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "RewardModalTitle", "奖励确认占位"),
			NSLOCTEXT("FinalFlowUI", "RewardModalBody", "当前 RootLayout 已支持在 Overlay 之上打开 Modal。后续奖励确认、放弃奖励、二次确认都应落在这一层。"));
	}
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOptionClicked(const int32 RewardIndex)
{
	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	if (!PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardOptionMissing", "当前奖励候选不存在，无法领取。"));
		RebuildVisual();
		return;
	}

	HandleClaimRewardById(PendingReward.RewardEntries[RewardIndex].RewardId);
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardById(const FName RewardId)
{
	if (RewardId.IsNone())
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardOptionMissingRewardId", "当前奖励候选缺少 RewardId，无法领取。"));
		RebuildVisual();
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystemForOption", "当前无法访问 RunFlowSubsystem，无法领取待领奖励。"));
		RebuildVisual();
		return;
	}

	const bool bClaimed = RunFlowSubsystem->ClaimPendingBattleRewardById(RewardId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bClaimed
			? NSLOCTEXT("FinalFlowUI", "RewardOptionClaimSucceeded", "已领取战后卡牌奖励。")
			: NSLOCTEXT("FinalFlowUI", "RewardOptionClaimFailed", "领取战后卡牌奖励失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleCandidateClicked(UFinalRunRewardCandidateEntryWidget* CandidateEntry)
{
	if (CandidateEntry == nullptr)
	{
		return;
	}

	HandleClaimRewardById(CandidateEntry->GetCandidateViewData().RewardId);
}

void UFinalRunRewardOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.06f, 0.08f, 0.11f, 0.96f), TEXT("RewardOverlayRoot"), TEXT("RewardOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (SourceText == nullptr)
	{
		SourceText = CreateStageLabel(TEXT("RewardOverlaySource"), 12);
		ContentBox->InsertChildAt(2, SourceText);
	}

	if (GoldText == nullptr)
	{
		GoldText = CreateStageLabel(TEXT("RewardOverlayGold"), 12);
		ContentBox->InsertChildAt(3, GoldText);
	}

	if (CandidateListBox == nullptr)
	{
		CandidateListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CandidateListBox"));
		ContentBox->InsertChildAt(4, CandidateListBox);
	}

	if (RewardEntriesText == nullptr)
	{
		RewardEntriesText = CreateStageLabel(TEXT("RewardOverlayEntries"), 13);
		ContentBox->InsertChildAt(5, RewardEntriesText);
	}

	if (SkipRewardButton == nullptr)
	{
		SkipRewardButton = CreateStageButton(
			TEXT("RewardOverlaySkipButton"),
			TEXT("RewardOverlaySkipButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardSkipButton", "跳过卡牌奖励"),
			SkipRewardButtonText);
		SkipRewardButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleSkipRewardClicked);
		ContentBox->AddChildToVerticalBox(SkipRewardButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("RewardOverlayCloseButton"),
			TEXT("RewardOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardCloseButton", "关闭奖励页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunRewardOverlayScreen::RebuildVisual()
{
	ClearFocusableWidgets();

	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingBattleRewardViewData& PendingReward = Snapshot.PendingBattleReward;
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RewardOverlayTitleText", "战后奖励"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardOverlaySummaryTextReadable", "{0} | {1} 个候选 | 当前牌库 {2} 张"),
			FormatBattleOutcomeText(PendingReward.SourceBattleOutcome),
			FText::AsNumber(PendingReward.RewardEntryViews.Num() > 0 ? PendingReward.RewardEntryViews.Num() : PendingReward.RewardEntries.Num()),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (SourceText)
	{
		SourceText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardOverlaySourceText", "来源：{0} / {1}"),
			FormatOptionalText(
				PendingReward.SourceNodeDisplayName,
				FormatOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNodeDisplayName", "未公开节点"))),
			FormatNodeTypeText(PendingReward.SourceNodeType)));
	}

	if (GoldText)
	{
		GoldText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardOverlayGoldText", "金币 +{0}（已入账） | 当前金币 {1} | 遗物 {2}"),
			FText::AsNumber(PendingReward.RewardGold),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount)));
	}

	if (RewardEntriesText)
	{
		RewardEntriesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardEntriesText", "候选明细:\n{0}"),
			FText::FromString(BuildRewardPresentationSummaryString(PendingReward.RewardEntryViews, PendingReward.RewardEntries))));
		RewardEntriesText->SetVisibility(ESlateVisibility::Collapsed);
	}

	RebuildCandidateList();

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardOverlayGapText",
			"战后金币已自动入账；请选择 1 张卡牌奖励，或跳过。"));
	}

	if (FeedbackText)
	{
		RefreshFeedbackText(NSLOCTEXT("FinalFlowUI", "RewardOverlayFeedbackDefault", "请选择一项战后卡牌奖励，或跳过本次卡牌奖励。"));
	}

	if (ClaimRewardButton)
	{
		ClaimRewardButton->SetVisibility(ESlateVisibility::Collapsed);
		const bool bCanClaimReward = PendingReward.bHasPendingReward
			&& PendingReward.bCanClaim
			&& Progression.bCanClaimPendingBattleReward
			&& PendingReward.RewardEntries.Num() == 1;
		ClaimRewardButton->SetIsEnabled(bCanClaimReward);
		RegisterFocusableWidget(ClaimRewardButton);
	}

	if (ClaimRewardButtonText)
	{
		ClaimRewardButtonText->SetText(PendingReward.RewardEntries.Num() == 1
			? NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取当前奖励条目")
			: NSLOCTEXT("FinalFlowUI", "RewardClaimButtonSelectBelow", "请选择一个卡牌奖励"));
	}

	auto ConfigureOptionButton = [&PendingReward, &Progression](const int32 OptionIndex, UButton* Button, UTextBlock* ButtonText)
	{
		const bool bHasOption = PendingReward.RewardEntryViews.IsValidIndex(OptionIndex) || PendingReward.RewardEntries.IsValidIndex(OptionIndex);
		const bool bCanChoose = PendingReward.bHasPendingReward
			&& PendingReward.bCanClaim
			&& Progression.bCanClaimPendingBattleReward
			&& bHasOption
			&& PendingReward.RewardEntries.IsValidIndex(OptionIndex)
			&& !PendingReward.RewardEntries[OptionIndex].RewardId.IsNone();

		if (Button)
		{
			Button->SetVisibility(ESlateVisibility::Collapsed);
			Button->SetIsEnabled(bCanChoose);
		}

		if (ButtonText)
		{
			if (PendingReward.RewardEntryViews.IsValidIndex(OptionIndex))
			{
				const FFinalRunRewardEntryViewData& EntryView = PendingReward.RewardEntryViews[OptionIndex];
				ButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "RewardClaimOptionButtonWithView", "选择 {0}: {1} - {2}"),
					FText::AsNumber(OptionIndex + 1),
					FormatRewardEntryViewPrimaryText(EntryView),
					EntryView.SecondaryText.IsEmpty() ? EntryView.DetailText : EntryView.SecondaryText));
			}
			else if (PendingReward.RewardEntries.IsValidIndex(OptionIndex))
			{
				const FFinalRunRewardEntry& Entry = PendingReward.RewardEntries[OptionIndex];
				ButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "RewardClaimOptionButtonWithEntry", "选择 {0}: {1}"),
					FText::AsNumber(OptionIndex + 1),
					FormatRewardEntryName(Entry)));
			}
			else
			{
				ButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "RewardClaimOptionButtonMissing", "奖励 {0}: 无候选"),
					FText::AsNumber(OptionIndex + 1)));
			}
		}
	};

	ConfigureOptionButton(0, ClaimRewardOption0Button, ClaimRewardOption0ButtonText);
	ConfigureOptionButton(1, ClaimRewardOption1Button, ClaimRewardOption1ButtonText);
	ConfigureOptionButton(2, ClaimRewardOption2Button, ClaimRewardOption2ButtonText);

	if (SkipRewardButton)
	{
		SkipRewardButton->SetIsEnabled(PendingReward.bHasPendingReward && PendingReward.bCanClaim && Progression.bCanClaimPendingBattleReward);
		RegisterFocusableWidget(SkipRewardButton);
	}

	if (SkipRewardButtonText)
	{
		SkipRewardButtonText->SetText(PendingReward.bHasPendingReward
			? NSLOCTEXT("FinalFlowUI", "RewardSkipButton", "跳过卡牌奖励")
			: NSLOCTEXT("FinalFlowUI", "RewardSkipButtonDisabled", "当前没有可跳过的卡牌奖励"));
	}

	if (OpenNodePageButton)
	{
		OpenNodePageButton->SetVisibility(ESlateVisibility::Collapsed);
		OpenNodePageButton->SetIsEnabled(Progression.bCanAdvanceToNextNode || Progression.CurrentNodeId != NAME_None);
		RegisterFocusableWidget(OpenNodePageButton);
	}

	if (OpenModalButton)
	{
		OpenModalButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (CloseButton)
	{
		CloseButton->SetVisibility(ESlateVisibility::Visible);
		CloseButton->SetIsEnabled(true);
		RegisterFocusableWidget(CloseButton);
	}

	FocusFirstAvailableAction();
}

void UFinalRunRewardOverlayScreen::RebuildCandidateList()
{
	if (WidgetTree == nullptr || CandidateListBox == nullptr)
	{
		return;
	}

	CandidateListBox->ClearChildren();

	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	const int32 CandidateCount = FMath::Min(
		3,
		FMath::Max(PendingReward.RewardEntries.Num(), PendingReward.RewardEntryViews.Num()));
	if (CandidateCount <= 0)
	{
		CandidateListBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const TSubclassOf<UFinalRunRewardCandidateEntryWidget> ConfiguredEntryClass = UFinalUIWidgetClassSettings::GetRunRewardCandidateEntryWidgetClass();
	UClass* EntryClass = ConfiguredEntryClass.Get() ? ConfiguredEntryClass.Get() : UFinalRunRewardCandidateEntryWidget::StaticClass();
	for (int32 RewardIndex = 0; RewardIndex < CandidateCount; ++RewardIndex)
	{
		UFinalRunRewardCandidateEntryWidget* CandidateEntry = WidgetTree->ConstructWidget<UFinalRunRewardCandidateEntryWidget>(
			EntryClass,
			*FString::Printf(TEXT("RunRewardCandidate_%d_%s"), RewardIndex, *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
		if (CandidateEntry == nullptr)
		{
			continue;
		}

		CandidateEntry->OnCandidateClicked.AddUObject(this, &UFinalRunRewardOverlayScreen::HandleCandidateClicked);
		CandidateEntry->ApplyCandidateView(BuildCandidateEntryData(RewardIndex));
		RegisterFocusableWidget(CandidateEntry->GetFocusTarget());
		if (UVerticalBoxSlot* CandidateSlot = CandidateListBox->AddChildToVerticalBox(CandidateEntry))
		{
			CandidateSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}

	CandidateListBox->SetVisibility(CandidateListBox->GetChildrenCount() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

FFinalRunRewardCandidateEntryViewData UFinalRunRewardOverlayScreen::BuildCandidateEntryData(const int32 RewardIndex) const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingBattleRewardViewData& PendingReward = Snapshot.PendingBattleReward;

	FFinalRunRewardCandidateEntryViewData Data;
	Data.RewardIndex = RewardIndex;
	Data.bEnabled = PendingReward.bHasPendingReward
		&& PendingReward.bCanClaim
		&& Snapshot.Progression.bCanClaimPendingBattleReward
		&& PendingReward.RewardEntries.IsValidIndex(RewardIndex)
		&& !PendingReward.RewardEntries[RewardIndex].RewardId.IsNone();
	Data.State = Data.bEnabled
		? NSLOCTEXT("FinalFlowUI", "RewardCandidateStateEnabled", "可领取")
		: NSLOCTEXT("FinalFlowUI", "RewardCandidateStateDisabled", "暂不可领取");

	if (PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntry& RewardEntry = PendingReward.RewardEntries[RewardIndex];
		Data.RewardId = RewardEntry.RewardId;
		Data.Title = FormatRewardEntryName(RewardEntry);
		Data.Meta = BuildRewardCandidateMetaText(RewardEntry);
	}

	if (PendingReward.RewardEntryViews.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntryViewData& EntryView = PendingReward.RewardEntryViews[RewardIndex];
		Data.Title = FormatRewardEntryViewPrimaryText(EntryView);
		Data.Subtitle = EntryView.SecondaryText;
		Data.Detail = EntryView.DetailText;
		Data.Meta = BuildRewardCandidateMetaText(EntryView);
		Data.IconId = EntryView.IconId;
		Data.PresentationKind = EntryView.PresentationKind;
		Data.VisualTier = EntryView.VisualTier;
	}

	if (Data.Title.IsEmpty())
	{
		Data.Title = FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardCandidateFallbackTitle", "奖励 {0}"),
			FText::AsNumber(RewardIndex + 1));
	}

	return Data;
}
