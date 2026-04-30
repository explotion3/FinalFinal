#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleResourceConsumeRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleResourceConsumeRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalStatusId RequiredStatusId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 MinimumStacks = 1;
};
