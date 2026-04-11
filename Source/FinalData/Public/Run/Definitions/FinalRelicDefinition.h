#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRelicBattleTypes.h"
#include "FinalRelicDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalRelicDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FFinalRelicId RelicId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FName DisplayId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRelicBattleStartEffectDefinition> BattleStartEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRelicPlayerTurnStartEffectDefinition> PlayerTurnStartEffects;
};
