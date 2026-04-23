#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Effects/FinalBattleGeneratedCardConsumeRequirement.h"
#include "FinalBattleConditionConsumedGeneratedCard.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionConsumedGeneratedCard : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	// 生效前要求本次效果链已消耗过衍生牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleGeneratedCardConsumeRequirement Requirement;
};
