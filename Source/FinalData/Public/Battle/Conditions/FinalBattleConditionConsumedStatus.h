#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Effects/FinalBattleStatusConsumeRequirement.h"
#include "FinalBattleConditionConsumedStatus.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionConsumedStatus : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	// 生效前要求本次效果链已消耗过指定状态层数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleStatusConsumeRequirement Requirement;
};
