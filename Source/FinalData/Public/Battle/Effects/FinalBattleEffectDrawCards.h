#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalBattleEffectDrawCards.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectDrawCards : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectDrawCards();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 DrawCount = 1;
};
