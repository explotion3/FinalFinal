#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalCardDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalCardDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Card")
	FFinalCardId CardId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	EFinalCardType CardType = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	EFinalRarity Rarity = EFinalRarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	int32 BaseCostAP = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FGameplayTagContainer Keywords;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FText RulesText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Card")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
