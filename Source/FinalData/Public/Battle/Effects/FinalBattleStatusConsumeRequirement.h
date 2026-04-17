#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleStatusConsumeRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleStatusConsumeRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireConsumedStatus = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireConsumedStatus"))
	FFinalStatusId RequiredStatusId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1", EditCondition = "bRequireConsumedStatus"))
	int32 MinimumStacks = 1;
};
