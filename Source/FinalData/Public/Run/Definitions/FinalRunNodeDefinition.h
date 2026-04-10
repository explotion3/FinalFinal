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
	Reward,
	EliteBattle,
	BossBattle
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
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName DisplayLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 ChapterIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 FloorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bStartsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText LockedReason;

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

	bool IsBattleNode() const
	{
		return NodeType == EFinalRunNodeType::Battle
			|| NodeType == EFinalRunNodeType::EliteBattle
			|| NodeType == EFinalRunNodeType::BossBattle;
	}
};
