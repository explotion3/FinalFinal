#include "UI/Screens/Flow/FinalRunEventNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

void UFinalRunEventNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunEventNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	RebuildVisual();
}

void UFinalRunEventNodeOverlayScreen::HandleOpenNodeSelectClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
	}
}

void UFinalRunEventNodeOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "EventNodeModalTitle", "事件节点页占位"),
			NSLOCTEXT("FinalFlowUI", "EventNodeModalBody", "当前已拆出独立的事件节点页挂点。后续若 FinalRun 提供事件文案、选项列表、条件与代价查询，这一页应承接事件节点的主界面。"));
	}
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

	if (MissingFieldsText == nullptr)
	{
		MissingFieldsText = CreateStageLabel(TEXT("EventNodeOverlayMissingFields"), 13);
		ContentBox->InsertChildAt(3, MissingFieldsText);
	}

	if (OpenNodeSelectButton == nullptr)
	{
		OpenNodeSelectButton = CreateStageButton(
			TEXT("EventNodeOverlayNodeSelectButton"),
			TEXT("EventNodeOverlayNodeSelectButtonText"),
			NSLOCTEXT("FinalFlowUI", "EventNodeOpenNodeSelectButton", "打开节点选择页"),
			OpenNodeSelectButtonText);
		OpenNodeSelectButton->OnClicked.AddDynamic(this, &UFinalRunEventNodeOverlayScreen::HandleOpenNodeSelectClicked);
		ContentBox->AddChildToVerticalBox(OpenNodeSelectButton);
	}

	if (OpenModalButton == nullptr)
	{
		OpenModalButton = CreateStageButton(
			TEXT("EventNodeOverlayModalButton"),
			TEXT("EventNodeOverlayModalButtonText"),
			NSLOCTEXT("FinalFlowUI", "EventNodeOpenModalButton", "打开事件节点说明模态"),
			OpenModalButtonText);
		OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunEventNodeOverlayScreen::HandleOpenModalClicked);
		ContentBox->AddChildToVerticalBox(OpenModalButton);
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

void UFinalRunEventNodeOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "EventNodeOverlayTitleText", "事件节点页"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "EventNodeOverlaySummaryText", "流程阶段: {0}\n当前金币: {1} | 遗物数: {2} | 牌库数: {3}\n当前节点已访问: {4}"),
			FormatFlowStageText(Progression.FlowStage),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount),
			FormatBool(Progression.bCurrentNodeVisited)));
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (MissingFieldsText)
	{
		MissingFieldsText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"EventNodeOverlayMissingFieldsText",
			"当前事件节点页已经从通用节点页拆出，但仍缺事件节点专用查询：事件正文、选项列表、可见条件、代价摘要和选项确认命令。"));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"EventNodeOverlayGapText",
			"这是一张结构化占位页：它专门承接 PendingEventNode，让流程不会再挤进节点选择页，也不会在 FinalApp 内伪造事件解析逻辑。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "EventNodeOverlayFeedbackDefault", "等待 Event Node 专用查询与命令接入。")));
	}

	if (OpenNodeSelectButton)
	{
		OpenNodeSelectButton->SetIsEnabled(Progression.AvailableNextNodes.Num() > 0 || Progression.bCanAdvanceToNextNode);
	}
}
