#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"

class FFinalBattleCardService;
class FFinalBattleConditionService;
class FFinalBattleEffectExecutionService;
class FFinalBattleUnitService;
struct FFinalBattleEffectExecutionSummary;
struct FFinalBattleState;

struct FFinalBattleResolvedCardTriggerContext
{
	FName RuntimeOwnerUnitId = NAME_None;
	FFinalCardId CardId;
	EFinalCardType CardType = EFinalCardType::Attack;
	int32 RuntimeCostAP = 0;
	FGameplayTagContainer RuntimeKeywords;
	bool bGeneratedCard = false;
};

class FFinalBattleTriggerService
{
public:
	void HandleOwnerTookHealthDamage(
		FFinalBattleState& BattleState,
		const FFinalBattleUnitService& UnitService,
		const FFinalBattleEffectExecutionService& EffectExecutionService,
		FFinalBattleEffectExecutionSummary& InOutSummary) const;

	void HandlePlayerTeamTookHealthDamage(
		FFinalBattleState& BattleState,
		int32 ActualHealthDamage,
		const FFinalBattleCardService& CardService,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;

	void HandlePlayerCardResolved(
		FFinalBattleState& BattleState,
		const FFinalBattleResolvedCardTriggerContext& CardContext,
		const FFinalBattleCardService& CardService,
		const FFinalBattleConditionService& ConditionService,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
};
