#include "UI/Screens/Flow/FinalRunShopNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

void UFinalRunShopNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::HandleOpenNodeSelectClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
	}
}

void UFinalRunShopNodeOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "ShopNodeModalTitle", "商店节点页占位"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeModalBody", "当前已拆出独立的商店节点页挂点。后续若 FinalRun 提供商品列表、价格、刷新规则和购买命令，这一页应承接商店节点主界面。"));
	}
}

void UFinalRunShopNodeOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunShopNodeOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.11f, 0.08f, 0.05f, 0.96f), TEXT("ShopNodeOverlayRoot"), TEXT("ShopNodeOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (CurrentNodeText == nullptr)
	{
		CurrentNodeText = CreateStageLabel(TEXT("ShopNodeOverlayCurrentNode"), 13);
		ContentBox->InsertChildAt(2, CurrentNodeText);
	}

	if (MissingFieldsText == nullptr)
	{
		MissingFieldsText = CreateStageLabel(TEXT("ShopNodeOverlayMissingFields"), 13);
		ContentBox->InsertChildAt(3, MissingFieldsText);
	}

	if (OpenNodeSelectButton == nullptr)
	{
		OpenNodeSelectButton = CreateStageButton(
			TEXT("ShopNodeOverlayNodeSelectButton"),
			TEXT("ShopNodeOverlayNodeSelectButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeOpenNodeSelectButton", "打开节点选择页"),
			OpenNodeSelectButtonText);
		OpenNodeSelectButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleOpenNodeSelectClicked);
		ContentBox->AddChildToVerticalBox(OpenNodeSelectButton);
	}

	if (OpenModalButton == nullptr)
	{
		OpenModalButton = CreateStageButton(
			TEXT("ShopNodeOverlayModalButton"),
			TEXT("ShopNodeOverlayModalButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeOpenModalButton", "打开商店节点说明模态"),
			OpenModalButtonText);
		OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleOpenModalClicked);
		ContentBox->AddChildToVerticalBox(OpenModalButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("ShopNodeOverlayCloseButton"),
			TEXT("ShopNodeOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeCloseButton", "关闭商店节点页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunShopNodeOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayTitleText", "商店节点页"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "ShopNodeOverlaySummaryText", "流程阶段: {0}\n当前金币: {1} | 遗物数: {2} | 牌库数: {3}\n已有解析器: {4}"),
			FormatFlowStageText(Progression.FlowStage),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount),
			FormatBool(Progression.bCurrentNodeHasImplementedResolver)));
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (MissingFieldsText)
	{
		MissingFieldsText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"ShopNodeOverlayMissingFieldsText",
			"当前商店节点页已经从通用节点页拆出，但仍缺商店节点专用查询：商品列表、价格、刷新/售罄状态、以及购买/离开商店命令。"));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"ShopNodeOverlayGapText",
			"这是一张结构化占位页：它专门承接 PendingShopNode，让流程层能明确展示“进入商店节点但尚无专用查询”的状态。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayFeedbackDefault", "等待 Shop Node 专用查询与命令接入。")));
	}

	if (OpenNodeSelectButton)
	{
		OpenNodeSelectButton->SetIsEnabled(Progression.AvailableNextNodes.Num() > 0 || Progression.bCanAdvanceToNextNode);
	}
}
