#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "FinalPrototypeBootstrapDefinition.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalPrototypeBootstrapCharacterState
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap|Growth", meta = (ClampMin = "1"))
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap|Growth", meta = (ClampMin = "0"))
	int32 BreakthroughValue = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap|Growth", meta = (ClampMin = "0"))
	int32 BreakthroughRequiredValue = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap|Growth", meta = (ClampMin = "0"))
	int32 RootBone = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap|Growth", meta = (ClampMin = "0"))
	int32 Insight = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap|Growth", meta = (ClampMin = "0"))
	int32 KillingIntent = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap", meta = (ClampMin = "0"))
	int32 CurrentStress = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	bool bCollapsed = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap", meta = (ClampMin = "0"))
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap", meta = (ClampMin = "0"))
	int32 CollapseCount = 0;
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalPrototypeBootstrapDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|PrototypeBootstrap")
	FName BootstrapId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	FName RunRouteId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	TArray<FFinalCharacterId> PartyCharacterIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	TArray<FFinalPrototypeBootstrapCharacterState> InitialCharacterStates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap")
	TArray<FFinalCardId> StarterDeckCardIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|PrototypeBootstrap", meta = (ClampMin = "0"))
	int32 InitialTeamCurrentHP = 0;

	bool IsValidDefinition() const
	{
		return !BootstrapId.IsNone()
			&& RuleConfigId.IsValid()
			&& EncounterId.IsValid()
			&& !RunRouteId.IsNone()
			&& PartyCharacterIds.Num() > 0
			&& InitialCharacterStates.Num() > 0
			&& StarterDeckCardIds.Num() > 0
			&& InitialTeamCurrentHP > 0;
	}
};
