#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleResourceConsumeRequirement.h"
#include "FinalBattleConditionResourceConsumed.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionResourceConsumed : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	virtual EFinalBattleConditionContext GetConditionContext() const override { return EFinalBattleConditionContext::ChainRecord; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleResourceConsumeRequirement Requirement;
};
