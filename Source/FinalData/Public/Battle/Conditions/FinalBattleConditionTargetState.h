#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleTargetStateRequirement.h"
#include "FinalBattleConditionTargetState.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionTargetState : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	virtual EFinalBattleConditionContext GetConditionContext() const override { return EFinalBattleConditionContext::TargetRequired; }

	// 生效前要求当前目标满足的状态条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleTargetStateRequirement Requirement;
};
