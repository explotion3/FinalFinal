#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalStatusDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalStatusDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	FFinalStatusId StatusId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	EFinalStatusCategory StatusCategory = EFinalStatusCategory::Buff;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	int32 MaxStacks = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	int32 DefaultDuration = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	FText SummaryText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Status")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> OnTickEffects;
};
