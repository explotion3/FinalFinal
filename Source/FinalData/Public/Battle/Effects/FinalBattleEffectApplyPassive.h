#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleTargetedEffectDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectApplyPassive.generated.h"

class UFinalPassiveDefinition;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectApplyPassive : public UFinalBattleTargetedEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectApplyPassive();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalPassiveId PassiveId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TObjectPtr<UFinalPassiveDefinition> PassiveDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 Stacks = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	int32 DurationOverride = 0;
};
