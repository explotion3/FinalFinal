#include "UI/Screens/Flow/FinalRunNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
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
		const FString DisplayName = FormatOptionalText(
			NodeOption.DisplayName,
			FormatOptionalName(NodeOption.NodeId, NSLOCTEXT("FinalFlowUI", "NodeOptionFallbackName", "未命名节点"))).ToString();
		const FString DisplayLabel = FormatOptionalName(
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
}

void UFinalRunNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	ClampSelectedNodeIndex();
	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleSelectPreviousNodeClicked()
{
	const int32 AvailableNodeCount = GetCachedSnapshot().Progression.AvailableNextNodes.Num();
	if (AvailableNodeCount <= 0)
	{
		return;
	}

	SelectedNodeIndex = SelectedNodeIndex == INDEX_NONE
		? 0
		: (SelectedNodeIndex - 1 + AvailableNodeCount) % AvailableNodeCount;

	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleSelectNextNodeClicked()
{
	const int32 AvailableNodeCount = GetCachedSnapshot().Progression.AvailableNextNodes.Num();
	if (AvailableNodeCount <= 0)
	{
		return;
	}

	SelectedNodeIndex = SelectedNodeIndex == INDEX_NONE
		? 0
		: (SelectedNodeIndex + 1) % AvailableNodeCount;

	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleAdvanceSelectedNodeClicked()
{
	const TArray<FFinalRunNodeOptionViewData>& AvailableNextNodes = GetCachedSnapshot().Progression.AvailableNextNodes;
	if (!AvailableNextNodes.IsValidIndex(SelectedNodeIndex))
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "NodeNoSelection", "当前没有可推进的目标节点。"));
		RebuildVisual();
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "NodeNoRunFlowSubsystem", "当前无法访问 RunFlowSubsystem，无法推进节点。"));
		RebuildVisual();
		return;
	}

	const FFinalRunNodeOptionViewData& SelectedNode = AvailableNextNodes[SelectedNodeIndex];
	const bool bAdvanced = RunFlowSubsystem->AdvanceToNode(SelectedNode.NodeId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAdvanced
			? NSLOCTEXT("FinalFlowUI", "NodeAdvanceSucceeded", "已转发 AdvanceToNode。")
			: NSLOCTEXT("FinalFlowUI", "NodeAdvanceFailed", "AdvanceToNode 执行失败。")));
	ClampSelectedNodeIndex();
	RebuildVisual();
}

void UFinalRunNodeOverlayScreen::HandleOpenRewardPageClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowBattleRewardOverlayPlaceholder();
	}
}

void UFinalRunNodeOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunNodeOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "NodeModalTitle", "节点选择页占位"),
			NSLOCTEXT("FinalFlowUI", "NodeModalBody", "当前节点选择页已经专门承接 AwaitingNodeAdvance。后续若需要节点地图确认、离开页面确认等阻断交互，应继续落在 Modal 层。"));
	}
}

void UFinalRunNodeOverlayScreen::ClampSelectedNodeIndex()
{
	const int32 AvailableNodeCount = GetCachedSnapshot().Progression.AvailableNextNodes.Num();
	if (AvailableNodeCount <= 0)
	{
		SelectedNodeIndex = INDEX_NONE;
		return;
	}

	if (!GetCachedSnapshot().Progression.AvailableNextNodes.IsValidIndex(SelectedNodeIndex))
	{
		SelectedNodeIndex = 0;
	}
}

void UFinalRunNodeOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.08f, 0.09f, 0.06f, 0.96f), TEXT("NodeOverlayRoot"), TEXT("NodeOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (CurrentNodeText == nullptr)
	{
		CurrentNodeText = CreateStageLabel(TEXT("NodeOverlayCurrentNode"), 13);
		ContentBox->InsertChildAt(2, CurrentNodeText);
	}

	if (AvailableNodesText == nullptr)
	{
		AvailableNodesText = CreateStageLabel(TEXT("NodeOverlayAvailableNodes"), 13);
		ContentBox->InsertChildAt(3, AvailableNodesText);
	}

	if (PreviousNodeButton == nullptr)
	{
		PreviousNodeButton = CreateStageButton(
			TEXT("NodeOverlayPreviousButton"),
			TEXT("NodeOverlayPreviousButtonText"),
			NSLOCTEXT("FinalFlowUI", "NodePreviousButton", "上一个候选节点"),
			PreviousNodeButtonText);
		PreviousNodeButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleSelectPreviousNodeClicked);
		ContentBox->AddChildToVerticalBox(PreviousNodeButton);
	}

	if (NextNodeButton == nullptr)
	{
		NextNodeButton = CreateStageButton(
			TEXT("NodeOverlayNextButton"),
			TEXT("NodeOverlayNextButtonText"),
			NSLOCTEXT("FinalFlowUI", "NodeNextButton", "下一个候选节点"),
			NextNodeButtonText);
		NextNodeButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleSelectNextNodeClicked);
		ContentBox->AddChildToVerticalBox(NextNodeButton);
	}

	if (AdvanceNodeButton == nullptr)
	{
		AdvanceNodeButton = CreateStageButton(
			TEXT("NodeOverlayAdvanceButton"),
			TEXT("NodeOverlayAdvanceButtonText"),
			NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonDisabled", "当前没有可推进节点"),
			AdvanceNodeButtonText);
		AdvanceNodeButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleAdvanceSelectedNodeClicked);
		ContentBox->AddChildToVerticalBox(AdvanceNodeButton);
	}

	if (OpenRewardPageButton == nullptr)
	{
		OpenRewardPageButton = CreateStageButton(
			TEXT("NodeOverlayRewardButton"),
			TEXT("NodeOverlayRewardButtonText"),
			NSLOCTEXT("FinalFlowUI", "NodeRewardButton", "查看战后奖励页"),
			OpenRewardPageButtonText);
		OpenRewardPageButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleOpenRewardPageClicked);
		ContentBox->AddChildToVerticalBox(OpenRewardPageButton);
	}

	if (OpenModalButton == nullptr)
	{
		OpenModalButton = CreateStageButton(
			TEXT("NodeOverlayModalButton"),
			TEXT("NodeOverlayModalButtonText"),
			NSLOCTEXT("FinalFlowUI", "NodeModalButton", "打开节点选择说明模态"),
			OpenModalButtonText);
		OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleOpenModalClicked);
		ContentBox->AddChildToVerticalBox(OpenModalButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("NodeOverlayCloseButton"),
			TEXT("NodeOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "NodeCloseButton", "关闭节点选择页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunNodeOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunNodeOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "NodeOverlayTitleText", "节点选择页"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "NodeOverlaySummaryText", "流程阶段: {0}\n可领奖励: {1}\n可推进节点: {2}\n待战斗桥接: {3}\n当前金币: {4} | 遗物数: {5} | 牌库数: {6}"),
			FormatFlowStageText(Progression.FlowStage),
			FormatBool(Progression.bCanClaimPendingBattleReward),
			FormatBool(Progression.bCanAdvanceToNextNode),
			FormatBool(Snapshot.PendingBattle.bHasPendingBattleStart),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (AvailableNodesText)
	{
		AvailableNodesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "NodeAvailableNodesSummary", "可选下一节点:\n{0}"),
			FText::FromString(BuildAvailableNodeSummary(Progression.AvailableNextNodes, SelectedNodeIndex))));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"NodeOverlayGapText",
			"当前页只承接 AwaitingNodeAdvance。奖励节点、事件节点、商店节点已拆到独立 Overlay 页，不再继续挤在这里。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "NodeOverlayFeedbackDefault", "当前页面会把推进节点意图转发给 RunFlowSubsystem，由它统一决定刷新与切页。")));
	}

	const bool bHasAvailableNodes = Progression.AvailableNextNodes.Num() > 0;
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
		const bool bHasSelectedNode = Progression.AvailableNextNodes.IsValidIndex(SelectedNodeIndex);
		const bool bSelectedNodeUnlocked = bHasSelectedNode ? !Progression.AvailableNextNodes[SelectedNodeIndex].bLocked : false;
		AdvanceNodeButton->SetIsEnabled(Progression.bCanAdvanceToNextNode && bHasAvailableNodes && bHasSelectedNode && bSelectedNodeUnlocked);
	}

	if (AdvanceNodeButtonText)
	{
		if (Progression.AvailableNextNodes.IsValidIndex(SelectedNodeIndex))
		{
			const FFinalRunNodeOptionViewData& SelectedNode = Progression.AvailableNextNodes[SelectedNodeIndex];
			if (SelectedNode.bLocked)
			{
				AdvanceNodeButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonLocked", "当前选中节点不可推进: {0}"),
					FormatOptionalText(
						SelectedNode.AvailabilityMessage,
						NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonLockedFallback", "该节点当前不可进入。"))));
			}
			else
			{
				AdvanceNodeButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "NodeAdvanceButton", "推进到选中节点: {0}"),
					FormatOptionalText(
						SelectedNode.DisplayName,
						FormatOptionalName(SelectedNode.NodeId, NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonFallback", "未命名节点")))));
			}
		}
		else
		{
			AdvanceNodeButtonText->SetText(NSLOCTEXT("FinalFlowUI", "NodeAdvanceButtonDisabled", "当前没有可推进节点"));
		}
	}

	if (OpenRewardPageButton)
	{
		OpenRewardPageButton->SetIsEnabled(Progression.bCanClaimPendingBattleReward || Snapshot.PendingBattleReward.bHasPendingReward);
	}
}
