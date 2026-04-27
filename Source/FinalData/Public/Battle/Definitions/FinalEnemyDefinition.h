#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "FinalEnemyDefinition.generated.h"

class UFinalEnemyIntentDefinition;

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalEnemyScriptedIntentStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Enemy")
	FName IntentId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Enemy")
	FName PhaseTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Enemy")
	bool bRepeatLastStep = false;
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalEnemyDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Enemy")
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
	EFinalIntentSelectRule IntentSelectRule = EFinalIntentSelectRule::Cycle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	TArray<FFinalEnemyPhaseDefinition> PhaseSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	TArray<FFinalEnemyScriptedIntentStep> ScriptedIntentSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	TArray<TSoftObjectPtr<UFinalEnemyIntentDefinition>> IntentPool;
};
