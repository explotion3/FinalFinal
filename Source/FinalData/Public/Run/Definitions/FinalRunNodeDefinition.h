#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalRunNodeDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalRunNodeType : uint8
{
	None,
	Battle,
	Event,
	Shop,
	Reward
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunNodeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunNodeType NodeType = EFinalRunNodeType::Battle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FName> NextNodeIds;

	bool IsValid() const
	{
		return !NodeId.IsNone();
	}
};
