#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "FinalCharacterDefinition.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalInitialLoadoutCardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	FFinalCardId CardId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character", meta = (ClampMin = "1"))
	int32 Count = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	EFinalLoadoutRole LoadoutRole = EFinalLoadoutRole::BaseAttack;
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Character")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	FGameplayTagContainer RoleTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	int32 BaseVitalShare = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	int32 BaseStressCap = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	int32 BaseAttack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	int32 BaseDefense = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	float BaseBreakRate = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	float BaseCritChance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	float BaseCritDamage = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	int32 EpGainPerAP = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character|Growth")
	FFinalCharacterGrowthConfigId GrowthConfigId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	TArray<FFinalInitialLoadoutCardEntry> InitialLoadoutCards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	TArray<FFinalCardId> CharacterCardPoolIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	FFinalUltimateId UltimateId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	FFinalStatusId SignatureStatusId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Character")
	TArray<FFinalRuntimeTriggerDefinition> BattleTriggers;
};
