#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleMovedCardRequirement.h"
#include "FinalBattleConditionMovedCards.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionMovedCards : public UFinalBattleConditionDefinition
{
	GENERATED_BODY()

public:
	virtual EFinalBattleConditionContext GetConditionContext() const override { return EFinalBattleConditionContext::ChainRecord; }

	// 生效前要求本次效果链已移动过满足条件的卡牌实例。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FFinalBattleMovedCardRequirement Requirement;
};
