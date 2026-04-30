#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "FinalPassiveDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalPassiveAppliesTo : uint8
{
	Shared,
	PlayerOnly,
	EnemyOnly
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalPassiveDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Passive")
	FFinalPassiveId PassiveId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	FName DisplayId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	FText SummaryText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	EFinalPassiveStackPolicy StackPolicy = EFinalPassiveStackPolicy::RefreshExisting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	EFinalPassiveDurationType DurationType = EFinalPassiveDurationType::Battle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive", meta = (ClampMin = "1"))
	int32 MaxStacks = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	EFinalPassiveAppliesTo AppliesTo = EFinalPassiveAppliesTo::Shared;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Passive")
	TArray<FFinalRuntimeTriggerDefinition> RuntimeTriggers;
};
