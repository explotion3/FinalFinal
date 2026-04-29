#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
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
