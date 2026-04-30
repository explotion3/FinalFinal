#pragma once

#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalBattleEffectApplyCardModifiers.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectApplyCardModifiers : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectApplyCardModifiers();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect|CardModifiers")
	TArray<FFinalTriggeredCardModifierDefinition> CardModifiers;
};
