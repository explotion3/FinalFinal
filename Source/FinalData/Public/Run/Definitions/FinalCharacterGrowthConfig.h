#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Run/Bridge/FinalBattleGrowthFact.h"
#include "FinalCharacterGrowthConfig.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalCharacterGrowthConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Growth")
    FFinalCharacterGrowthConfigId GrowthConfigId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth", meta = (ClampMin = "1"))
    int32 BaseBreakthroughRequiredValue = 100;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|Breakthrough")
    TArray<EFinalBattleGrowthFactType> PreferredBreakthroughFactTypes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|Breakthrough")
    TMap<EFinalBattleGrowthFactType, float> BreakthroughGainScalarByFactType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|Breakthrough", meta = (ClampMin = "0"))
    int32 NormalBattleVictoryBreakthroughReward = 8;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|Breakthrough", meta = (ClampMin = "0"))
    int32 EliteBattleVictoryBreakthroughReward = 18;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|Breakthrough", meta = (ClampMin = "0"))
    int32 BossBattleVictoryBreakthroughReward = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|RootBone")
    int32 RootBoneVitalSharePerPoint = 6;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|RootBone")
    int32 RootBoneDefensePerPoint = 2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|RootBone")
    int32 RootBoneStressCapPerPoint = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|Insight")
    float InsightBreakthroughGainMultiplierPerPoint = 0.10f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|KillingIntent")
    int32 KillingIntentAttackPerPoint = 2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|KillingIntent")
    float KillingIntentCritChancePerPoint = 0.01f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth|KillingIntent")
    float KillingIntentCritDamagePerPoint = 0.03f;
};
