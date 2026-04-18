#pragma once

#include "CoreMinimal.h"
#include "FinalBattleTargetStateRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleTargetStateRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireEnemyTarget = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireEnemyTarget"))
	bool bRequireTargetBroken = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireEnemyTarget"))
	bool bRequireTargetAlive = false;
};
