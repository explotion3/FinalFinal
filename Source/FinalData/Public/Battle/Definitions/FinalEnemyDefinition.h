#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalEnemyDefinition.generated.h"

class UFinalEnemyIntentDefinition;

UCLASS(BlueprintType)
class FINALDATA_API UFinalEnemyDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FFinalEnemyId EnemyId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FGameplayTagContainer RoleTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	int32 MaxHP = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	int32 MaxBreakValue = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	int32 BaseDamagePower = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	int32 InitialInitiativeValue = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	int32 InitiativeResponse = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	TArray<TSoftObjectPtr<UFinalEnemyIntentDefinition>> IntentPool;
};
