#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleTargetedEffectDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectConsumeStatusResource.generated.h"

class UFinalStatusDefinition;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectConsumeStatusResource : public UFinalBattleTargetedEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectConsumeStatusResource();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalStatusId StatusId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TObjectPtr<UFinalStatusDefinition> StatusDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 StacksToConsume = 1;
};
