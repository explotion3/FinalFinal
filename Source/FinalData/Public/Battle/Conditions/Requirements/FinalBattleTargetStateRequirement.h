#pragma once

#include "CoreMinimal.h"
#include "FinalBattleTargetStateRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleTargetStateRequirement
{
	GENERATED_BODY()

	// 是否要求目标必须是敌方单位。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireEnemyTarget = false;

	// 是否要求目标已处于 Break 状态。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireEnemyTarget"))
	bool bRequireTargetBroken = false;

	// 是否要求目标当前仍存活。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireEnemyTarget"))
	bool bRequireTargetAlive = false;
};
