#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalRunCommand.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunSnapshot.h"
#include "Requests/FinalBattleResult.h"
#include "Requests/FinalBattleStartRequest.h"
#include "Runtime/FinalRunState.h"
#include "UObject/Object.h"
#include "FinalRunSession.generated.h"

UCLASS(BlueprintType)
class FINALRUN_API UFinalRunSession : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void InitializeRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void ConfigureBattleStartState(const FFinalEncounterId& EncounterId, const FFinalRuleConfigId& RuleConfigId, const TArray<FFinalRunPersistentCharacterState>& PartyStates, const TArray<FFinalCardId>& DeckCardIds, int32 InTeamCurrentHP = 0);

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	bool SubmitRunCommand(const FFinalRunCommand& Command);

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	bool HasValidBattleStartState() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	FFinalBattleStartRequest BuildBattleStartRequest() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void ApplyBattleResult(const FFinalBattleResult& Result);

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	FFinalRunSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	FFinalRunState GetRunState() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	TArray<FFinalRunEvent> GetRunLogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	TArray<FFinalRunEvent> GetRunEventsSince(int32 LastSeenEventSequence) const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	int32 GetLatestRunEventSequence() const;

private:
	void AppendEvent(const FFinalRunEvent& Event);

	UPROPERTY(Transient)
	FFinalRunState CurrentState;

	UPROPERTY(Transient)
	TArray<FFinalRunEvent> RunLogEntries;

	int32 LastEventSequence = 0;
};
