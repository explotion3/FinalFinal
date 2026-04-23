#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleStatusChangeRequirement.h"
#include "FinalBattleConditionStatusChanged.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionStatusChanged : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	virtual EFinalBattleConditionContext GetConditionContext() const override { return EFinalBattleConditionContext::ChainRecord; }

	// 生效前要求本次效果链已产生过指定状态变化记录。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleStatusChangeRequirement Requirement;
};
