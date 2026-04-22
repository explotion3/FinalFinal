#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleStatusConsumeRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleStatusConsumeRequirement
{
	GENERATED_BODY()

	// 是否要求本次效果前已成功消耗过状态层数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireConsumedStatus = false;

	// 要求被消耗的状态 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireConsumedStatus"))
	FFinalStatusId RequiredStatusId;

	// 要求至少消耗的状态层数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1", EditCondition = "bRequireConsumedStatus"))
	int32 MinimumStacks = 1;
};
