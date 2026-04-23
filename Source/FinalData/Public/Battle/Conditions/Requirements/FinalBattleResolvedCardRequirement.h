#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleResolvedCardRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleResolvedCardRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	bool bRequireCardCostAP = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition", meta = (EditCondition = "bRequireCardCostAP"))
	int32 RequiredCardCostAP = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	bool bRequireCardType = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition", meta = (EditCondition = "bRequireCardType"))
	EFinalCardType RequiredCardType = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FGameplayTag RequiredKeyword;
};
