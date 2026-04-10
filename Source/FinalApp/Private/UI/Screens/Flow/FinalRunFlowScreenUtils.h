#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunSnapshot.h"

namespace FinalRunFlowScreenUtils
{
inline FText FormatOptionalName(const FName Name, const FText& Fallback)
{
	return Name != NAME_None ? FText::FromName(Name) : Fallback;
}

inline FText FormatOptionalText(const FText& Value, const FText& Fallback)
{
	return !Value.IsEmpty() ? Value : Fallback;
}

inline FText FormatBool(const bool bValue)
{
	return bValue
		? NSLOCTEXT("FinalFlowUI", "RunFlowBoolYes", "是")
		: NSLOCTEXT("FinalFlowUI", "RunFlowBoolNo", "否");
}

inline FText FormatBattleOutcomeText(const EFinalBattleOutcome Outcome)
{
	switch (Outcome)
	{
	case EFinalBattleOutcome::Victory:
		return NSLOCTEXT("FinalFlowUI", "RunFlowBattleOutcomeVictory", "胜利");

	case EFinalBattleOutcome::Defeat:
		return NSLOCTEXT("FinalFlowUI", "RunFlowBattleOutcomeDefeat", "失败");

	case EFinalBattleOutcome::Escape:
		return NSLOCTEXT("FinalFlowUI", "RunFlowBattleOutcomeEscape", "撤离");

	case EFinalBattleOutcome::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowBattleOutcomeNone", "未结算");
	}
}

inline FText FormatFlowStageText(const EFinalRunFlowStage FlowStage)
{
	switch (FlowStage)
	{
	case EFinalRunFlowStage::PreparingBattle:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePreparingBattleShared", "战前准备");

	case EFinalRunFlowStage::PendingBattleReward:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingBattleRewardShared", "待领奖励");

	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageAwaitingNodeAdvanceShared", "等待推进节点");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingRewardNodeShared", "待处理奖励节点");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingEventNodeShared", "待处理事件节点");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePendingShopNodeShared", "待处理商店节点");

	case EFinalRunFlowStage::RunEnded:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageRunEndedShared", "本局结束");

	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageNoneShared", "未初始化");
	}
}

inline FText FormatNodeTypeText(const EFinalRunNodeType NodeType)
{
	switch (NodeType)
	{
	case EFinalRunNodeType::Battle:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeBattle", "Battle");

	case EFinalRunNodeType::Event:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeEvent", "Event");

	case EFinalRunNodeType::Shop:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeShop", "Shop");

	case EFinalRunNodeType::Reward:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeReward", "Reward");

	case EFinalRunNodeType::EliteBattle:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeEliteBattle", "Elite Battle");

	case EFinalRunNodeType::BossBattle:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeBossBattle", "Boss Battle");

	case EFinalRunNodeType::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNodeTypeNone", "None");
	}
}

inline FText FormatRewardTypeText(const EFinalRunRewardType RewardType)
{
	switch (RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeGold", "金币");

	case EFinalRunRewardType::CardGrant:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeCardGrant", "获得卡牌");

	case EFinalRunRewardType::RelicGrant:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeRelicGrant", "获得遗物");

	case EFinalRunRewardType::RemoveCard:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeRemoveCard", "移除卡牌");

	case EFinalRunRewardType::UpgradeCard:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeUpgradeCard", "强化卡牌");

	case EFinalRunRewardType::Growth:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeGrowth", "成长");

	case EFinalRunRewardType::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardTypeNone", "未定义");
	}
}

inline FText FormatRewardEntryName(const FFinalRunRewardEntry& RewardEntry)
{
	if (!RewardEntry.DisplayName.IsEmpty())
	{
		return RewardEntry.DisplayName;
	}

	if (RewardEntry.DisplayId != NAME_None)
	{
		return FText::FromName(RewardEntry.DisplayId);
	}

	if (RewardEntry.RewardId != NAME_None)
	{
		return FText::FromName(RewardEntry.RewardId);
	}

	return NSLOCTEXT("FinalFlowUI", "RunFlowRewardEntryUnnamed", "未命名奖励");
}

inline FText FormatRewardClaimStateText(const FFinalRunRewardEntry& RewardEntry)
{
	if (RewardEntry.bClaimed)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardEntryClaimed", "已领取");
	}

	if (RewardEntry.bCanClaim)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardEntryCanClaim", "可领取");
	}

	return NSLOCTEXT("FinalFlowUI", "RunFlowRewardEntryCannotClaim", "暂不可领取");
}

inline FString BuildRewardEntriesSummaryString(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	if (RewardEntries.Num() <= 0)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRewardEntriesEmptyShared", "当前没有公开的结构化奖励条目。").ToString();
	}

	FString RewardEntrySummary;
	for (int32 Index = 0; Index < RewardEntries.Num(); ++Index)
	{
		const FFinalRunRewardEntry& RewardEntry = RewardEntries[Index];
		RewardEntrySummary += FString::Printf(
			TEXT("[%d] %s | 类型: %s | 数值: %d | 状态: %s"),
			Index + 1,
			*FormatRewardEntryName(RewardEntry).ToString(),
			*FormatRewardTypeText(RewardEntry.RewardType).ToString(),
			RewardEntry.Value,
			*FormatRewardClaimStateText(RewardEntry).ToString());

		if (RewardEntry.RewardId != NAME_None)
		{
			RewardEntrySummary += FString::Printf(TEXT(" | RewardId: %s"), *RewardEntry.RewardId.ToString());
		}

		RewardEntrySummary += TEXT("\n");
	}

	RewardEntrySummary.TrimEndInline();
	return RewardEntrySummary;
}

inline FText BuildCurrentNodeSummaryText(const FFinalRunProgressionViewData& Progression)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowCurrentNodeSummary", "当前节点: {0}\n显示标签: {1}\nNodeId: {2}\n当前节点类型: {3}\n章节/楼层: {4}/{5}\n已访问: {6}\n需要解析: {7}\n已有解析器: {8}\n当前节点状态: {9}"),
		FormatOptionalText(
			Progression.CurrentNodeDisplayName,
			FormatOptionalName(Progression.CurrentNodeId, NSLOCTEXT("FinalFlowUI", "RunFlowCurrentNodeNone", "无"))),
		FormatOptionalName(Progression.CurrentNodeDisplayLabel, NSLOCTEXT("FinalFlowUI", "RunFlowCurrentNodeLabelNone", "无")),
		FormatOptionalName(Progression.CurrentNodeId, NSLOCTEXT("FinalFlowUI", "RunFlowCurrentNodeIdNone", "无")),
		FormatNodeTypeText(Progression.CurrentNodeType),
		FText::AsNumber(Progression.CurrentChapter),
		FText::AsNumber(Progression.CurrentFloor),
		FormatBool(Progression.bCurrentNodeVisited),
		FormatBool(Progression.bCurrentNodeNeedsResolution),
		FormatBool(Progression.bCurrentNodeHasImplementedResolver),
		FormatOptionalText(
			Progression.CurrentNodeStateMessage,
			NSLOCTEXT("FinalFlowUI", "RunFlowCurrentNodeStateDefault", "当前没有额外状态说明。")));
}
}
