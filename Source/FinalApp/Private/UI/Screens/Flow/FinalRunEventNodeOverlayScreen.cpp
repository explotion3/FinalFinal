#include "UI/Screens/Flow/FinalRunEventNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

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
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "EventNodeMissingRunFlow", "当前无法访问 RunFlowSubsystem，无法提交事件节点选项。"));
		RebuildVisual();
		return;
	}

	const FFinalRunEventOptionViewData* SelectedOption = GetSelectedOptionView(GetCachedSnapshot().PendingEventNode, SelectedOptionIndex);
	if (SelectedOption == nullptr || SelectedOption->OptionId == NAME_None)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "EventNodeMissingSelectedOption", "当前没有可提交的事件选项。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->ResolveEventOption(SelectedOption->OptionId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAccepted
			? NSLOCTEXT("FinalFlowUI", "EventNodeResolveSucceeded", "已转发 ResolveEvent。")
			: NSLOCTEXT("FinalFlowUI", "EventNodeResolveFailed", "ResolveEvent 执行失败。")));
	RebuildVisual();
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

	if (OptionsListText == nullptr)
	{
		OptionsListText = CreateStageLabel(TEXT("EventNodeOverlayOptionsList"), 13);
		ContentBox->InsertChildAt(3, OptionsListText);
	}

	if (SelectedOptionText == nullptr)
	{
		SelectedOptionText = CreateStageLabel(TEXT("EventNodeOverlaySelectedOption"), 13);
		ContentBox->InsertChildAt(4, SelectedOptionText);
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
			NSLOCTEXT("FinalFlowUI", "EventNodeOverlaySummaryText", "流程阶段: {0}\n事件标题: {1}\n事件摘要: {2}\n节点内容存在: {3}\n可解析: {4}\n已解析: {5}\n选项数: {6}\n当前金币: {7} | 遗物数: {8} | 牌库数: {9}"),
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

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (OptionsListText)
	{
		OptionsListText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "EventNodeOverlayOptionsText", "事件选项列表:\n{0}"),
			FText::FromString(BuildEventOptionsSummaryString(PendingEventNode.Options, SelectedOptionIndex))));
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
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"EventNodeOverlayGapText",
			"当前页已优先消费 EventOption.RewardEntryViews 的 PresentationKind / VisualTier / DetailText / IconId，并在缺失时回退到 raw RewardEntries。剩余缺口主要是 richer 布局、长文本滚动、二次确认和更复杂的分支表现。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "EventNodeOverlayFeedbackDefault", "当前页面会把选中的 OptionId 通过 ResolveEvent 转发给 RunFlowSubsystem，由它统一刷新或切页。")));
	}

	if (PreviousOptionButton)
	{
		PreviousOptionButton->SetIsEnabled(PendingEventNode.Options.Num() > 1);
	}

	if (NextOptionButton)
	{
		NextOptionButton->SetIsEnabled(PendingEventNode.Options.Num() > 1);
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
