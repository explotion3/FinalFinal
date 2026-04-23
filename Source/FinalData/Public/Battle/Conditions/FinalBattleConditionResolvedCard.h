#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleResolvedCardRequirement.h"
#include "FinalBattleConditionResolvedCard.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionResolvedCard : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	virtual EFinalBattleConditionContext GetConditionContext() const override { return EFinalBattleConditionContext::ResolvedCard; }

	// 生效前要求当前 resolved-card 上下文满足的条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleResolvedCardRequirement Requirement;
};
