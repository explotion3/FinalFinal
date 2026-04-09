#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalUltimateDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalUltimateDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Ultimate")
	FFinalUltimateId UltimateId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Ultimate")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Ultimate")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Ultimate")
	int32 BaseCostEP = 45;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Ultimate")
	FText RulesText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Ultimate")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
