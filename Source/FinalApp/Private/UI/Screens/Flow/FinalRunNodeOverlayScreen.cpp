#include "UI/Screens/Flow/FinalRunNodeOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"

namespace
{
FText FormatNodeOverlayOptionalName(const FName Name, const FText& Fallback)
{
	return Name != NAME_None ? FText::FromName(Name) : Fallback;
}

FText FormatNodeOverlayOptionalText(const FText& Value, const FText& Fallback)
{
	return !Value.IsEmpty() ? Value : Fallback;
}

FText FormatNodeOverlayBool(const bool bValue)
{
	return bValue
		? NSLOCTEXT("FinalFlowUI", "NodeBoolYes", "是")
		: NSLOCTEXT("FinalFlowUI", "NodeBoolNo", "否");
}

FText FormatFlowStageText(const EFinalRunFlowStage FlowStage)
{
	switch (FlowStage)
	{
	case EFinalRunFlowStage::PreparingBattle:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePreparingBattle", "战前准备");

	case EFinalRunFlowStage::PendingBattleReward:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingReward", "待领奖励");

	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageAwaitingNodeAdvance", "等待推进节点");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingRewardNode", "待处理奖励节点");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingEventNode", "待处理事件节点");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingShopNode", "待处理商店节点");

	case EFinalRunFlowStage::RunEnded:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageRunEnded", "本局结束");

	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageNone", "未初始化");
	}
}

FText FormatNodeTypeText(const EFinalRunNodeType NodeType)
{
	switch (NodeType)
	{
	case EFinalRunNodeType::Battle:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeBattle", "Battle");

	case EFinalRunNodeType::Event:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeEvent", "Event");

	case EFinalRunNodeType::Shop:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeShop", "Shop");

	case EFinalRunNodeType::Reward:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeReward", "Reward");

	case EFinalRunNodeType::EliteBattle:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeEliteBattle", "Elite Battle");

	case EFinalRunNodeType::BossBattle:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeBossBattle", "Boss Battle");

	case EFinalRunNodeType::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunNodeTypeNone", "None");
	}
}

FString BuildAvailableNodeSummary(const TArray<FFinalRunNodeOptionViewData>& AvailableNextNodes, const int32 SelectedNodeIndex)
{
	if (AvailableNextNodes.Num() <= 0)
	{
		return NSLOCTEXT("FinalFlowUI", "NodeNoAvailableNodes", "当前没有公开的下一节点候选。").ToString();
	}

	FString NodeSummary;
	for (int32 Index = 0; Index < AvailableNextNodes.Num(); ++Index)
	{
		const FFinalRunNodeOptionViewData& NodeOption = AvailableNextNodes[Index];
		const FString Prefix = Index == SelectedNodeIndex ? TEXT("> ") : TEXT("  ");
		const FString DisplayName = FormatNodeOverlayOptionalText(
			NodeOption.DisplayName,
			FormatNodeOverlayOptionalName(NodeOption.NodeId, NSLOCTEXT("FinalFlowUI", "NodeOptionFallbackName", "未命名节点"))).ToString();
		const FString DisplayLabel = FormatNodeOverlayOptionalName(
			NodeOption.DisplayLabel,
			NSLOCTEXT("FinalFlowUI", "NodeOptionFallbackLabel", "无")).ToString();
		const FString AvailabilityMessage = !NodeOption.AvailabilityMessage.IsEmpty()
			? NodeOption.AvailabilityMessage.ToString()
			: NSLOCTEXT("FinalFlowUI", "NodeOptionAvailabilityDefault", "可前往").ToString();

		NodeSummary += FString::Printf(
			TEXT("%s%s [%s]\n    Label=%s | NodeId=%s | Chapter/Floor=%d-%d\n    已访问=%s | 已锁定=%s | 已有解析器=%s\n    可用性=%s"),
			*Prefix,
			*DisplayName,
			*FormatNodeTypeText(NodeOption.NodeType).ToString(),
			*DisplayLabel,
			*NodeOption.NodeId.ToString(),
			NodeOption.ChapterIndex,
			NodeOption.FloorIndex,
			NodeOption.bVisited ? TEXT("是") : TEXT("否"),
			NodeOption.bLocked ? TEXT("是") : TEXT("否"),
			NodeOption.bHasImplementedResolver ? TEXT("是") : TEXT("否"),
			*AvailabilityMessage);

		if (NodeOption.EncounterId.IsValid())
		{
			NodeSummary += FString::Printf(TEXT("\n    Encounter=%s"), *NodeOption.EncounterId.Value.ToString());
		}

		NodeSummary += TEXT("\n");
	}

	NodeSummary.TrimEndInline();
	return NodeSummary;
}

UTextBlock* CreateNodeOverlayLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

UFinalRunFlowSubsystem* ResolveNodeOverlayRunFlowSubsystem(UUserWidget* Widget)
{
	if (Widget == nullptr)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = Widget->GetGameInstance();
	if (GameInstance == nullptr)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UFinalRunFlowSubsystem>();
}
}

void UFinalRunNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	CachedSnapshot = InSnapshot;
	LastActionFeedback = FText::GetEmpty();
	ClampSelectedNodeIndex();
	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleSelectPreviousNodeClicked()
{
	const int32 AvailableNodeCount = CachedSnapshot.Progression.AvailableNextNodes.Num();
	if (AvailableNodeCount <= 0)
	{
		return;
	}

	if (SelectedNodeIndex == INDEX_NONE)
	{
		SelectedNodeIndex = 0;
	}
	else
	{
		SelectedNodeIndex = (SelectedNodeIndex - 1 + AvailableNodeCount) % AvailableNodeCount;
	}

	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleSelectNextNodeClicked()
{
	const int32 AvailableNodeCount = CachedSnapshot.Progression.AvailableNextNodes.Num();
	if (AvailableNodeCount <= 0)
	{
		return;
	}

	if (SelectedNodeIndex == INDEX_NONE)
	{
		SelectedNodeIndex = 0;
	}
	else
	{
		SelectedNodeIndex = (SelectedNodeIndex + 1) % AvailableNodeCount;
	}

	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleAdvanceSelectedNodeClicked()
{
	const TArray<FFinalRunNodeOptionViewData>& AvailableNextNodes = CachedSnapshot.Progression.AvailableNextNodes;
	if (!AvailableNextNodes.IsValidIndex(SelectedNodeIndex))
	{
		LastActionFeedback = NSLOCTEXT("FinalFlowUI", "NodeNoSelection", "当前没有可推进的目标节点。");
		RebuildVisual();
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveNodeOverlayRunFlowSubsystem(this);
	if (RunFlowSubsystem == nullptr)
	{
		LastActionFeedback = NSLOCTEXT("FinalFlowUI", "NodeNoRunFlowSubsystem", "当前无法访问 RunFlowSubsystem，无法推进节点。");
		RebuildVisual();
		return;
	}

	const FFinalRunNodeOptionViewData& SelectedNode = AvailableNextNodes[SelectedNodeIndex];
	const bool bAdvanced = RunFlowSubsystem->AdvanceToNode(SelectedNode.NodeId);
	CachedSnapshot = RunFlowSubsystem->GetCurrentRunSnapshot();
	ClampSelectedNodeIndex();
	LastActionFeedback = !RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAdvanced
			? NSLOCTEXT("FinalFlowUI", "NodeAdvanceSucceeded", "已转发 AdvanceToNode。")
			: NSLOCTEXT("FinalFlowUI", "NodeAdvanceFailed", "AdvanceToNode 执行失败。"));
	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleOpenRewardPageClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->ShowBattleRewardOverlayPlaceholder();
		}
	}
}

void UFinalRunNodeOverlayScreen::HandleCloseClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->CloseOverlayScreen(this);
		}
	}
}

void UFinalRunNodeOverlayScreen::HandleOpenModalClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->ShowPlaceholderModal(
				NSLOCTEXT("FinalFlowUI", "NodeModalTitle", "节点确认占位"),
				NSLOCTEXT("FinalFlowUI", "NodeModalBody", "后续节点选择确认、离开当前页、返回地图等阻断交互，应该落在 Modal 层，不影响常驻 Battle HUD。"));
		}
	}
}

void UFinalRunNodeOverlayScreen::ClampSelectedNodeIndex()
{
	const int32 AvailableNodeCount = CachedSnapshot.Progression.AvailableNextNodes.Num();
	if (AvailableNodeCount <= 0)
	{
		SelectedNodeIndex = INDEX_NONE;
		return;
	}

	if (!CachedSnapshot.Progression.AvailableNextNodes.IsValidIndex(SelectedNodeIndex))
	{
		SelectedNodeIndex = 0;
	}
}

void UFinalRunNodeOverlayScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NodeOverlayRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.08f, 0.09f, 0.06f, 0.96f));
	RootBorder->SetPadding(FMargin(24.0f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NodeOverlayContent"));
	RootBorder->SetContent(ContentBox);

	TitleText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayTitle"), 22);
	ContentBox->AddChildToVerticalBox(TitleText);

	SummaryText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlaySummary"), 14);
	ContentBox->AddChildToVerticalBox(SummaryText);

	CurrentNodeText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayCurrentNode"), 13);
	ContentBox->AddChildToVerticalBox(CurrentNodeText);

	AvailableNodesText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayAvailableNodes"), 13);
	ContentBox->AddChildToVerticalBox(AvailableNodesText);

	GapText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayGap"), 12);
	ContentBox->AddChildToVerticalBox(GapText);

	FeedbackText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayFeedback"), 12);
	ContentBox->AddChildToVerticalBox(FeedbackText);

	PreviousNodeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NodeOverlayPreviousButton"));
	PreviousNodeButtonText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayPreviousButtonText"), 13);
	PreviousNodeButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodePreviousButton", "上一个候选节点"));
	PreviousNodeButton->AddChild(PreviousNodeButtonText);
	PreviousNodeButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleSelectPreviousNodeClicked);
	ContentBox->AddChildToVerticalBox(PreviousNodeButton);

	NextNodeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NodeOverlayNextButton"));
	NextNodeButtonText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayNextButtonText"), 13);
	NextNodeButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodeNextButton", "下一个候选节点"));
	NextNodeButton->AddChild(NextNodeButtonText);
	NextNodeButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleSelectNextNodeClicked);
	ContentBox->AddChildToVerticalBox(NextNodeButton);

	AdvanceNodeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NodeOverlayAdvanceButton"));
	AdvanceNodeButtonText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayAdvanceButtonText"), 13);
	AdvanceNodeButton->AddChild(AdvanceNodeButtonText);
	AdvanceNodeButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleAdvanceSelectedNodeClicked);
	ContentBox->AddChildToVerticalBox(AdvanceNodeButton);

	OpenRewardPageButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NodeOverlayRewardButton"));
	OpenRewardPageButtonText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayRewardButtonText"), 13);
	OpenRewardPageButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodeRewardButton", "查看待领奖励页"));
	OpenRewardPageButton->AddChild(OpenRewardPageButtonText);
	OpenRewardPageButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleOpenRewardPageClicked);
	ContentBox->AddChildToVerticalBox(OpenRewardPageButton);

	OpenModalButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NodeOverlayModalButton"));
	OpenModalButtonText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayModalButtonText"), 13);
	OpenModalButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodeModalButton", "打开节点说明模态"));
	OpenModalButton->AddChild(OpenModalButtonText);
	OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleOpenModalClicked);
	ContentBox->AddChildToVerticalBox(OpenModalButton);

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NodeOverlayCloseButton"));
	CloseButtonText = CreateNodeOverlayLabel(WidgetTree, TEXT("NodeOverlayCloseButtonText"), 13);
	CloseButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodeCloseButton", "关闭节点页"));
	CloseButton->AddChild(CloseButtonText);
	CloseButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleCloseClicked);
	ContentBox->AddChildToVerticalBox(CloseButton);
}

void UFinalRunNodeOverlayScreen::RebuildVisual()
{
	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "NodeOverlayTitleText", "节点推进页"));
	}

	if (SummaryText)
	{
		const FFinalRunProgressionViewData& Progression = CachedSnapshot.Progression;
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "NodeOverlaySummaryText", "流程阶段: {0}\n可领奖励: {1}\n可推进节点: {2}\n待战斗桥接: {3}\n当前金币: {4} | 遗物数: {5} | 牌库数: {6}"),
			FormatFlowStageText(Progression.FlowStage),
			Progression.bCanClaimPendingBattleReward
				? NSLOCTEXT("FinalFlowUI", "NodeCanClaimReward", "是")
				: NSLOCTEXT("FinalFlowUI", "NodeCannotClaimReward", "否"),
			Progression.bCanAdvanceToNextNode
				? NSLOCTEXT("FinalFlowUI", "NodeCanAdvance", "是")
				: NSLOCTEXT("FinalFlowUI", "NodeCannotAdvance", "否"),
			CachedSnapshot.PendingBattle.bHasPendingBattleStart
				? NSLOCTEXT("FinalFlowUI", "NodeHasPendingBattle", "已配置")
				: NSLOCTEXT("FinalFlowUI", "NodeNoPendingBattle", "无"),
			FText::AsNumber(CachedSnapshot.Gold),
			FText::AsNumber(CachedSnapshot.RelicCount),
			FText::AsNumber(CachedSnapshot.DeckCount)));
	}

	if (CurrentNodeText)
	{
		const FFinalRunProgressionViewData& Progression = CachedSnapshot.Progression;
		CurrentNodeText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "NodeOverlayCurrentNodeText", "当前节点: {0}\n显示标签: {1}\nNodeId: {2}\n当前节点类型: {3}\n章节/楼层: {4}/{5}\n已访问: {6}\n需要解析: {7}\n已有解析器: {8}\n当前节点状态: {9}"),
			FormatNodeOverlayOptionalText(
				Progression.CurrentNodeDisplayName,
				FormatNodeOverlayOptionalName(Progression.CurrentNodeId, NSLOCTEXT("FinalFlowUI", "NodeCurrentNodeNone", "无"))),
			FormatNodeOverlayOptionalName(Progression.CurrentNodeDisplayLabel, NSLOCTEXT("FinalFlowUI", "NodeCurrentNodeLabelNone", "无")),
			FormatNodeOverlayOptionalName(Progression.CurrentNodeId, NSLOCTEXT("FinalFlowUI", "NodeCurrentNodeIdNone", "无")),
			FormatNodeTypeText(Progression.CurrentNodeType),
			FText::AsNumber(Progression.CurrentChapter),
			FText::AsNumber(Progression.CurrentFloor),
			FormatNodeOverlayBool(Progression.bCurrentNodeVisited),
			FormatNodeOverlayBool(Progression.bCurrentNodeNeedsResolution),
			FormatNodeOverlayBool(Progression.bCurrentNodeHasImplementedResolver),
			FormatNodeOverlayOptionalText(
				Progression.CurrentNodeStateMessage,
				NSLOCTEXT("FinalFlowUI", "NodeCurrentNodeStateDefault", "当前没有额外状态说明。"))));
	}

	if (AvailableNodesText)
	{
		AvailableNodesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "NodeAvailableNodesSummary", "可选下一节点:\n{0}"),
			FText::FromString(BuildAvailableNodeSummary(CachedSnapshot.Progression.AvailableNextNodes, SelectedNodeIndex))));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"NodeOverlayGapText",
			"当前页已真实消费章节/楼层、节点显示名/标签、访问状态、锁定状态、可达说明与解析器实现状态。剩余缺口主要是地图布局、节点图标、以及事件/商店/奖励节点的专用详情页。"));
	}

	if (FeedbackText)
	{
		if (!LastActionFeedback.IsEmpty())
		{
			FeedbackText->SetText(LastActionFeedback);
		}
		else if (!CachedSnapshot.Progression.CurrentNodeStateMessage.IsEmpty())
		{
			FeedbackText->SetText(CachedSnapshot.Progression.CurrentNodeStateMessage);
		}
		else
		{
			FeedbackText->SetText(NSLOCTEXT("FinalFlowUI", "NodeOverlayFeedbackDefault", "当前页面会把推进节点意图转发给 RunFlowSubsystem，由它统一决定刷新与切页。"));
		}
	}

	const bool bHasAvailableNodes = CachedSnapshot.Progression.AvailableNextNodes.Num() > 0;
	if (PreviousNodeButton)
	{
		PreviousNodeButton->SetIsEnabled(bHasAvailableNodes);
	}

	if (NextNodeButton)
	{
		NextNodeButton->SetIsEnabled(bHasAvailableNodes);
	}

	if (AdvanceNodeButton)
	{
		const bool bHasSelectedNode = CachedSnapshot.Progression.AvailableNextNodes.IsValidIndex(SelectedNodeIndex);
		const bool bSelectedNodeUnlocked = bHasSelectedNode ? !CachedSnapshot.Progression.AvailableNextNodes[SelectedNodeIndex].bLocked : false;
		AdvanceNodeButton->SetIsEnabled(CachedSnapshot.Progression.bCanAdvanceToNextNode && bHasAvailableNodes && bHasSelectedNode && bSelectedNodeUnlocked);
	}

	if (AdvanceNodeButtonText)
	{
		if (CachedSnapshot.Progression.AvailableNextNodes.IsValidIndex(SelectedNodeIndex))
		{
			const FFinalRunNodeOptionViewData& SelectedNode = CachedSnapshot.Progression.AvailableNextNodes[SelectedNodeIndex];
			if (SelectedNode.bLocked)
			{
				AdvanceNodeButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonLocked", "当前选中节点不可推进: {0}"),
					FormatNodeOverlayOptionalText(
						SelectedNode.AvailabilityMessage,
						NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonLockedFallback", "该节点当前不可进入。"))));
			}
			else
			{
				AdvanceNodeButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "NodeAdvanceButton", "推进到选中节点: {0}"),
					FormatNodeOverlayOptionalText(
						SelectedNode.DisplayName,
						FormatNodeOverlayOptionalName(SelectedNode.NodeId, NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonFallback", "未命名节点")))));
			}
		}
		else
		{
			AdvanceNodeButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonDisabled", "当前没有可推进节点"));
		}
	}

	if (OpenRewardPageButton)
	{
		OpenRewardPageButton->SetIsEnabled(CachedSnapshot.Progression.bCanClaimPendingBattleReward || CachedSnapshot.PendingBattleReward.bHasPendingReward);
	}
}
