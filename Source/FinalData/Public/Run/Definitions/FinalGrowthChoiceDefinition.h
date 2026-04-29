#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "FinalGrowthChoiceDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalGrowthChoiceType : uint8
{
    AttributeGrowth,
    CardEvolution,
    Special
};

UENUM(BlueprintType)
enum class EFinalGrowthAttributeType : uint8
{
    RootBone,
    Insight,
    KillingIntent
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalGrowthChoiceDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Growth")
    FFinalGrowthChoiceId GrowthChoiceId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    EFinalGrowthChoiceType ChoiceType = EFinalGrowthChoiceType::AttributeGrowth;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth", meta = (EditCondition = "ChoiceType == EFinalGrowthChoiceType::AttributeGrowth", EditConditionHides))
    EFinalGrowthAttributeType AttributeType = EFinalGrowthAttributeType::RootBone;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth", meta = (EditCondition = "ChoiceType == EFinalGrowthChoiceType::AttributeGrowth", EditConditionHides))
    int32 AttributeDelta = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth", meta = (EditCondition = "ChoiceType == EFinalGrowthChoiceType::CardEvolution", EditConditionHides))
    FFinalCardEvolutionId CardEvolutionId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth", meta = (MultiLine = "true"))
    FText Description;
};