#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleHandCardRequirement.h"
#include "FinalBattleConditionHandCard.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionHandCard : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	// 生效前要求当前手牌满足的条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleHandCardRequirement Requirement;
};
