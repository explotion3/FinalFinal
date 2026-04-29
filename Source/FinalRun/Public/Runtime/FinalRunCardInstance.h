#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalCardEvolutionDefinition.h"
#include "FinalRunCardInstance.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunCardInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Deck")
	FName InstanceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Deck")
	FFinalCardId BaseCardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Deck")
	FFinalCardId CurrentCardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Deck")
	FFinalCharacterId OwnerCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Deck")
	EFinalCardEvolutionStage EvolutionStage = EFinalCardEvolutionStage::Base;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Deck")
	int32 TimesPlayedThisRun = 0;

	bool IsValid() const
	{
		return BaseCardId.IsValid() || CurrentCardId.IsValid();
	}

	FFinalCardId GetEffectiveCardId() const
	{
		return CurrentCardId.IsValid() ? CurrentCardId : BaseCardId;
	}

	static FFinalRunCardInstance Make(
		const FName InInstanceId,
		const FFinalCardId& InCardId,
		const FFinalCharacterId& InOwnerCharacterId = FFinalCharacterId{})
	{
		FFinalRunCardInstance Instance;
		Instance.InstanceId = InInstanceId;
		Instance.BaseCardId = InCardId;
		Instance.CurrentCardId = InCardId;
		Instance.OwnerCharacterId = InOwnerCharacterId;
		Instance.EvolutionStage = EFinalCardEvolutionStage::Base;
		return Instance;
	}
};
