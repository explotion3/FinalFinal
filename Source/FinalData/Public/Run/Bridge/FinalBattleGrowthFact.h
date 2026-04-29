#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleGrowthFact.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleGrowthFactType : uint8
{
	None,
	OwnedCardResolved,
	BreakDamageDealt,
	EffectiveHealingDone,
	EnemyKilled,
	BattleVictoryBaseReward
};

UENUM(BlueprintType)
enum class EFinalBattleGrowthCommandSource : uint8
{
	None,
	PlayCard,
	PlayUltimate,
	EndTurn,
	EnemyPhase,
	Passive
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleGrowthFact
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	EFinalBattleGrowthFactType FactType = EFinalBattleGrowthFactType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	int32 Magnitude = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	FFinalCardId SourceCardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	bool bCausedByPlayerCommand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	EFinalBattleGrowthCommandSource CommandSource = EFinalBattleGrowthCommandSource::None;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleGrowthFactBatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	int32 BatchSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	bool bCausedByPlayerCommand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	EFinalBattleGrowthCommandSource CommandSource = EFinalBattleGrowthCommandSource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Bridge")
	TArray<FFinalBattleGrowthFact> Facts;
};
