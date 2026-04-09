#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "FinalBattleRuleConfig.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalBattleRuleConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 InitialAP = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 InitialHandSize = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 HandLimit = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 MaxEP = 70;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 EndTurnEpGain = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 OnHitEpGain = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 BaseCardEpGain = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 BreakRewardAP = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 NormalCardInitiativeEventCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 CollapsedCardInitiativeEventCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	bool bUltimateTriggersInitiativeEvent = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	TMap<int32, int32> AwakenThresholdByCollapseCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	TMap<int32, float> DirectAwakenChanceByRemainingCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 AwakenStressResetValue = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 CollapseCardAwakenGain = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 StressHpLossPerPoint = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 StressHealPerPoint = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 MinStressChangePerEvent = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 MaxStressGainPerHit = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 StressRandomProtectionCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Rules")
	int32 DamageToBreakCap = 6;
};
