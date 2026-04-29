#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/Requirements/FinalBattleHandCardRequirement.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Systems/FinalBattleCardMatchCriteria.h"

class UFinalCardDefinition;
class FFinalBattleUnitService;
struct FFinalBattleCardInstance;
struct FFinalBattleCardInitData;
struct FFinalBattleCardRefreshRequest;
struct FFinalBattleCardViewData;
struct FFinalBattleState;
struct FFinalTeamDeckState;

enum class EFinalBattleCardZone : uint8
{
	Hand,           // 手牌区
	DrawPileTop,    // 抽牌堆顶
	DrawPileBottom, // 抽牌堆底
	DiscardPile,    // 弃牌堆
	OngoingZone,    // 持续区
	ConsumePile     // 消耗区
};

class FFinalBattleCardService
{
public:
	// 初始化牌区容器本身，不创建任何卡牌实例。
	void InitializeDeckState(FFinalTeamDeckState& DeckState) const;

	// 基于构筑定义创建战斗内卡牌实例，并放入初始抽牌堆。
	void InitializeDeckCards(
		FFinalBattleState& BattleState,
		const TArray<FFinalBattleCardInitData>& DeckCards,
		const TMap<FName, FName>& TemplateToRuntimeUnitMap) const;

	// 开战前整理初始抽牌堆：先洗牌，再把带“开战”关键词的牌置顶。
	void PrepareInitialDrawPile(FFinalBattleState& BattleState) const;

	// 按实例 Id 查可写卡牌实例；查不到返回 nullptr。
	FFinalBattleCardInstance* FindCardInstance(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;

	// 按实例 Id 查只读卡牌实例；查不到返回 nullptr。
	const FFinalBattleCardInstance* FindCardInstance(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;

	// 判断某张卡当前是否仍位于手牌区。
	bool IsCardInHand(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;

	// 统计某个牌区内满足条件的卡牌实例数量。
	int32 CountMatchingCardsInZone(
		const FFinalBattleState& BattleState,
		EFinalBattleCardZone SourceZone,
		const FFinalBattleCardMatchCriteria& Criteria) const;

	// 手牌专用薄封装：统计当前手牌中满足条件的卡牌数量。
	int32 CountMatchingCardsInHand(
		const FFinalBattleState& BattleState,
		FName RuntimeOwnerUnitId,
		const FFinalCardId& RequiredCardId,
		const FGameplayTag& RequiredKeyword,
		bool bGeneratedOnly) const;

	// 判断某个牌区内满足条件的卡牌数量是否达到要求。
	bool SatisfiesMatchCriteriaInZone(
		const FFinalBattleState& BattleState,
		EFinalBattleCardZone SourceZone,
		const FFinalBattleCardMatchCriteria& Criteria,
		int32 MinimumCount) const;

	// 手牌专用判定：是否满足某个效果对手牌内容的前置要求。
	bool SatisfiesHandCardRequirement(
		const FFinalBattleState& BattleState,
		FName RuntimeOwnerUnitId,
		const FFinalBattleHandCardRequirement& Requirement) const;

	// 创建一张新的战斗内卡牌实例，并初始化运行时关键词与行为缓存。
	FGuid CreateCardInstance(
		FFinalBattleState& BattleState,
		UFinalCardDefinition* CardDefinition,
		FName RuntimeOwnerUnitId,
		FName SourceRunCardInstanceId = NAME_None,
		bool bGeneratedCard = false,
		bool bTemporaryCard = false) const;

	int32 RefreshCardsForRunCardInstance(
		FFinalBattleState& BattleState,
		const FFinalBattleCardRefreshRequest& RefreshRequest) const;

	// 把现有卡牌实例移动到指定牌区；进入目标牌区前会先脱离所有旧牌区。
	bool MoveCardInstanceToZone(
		FFinalBattleState& BattleState,
		const FGuid& CardInstanceId,
		EFinalBattleCardZone Zone) const;

	// 从源牌区按条件匹配若干张牌，并移动到目标牌区。
	int32 MoveMatchingCardsBetweenZones(
		FFinalBattleState& BattleState,
		EFinalBattleCardZone SourceZone,
		EFinalBattleCardZone DestinationZone,
		const FFinalBattleCardMatchCriteria& Criteria,
		int32 MoveCount,
		TArray<FGuid>* OutMovedCardInstanceIds = nullptr) const;

	// 回合结束时整理手牌：非保留牌进入弃牌堆，保留牌继续留在手牌。
	void ResolveEndTurnHandCleanup(FFinalBattleState& BattleState) const;

	// 打出手牌后的默认去向处理：消耗牌进消耗区，其余进弃牌堆。
	void MoveHandCardAfterPlay(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;

	// 抽指定数量的牌；若抽牌堆为空，会先尝试把弃牌堆洗回抽牌堆。
	int32 DrawCards(FFinalBattleState& BattleState, int32 DrawCount) const;

	// 基于当前手牌实例构建对外只读的手牌视图数据。
	void BuildHandCardViews(
		const FFinalBattleState& BattleState,
		const FFinalBattleUnitService& UnitService,
		TArray<FFinalBattleCardViewData>& OutViews) const;

private:
	// 抽牌堆为空时，把弃牌堆洗回抽牌堆。
	bool RefillDrawPileFromDiscard(FFinalBattleState& BattleState) const;

	void ApplyCardDefinitionToInstance(FFinalBattleCardInstance& CardInstance, UFinalCardDefinition* CardDefinition) const;

	// 从指定牌区收集满足条件的卡牌实例 Id，不做实际迁移。
	void CollectMatchingCardInstanceIdsInZone(
		const FFinalBattleState& BattleState,
		EFinalBattleCardZone SourceZone,
		const FFinalBattleCardMatchCriteria& Criteria,
		int32 MaxCount,
		TArray<FGuid>& OutCardInstanceIds) const;

	// 让某张卡牌实例脱离所有牌区，保证同一实例不会同时挂在多个牌区。
	void RemoveCardInstanceFromAllZones(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
};
