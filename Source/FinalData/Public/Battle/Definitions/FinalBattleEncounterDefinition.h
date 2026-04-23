#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEncounterDefinition.generated.h"

class UFinalBattleRuleConfig;
class UFinalEnemyDefinition;

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalEnemyRosterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Encounter")
	TSoftObjectPtr<UFinalEnemyDefinition> EnemyDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Encounter")
	int32 PositionIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Encounter")
	int32 SpawnWave = 1;
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalBattleEncounterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Encounter")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Encounter")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Encounter")
	TSoftObjectPtr<UFinalBattleRuleConfig> RuleConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Encounter")
	TArray<FFinalEnemyRosterEntry> EnemyRoster;
};
