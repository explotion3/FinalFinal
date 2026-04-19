#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"

class FFinalBattleCardService;
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

class FFinalBattleRelicService
{
public:
	void InitializeRelics(
		FFinalBattleState& BattleState,
		TArray<FFinalBattleStartRelicInput> ActiveRelics,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;

	void ApplyPlayerTurnStartRelicEffects(FFinalBattleState& BattleState, TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
	void ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const;
	void HandlePlayerTeamTookHealthDamage(
		FFinalBattleState& BattleState,
		int32 ActualHealthDamage,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
	void HandlePlayerCardResolved(
		FFinalBattleState& BattleState,
		const FFinalBattleResolvedCardTriggerContext& CardContext,
		const FFinalBattleCardService& CardService,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
};
