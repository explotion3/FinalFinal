#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "Run/Definitions/FinalRelicBattleTypes.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "FinalRelicDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalRelicDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Relic")
	FFinalRelicId RelicId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FName DisplayId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRarity Rarity = EFinalRarity::Rare;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRelicBattleStartEffectDefinition> BattleStartEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRelicPlayerTurnStartEffectDefinition> PlayerTurnStartEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRuntimeTriggerDefinition> RuntimeTriggers;
};
