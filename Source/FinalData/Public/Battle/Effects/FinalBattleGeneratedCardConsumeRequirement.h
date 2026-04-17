#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalBattleGeneratedCardConsumeRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleGeneratedCardConsumeRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireConsumedGeneratedCard = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireConsumedGeneratedCard"))
	FFinalCardId RequiredCardId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireConsumedGeneratedCard"))
	FGameplayTag RequiredKeyword;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1", EditCondition = "bRequireConsumedGeneratedCard"))
	int32 MinimumCount = 1;
};
