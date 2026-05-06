#include "UI/Screens/Flow/FinalRunEventNodeOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "Styling/CoreStyle.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
const FFinalRunEventOptionViewData* GetSelectedOptionView(const FFinalRunPendingEventNodeViewData& PendingEventNode, const int32 SelectedOptionIndex)
{
	return PendingEventNode.Options.IsValidIndex(SelectedOptionIndex)
		? &PendingEventNode.Options[SelectedOptionIndex]
		: nullptr;
}

FString BuildEventOptionsSummaryString(const TArray<FFinalRunEventOptionViewData>& Options, const int32 SelectedOptionIndex)
{
	if (Options.Num() <= 0)
	{
		return NSLOCTEXT("FinalFlowUI", "EventNodeOptionsEmpty", "当前没有公开的事件选项。").ToString();
	}

	FString OptionsSummary;
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		const FFinalRunEventOptionViewData& Option = Options[Index];
		OptionsSummary += FString::Printf(
			TEXT("%s[%d] %s | OptionId: %s | 可选择: %s"),
			Index == SelectedOptionIndex ? TEXT("> ") : TEXT("  "),
			Index + 1,
			*FormatOptionalText(Option.DisplayText, NSLOCTEXT("FinalFlowUI", "EventNodeOptionUnnamed", "未命名选项")).ToString(),
			Option.OptionId != NAME_None ? *Option.OptionId.ToString() : TEXT("None"),
			Option.bSelectable ? TEXT("是") : TEXT("否"));

		if (!Option.AvailabilityMessage.IsEmpty())
		{
			OptionsSummary += FString::Printf(TEXT(" | 限制: %s"), *Option.AvailabilityMessage.ToString());
		}

		const int32 RewardEntryCount = Option.RewardEntryViews.Num() > 0 ? Option.RewardEntryViews.Num() : Option.RewardEntries.Num();
		OptionsSummary += FString::Printf(TEXT(" | 奖励条目数: %d\n"), RewardEntryCount);
	}

	OptionsSummary.TrimEndInline();
	return OptionsSummary;
}

FText BuildEventOptionMetaText(const FFinalRunEventOptionViewData& Option)
{
	const int32 RewardEntryCount = Option.RewardEntryViews.Num() > 0 ? Option.RewardEntryViews.Num() : Option.RewardEntries.Num();
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "EventOptionMetaText", "奖励条目 {0}"),
		FText::AsNumber(RewardEntryCount));
}

FName FindFirstEventOptionIconId(const FFinalRunEventOptionViewData& Option)
{
	for (const FFinalRunRewardEntryViewData& RewardEntryView : Option.RewardEntryViews)
	{
		if (RewardEntryView.IconId != NAME_None)
		{
			return RewardEntryView.IconId;
		}
	}
	return NAME_None;
}

EFinalRunRewardPresentationKind FindFirstEventOptionPresentationKind(const FFinalRunEventOptionViewData& Option)
{
	return Option.RewardEntryViews.Num() > 0
		? Option.RewardEntryViews[0].PresentationKind
		: EFinalRunRewardPresentationKind::None;
}

EFinalRunRewardVisualTier FindFirstEventOptionVisualTier(const FFinalRunEventOptionViewData& Option)
{
	return Option.RewardEntryViews.Num() > 0
		? Option.RewardEntryViews[0].VisualTier
		: EFinalRunRewardVisualTier::None;
}
}

void UFinalRunEventOptionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();

	if (OptionButton)
	{
		OptionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunEventOptionEntryWidget::HandleClicked);
	}

	RefreshBoundWidgets();
}

void UFinalRunEventOptionEntryWidget::ApplyOptionView(const FFinalRunEventOptionEntryViewData& InViewData)
{
	CachedViewData = InViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnOptionViewApplied(CachedViewData);
}

void UFinalRunEventOptionEntryWidget::HandleClicked()
{
	OnOptionClicked.Broadcast(this);
}

void UFinalRunEventOptionEntryWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	OptionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OptionButton"));
	WidgetTree->RootWidget = OptionButton;

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EventOptionRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.08f, 0.07f, 0.11f, 0.94f));
	RootBorder->SetPadding(FMargin(8.0f));
	OptionButton->AddChild(RootBorder);

	UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EventOptionTextBox"));
	RootBorder->SetContent(TextBox);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetAutoWrapText(true);
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 14));
	TextBox->AddChildToVerticalBox(TitleText);

	DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DescriptionText"));
	DescriptionText->SetAutoWrapText(true);
	DescriptionText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(DescriptionText);

	PreviewRewardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PreviewRewardText"));
	PreviewRewardText->SetAutoWrapText(true);
	PreviewRewardText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(PreviewRewardText);

	CostText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CostText"));
	CostText->SetAutoWrapText(true);
	CostText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(CostText);

	StateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
	StateText->SetAutoWrapText(true);
	StateText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(StateText);

	DisabledReasonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DisabledReasonText"));
	DisabledReasonText->SetAutoWrapText(true);
	DisabledReasonText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(DisabledReasonText);
}

void UFinalRunEventOptionEntryWidget::RefreshBoundWidgets()
{
	if (TitleText)
	{
		TitleText->SetText(CachedViewData.Title);
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(CachedViewData.Description);
		DescriptionText->SetVisibility(CachedViewData.Description.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (PreviewRewardText)
	{
		PreviewRewardText->SetText(CachedViewData.PreviewRewardText);
		PreviewRewardText->SetVisibility(CachedViewData.PreviewRewardText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (CostText)
	{
		CostText->SetText(CachedViewData.CostText);
		CostText->SetVisibility(CachedViewData.CostText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (StateText)
	{
		StateText->SetText(CachedViewData.StateText);
	}
	if (DisabledReasonText)
	{
		DisabledReasonText->SetText(CachedViewData.DisabledReason);
		DisabledReasonText->SetVisibility(CachedViewData.DisabledReason.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
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

void UFinalRunEventNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunEventNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	NormalizeSelectedOptionIndex();
	RebuildVisual();
}

void UFinalRunEventNodeOverlayScreen::HandlePreviousOptionClicked()
{
	StepSelectedOption(-1);
}

void UFinalRunEventNodeOverlayScreen::HandleNextOptionClicked()
{
	StepSelectedOption(1);
}

void UFinalRunEventNodeOverlayScreen::HandleResolveOptionClicked()
{
	const FFinalRunEventOptionViewData* SelectedOption = GetSelectedOptionView(GetCachedSnapshot().PendingEventNode, SelectedOptionIndex);
	if (SelectedOption == nullptr || SelectedOption->OptionId == NAME_None)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "EventNodeMissingSelectedOption", "当前没有可提交的事件选项。"));
		RebuildVisual();
		return;
	}

	HandleResolveOptionById(SelectedOption->OptionId);
}

void UFinalRunEventNodeOverlayScreen::HandleResolveOptionById(const FName OptionId)
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "EventNodeMissingRunFlow", "当前无法访问 RunFlowSubsystem，无法提交事件节点选项。"));
		RebuildVisual();
		return;
	}

	if (OptionId == NAME_None)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "EventNodeMissingOptionId", "当前没有可提交的事件选项。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->ResolveEventOption(OptionId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAccepted
			? NSLOCTEXT("FinalFlowUI", "EventNodeResolveSucceeded", "已转发 ResolveEvent。")
			: NSLOCTEXT("FinalFlowUI", "EventNodeResolveFailed", "ResolveEvent 执行失败。")));
	RebuildVisual();
}

void UFinalRunEventNodeOverlayScreen::HandleOptionClicked(UFinalRunEventOptionEntryWidget* OptionEntry)
{
	if (OptionEntry == nullptr)
	{
		return;
	}

	HandleResolveOptionById(OptionEntry->GetOptionViewData().OptionId);
}

void UFinalRunEventNodeOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunEventNodeOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.08f, 0.06f, 0.10f, 0.96f), TEXT("EventNodeOverlayRoot"), TEXT("EventNodeOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (CurrentNodeText == nullptr)
	{
		CurrentNodeText = CreateStageLabel(TEXT("EventNodeOverlayCurrentNode"), 13);
		ContentBox->InsertChildAt(2, CurrentNodeText);
	}
	if (NodeText == nullptr)
	{
		NodeText = CurrentNodeText;
	}

	if (OptionListBox == nullptr)
	{
		OptionListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionListBox"));
		ContentBox->InsertChildAt(3, OptionListBox);
	}

	if (RewardPreviewBox == nullptr)
	{
		RewardPreviewBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardPreviewBox"));
		ContentBox->InsertChildAt(4, RewardPreviewBox);
	}

	if (OptionsListText == nullptr)
	{
		OptionsListText = CreateStageLabel(TEXT("EventNodeOverlayOptionsList"), 13);
		OptionsListText->SetVisibility(ESlateVisibility::Collapsed);
		ContentBox->InsertChildAt(5, OptionsListText);
	}

	if (SelectedOptionText == nullptr)
	{
		SelectedOptionText = CreateStageLabel(TEXT("EventNodeOverlaySelectedOption"), 13);
		SelectedOptionText->SetVisibility(ESlateVisibility::Collapsed);
		ContentBox->InsertChildAt(6, SelectedOptionText);
	}

	if (PreviousOptionButton == nullptr)
	{
		PreviousOptionButton = CreateStageButton(
			TEXT("EventNodeOverlayPrevOptionButton"),
			TEXT("EventNodeOverlayPrevOptionButtonText"),
			NSLOCTEXT("FinalFlowUI", "EventNodePrevOptionButton", "上一项"),
			PreviousOptionButtonText);
		PreviousOptionButton->OnClicked.AddDynamic(this, &UFinalRunEventNodeOverlayScreen::HandlePreviousOptionClicked);
		ContentBox->AddChildToVerticalBox(PreviousOptionButton);
	}

	if (NextOptionButton == nullptr)
	{
		NextOptionButton = CreateStageButton(
			TEXT("EventNodeOverlayNextOptionButton"),
			TEXT("EventNodeOverlayNextOptionButtonText"),
			NSLOCTEXT("FinalFlowUI", "EventNodeNextOptionButton", "下一项"),
			NextOptionButtonText);
		NextOptionButton->OnClicked.AddDynamic(this, &UFinalRunEventNodeOverlayScreen::HandleNextOptionClicked);
		ContentBox->AddChildToVerticalBox(NextOptionButton);
	}

	if (ResolveOptionButton == nullptr)
	{
		ResolveOptionButton = CreateStageButton(
			TEXT("EventNodeOverlayResolveButton"),
			TEXT("EventNodeOverlayResolveButtonText"),
			NSLOCTEXT("FinalFlowUI", "EventNodeResolveButton", "确认当前事件选项"),
			ResolveOptionButtonText);
		ResolveOptionButton->OnClicked.AddDynamic(this, &UFinalRunEventNodeOverlayScreen::HandleResolveOptionClicked);
		ContentBox->AddChildToVerticalBox(ResolveOptionButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("EventNodeOverlayCloseButton"),
			TEXT("EventNodeOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "EventNodeCloseButton", "关闭事件节点页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunEventNodeOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunEventNodeOverlayScreen::NormalizeSelectedOptionIndex()
{
	const TArray<FFinalRunEventOptionViewData>& Options = GetCachedSnapshot().PendingEventNode.Options;
	if (Options.Num() <= 0)
	{
		SelectedOptionIndex = INDEX_NONE;
		return;
	}

	if (!Options.IsValidIndex(SelectedOptionIndex))
	{
		const int32 FirstSelectableIndex = Options.IndexOfByPredicate([](const FFinalRunEventOptionViewData& Option)
		{
			return Option.OptionId != NAME_None;
		});

		SelectedOptionIndex = FirstSelectableIndex != INDEX_NONE ? FirstSelectableIndex : 0;
	}
}

void UFinalRunEventNodeOverlayScreen::StepSelectedOption(const int32 Direction)
{
	const int32 OptionCount = GetCachedSnapshot().PendingEventNode.Options.Num();
	if (OptionCount <= 0)
	{
		SelectedOptionIndex = INDEX_NONE;
		RebuildVisual();
		return;
	}

	NormalizeSelectedOptionIndex();
	SelectedOptionIndex = (SelectedOptionIndex + Direction + OptionCount) % OptionCount;
	RebuildVisual();
}

void UFinalRunEventNodeOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingEventNodeViewData& PendingEventNode = Snapshot.PendingEventNode;
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;
	const FFinalRunEventOptionViewData* SelectedOption = GetSelectedOptionView(PendingEventNode, SelectedOptionIndex);

	if (TitleText)
	{
		TitleText->SetText(FormatOptionalText(
			PendingEventNode.Title,
			NSLOCTEXT("FinalFlowUI", "EventNodeOverlayTitleText", "事件节点页")));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "EventNodeOverlaySummaryText", "{1}\n\n{2}\n\n选项 {6} | 当前金币 {7}"),
			FormatFlowStageText(Progression.FlowStage),
			FormatOptionalText(PendingEventNode.Title, NSLOCTEXT("FinalFlowUI", "EventNodeNoTitle", "未公开标题")),
			FormatOptionalText(PendingEventNode.Summary, NSLOCTEXT("FinalFlowUI", "EventNodeNoSummary", "当前没有额外摘要说明。")),
			FormatBool(PendingEventNode.bHasPendingContent),
			FormatBool(PendingEventNode.bCanResolve),
			FormatBool(PendingEventNode.bResolved),
			FText::AsNumber(PendingEventNode.Options.Num()),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (NodeText)
	{
		NodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (OptionsListText)
	{
		OptionsListText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "EventNodeOverlayOptionsText", "事件选项列表:\n{0}"),
			FText::FromString(BuildEventOptionsSummaryString(PendingEventNode.Options, SelectedOptionIndex))));
		OptionsListText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SelectedOptionText)
	{
		if (SelectedOption == nullptr)
		{
			SelectedOptionText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeSelectedOptionMissing", "当前没有可预览的事件选项。"));
		}
		else
		{
			SelectedOptionText->SetText(FText::Format(
				NSLOCTEXT("FinalFlowUI", "EventNodeSelectedOptionText", "当前选中选项: {0}\nOptionId: {1}\nOutcomeSummary: {2}\n可选择: {3}\n可用性说明: {4}\n奖励条目:\n{5}"),
				FormatOptionalText(SelectedOption->DisplayText, NSLOCTEXT("FinalFlowUI", "EventNodeSelectedOptionNoText", "未公开选项文案")),
				FormatOptionalName(SelectedOption->OptionId, NSLOCTEXT("FinalFlowUI", "EventNodeSelectedOptionNoId", "None")),
				FormatOptionalText(SelectedOption->OutcomeSummary, NSLOCTEXT("FinalFlowUI", "EventNodeSelectedOptionNoOutcome", "当前没有公开结果摘要。")),
				FormatBool(SelectedOption->bSelectable),
				FormatOptionalText(SelectedOption->AvailabilityMessage, NSLOCTEXT("FinalFlowUI", "EventNodeSelectedOptionNoAvailability", "当前没有额外限制说明。")),
				FText::FromString(BuildRewardPresentationSummaryString(SelectedOption->RewardEntryViews, SelectedOption->RewardEntries))));
		}
		SelectedOptionText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeOverlayGapText", "选择一个事件选项。不可选项会保留显示，并给出阻塞原因。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "EventNodeOverlayFeedbackDefault", "事件选项会通过 RunFlowSubsystem 提交，由 FinalRun 统一校验和结算。")));
	}

	RebuildOptionList();

	if (PreviousOptionButton)
	{
		PreviousOptionButton->SetIsEnabled(PendingEventNode.Options.Num() > 1);
		PreviousOptionButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (NextOptionButton)
	{
		NextOptionButton->SetIsEnabled(PendingEventNode.Options.Num() > 1);
		NextOptionButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ResolveOptionButton)
	{
		ResolveOptionButton->SetIsEnabled(
			PendingEventNode.bHasPendingContent
			&& PendingEventNode.bCanResolve
			&& !PendingEventNode.bResolved
			&& SelectedOption != nullptr
			&& SelectedOption->OptionId != NAME_None
			&& SelectedOption->bSelectable);
		ResolveOptionButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ResolveOptionButtonText)
	{
		if (!PendingEventNode.bHasPendingContent)
		{
			ResolveOptionButtonText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeResolveButtonMissing", "当前没有待处理事件节点内容"));
		}
		else if (PendingEventNode.bResolved)
		{
			ResolveOptionButtonText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeResolveButtonResolved", "当前事件节点已解析"));
		}
		else if (SelectedOption == nullptr || SelectedOption->OptionId == NAME_None)
		{
			ResolveOptionButtonText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeResolveButtonNoOption", "当前没有可提交的事件选项"));
		}
		else if (!PendingEventNode.bCanResolve || !SelectedOption->bSelectable)
		{
			ResolveOptionButtonText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeResolveButtonBlocked", "当前选项暂不可提交"));
		}
		else
		{
			ResolveOptionButtonText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeResolveButton", "确认当前事件选项"));
		}
	}
}

void UFinalRunEventNodeOverlayScreen::RebuildOptionList()
{
	if (OptionListBox == nullptr || WidgetTree == nullptr)
	{
		return;
	}

	OptionListBox->ClearChildren();
	const FFinalRunPendingEventNodeViewData& PendingEventNode = GetCachedSnapshot().PendingEventNode;
	const TSubclassOf<UFinalRunEventOptionEntryWidget> ConfiguredEntryClass = UFinalUIWidgetClassSettings::GetRunEventOptionEntryWidgetClass();
	UClass* EntryClass = ConfiguredEntryClass.Get() ? ConfiguredEntryClass.Get() : UFinalRunEventOptionEntryWidget::StaticClass();

	for (int32 OptionIndex = 0; OptionIndex < PendingEventNode.Options.Num(); ++OptionIndex)
	{
		UFinalRunEventOptionEntryWidget* OptionEntry = WidgetTree->ConstructWidget<UFinalRunEventOptionEntryWidget>(
			EntryClass,
			FName(*FString::Printf(TEXT("EventOptionEntry_%d"), OptionIndex)));
		if (OptionEntry == nullptr)
		{
			continue;
		}

		OptionEntry->OnOptionClicked.AddUObject(this, &UFinalRunEventNodeOverlayScreen::HandleOptionClicked);
		OptionEntry->ApplyOptionView(BuildOptionEntryData(OptionIndex));

		UVerticalBoxSlot* EntrySlot = OptionListBox->AddChildToVerticalBox(OptionEntry);
		if (EntrySlot)
		{
			EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}

	OptionListBox->SetVisibility(PendingEventNode.Options.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

FFinalRunEventOptionEntryViewData UFinalRunEventNodeOverlayScreen::BuildOptionEntryData(const int32 OptionIndex) const
{
	FFinalRunEventOptionEntryViewData EntryData;
	const FFinalRunPendingEventNodeViewData& PendingEventNode = GetCachedSnapshot().PendingEventNode;
	if (!PendingEventNode.Options.IsValidIndex(OptionIndex))
	{
		EntryData.Title = NSLOCTEXT("FinalFlowUI", "EventOptionEntryInvalid", "无效事件选项");
		EntryData.StateText = NSLOCTEXT("FinalFlowUI", "EventOptionEntryInvalidState", "不可选择");
		return EntryData;
	}

	const FFinalRunEventOptionViewData& Option = PendingEventNode.Options[OptionIndex];
	EntryData.OptionId = Option.OptionId;
	EntryData.OptionIndex = OptionIndex;
	EntryData.Title = FormatOptionalText(
		Option.DisplayText,
		FormatOptionalName(Option.OptionId, NSLOCTEXT("FinalFlowUI", "EventOptionEntryTitleFallback", "未命名选项")));
	EntryData.Description = FormatOptionalText(
		Option.OutcomeSummary,
		NSLOCTEXT("FinalFlowUI", "EventOptionEntryDescriptionFallback", "无额外结果说明。"));
	EntryData.PreviewRewardText = FText::FromString(BuildRewardPresentationSummaryString(Option.RewardEntryViews, Option.RewardEntries));
	EntryData.CostText = BuildEventOptionMetaText(Option);
	EntryData.DisabledReason = Option.AvailabilityMessage;
	EntryData.IconId = FindFirstEventOptionIconId(Option);
	EntryData.PresentationKind = FindFirstEventOptionPresentationKind(Option);
	EntryData.VisualTier = FindFirstEventOptionVisualTier(Option);
	EntryData.bEnabled = PendingEventNode.bHasPendingContent
		&& PendingEventNode.bCanResolve
		&& !PendingEventNode.bResolved
		&& Option.bSelectable
		&& Option.OptionId != NAME_None;
	EntryData.StateText = EntryData.bEnabled
		? NSLOCTEXT("FinalFlowUI", "EventOptionEntryEnabled", "可选择")
		: FormatOptionalText(Option.AvailabilityMessage, NSLOCTEXT("FinalFlowUI", "EventOptionEntryDisabled", "暂不可选择"));
	return EntryData;
}
