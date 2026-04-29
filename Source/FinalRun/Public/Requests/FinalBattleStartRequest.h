#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "FinalBattleStartRequest.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalBattleStartDeckEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName SourceRunCardInstanceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCardId EffectiveCardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCharacterId OwnerCharacterId;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalBattleStartRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalCharacterId> PartyCharacterIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunPersistentCharacterState> PartyStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalBattleStartDeckEntry> DeckEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalCardId> DeckCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalBattleStartRelicInput> BattleStartRelics;
};
