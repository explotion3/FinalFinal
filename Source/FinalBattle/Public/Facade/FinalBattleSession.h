#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalBattleCommand.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Run/Bridge/FinalBattleGrowthFact.h"
#include "UObject/Object.h"
#include "FinalBattleSession.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
struct FFinalBattleState;
class FFinalBattleResolver;

UCLASS(BlueprintType)
class FINALBATTLE_API UFinalBattleSession : public UObject
{
	GENERATED_BODY()

public:
	UFinalBattleSession();
	virtual ~UFinalBattleSession() override;

	void InitializeSession(UFinalBattleEncounterDefinition* InEncounterDefinition, UFinalBattleRuleConfig* InRuleConfig, const FFinalBattleInitContext& InitContext);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	FFinalBattleEvent SubmitCommand(const FFinalBattleCommand& Command);

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FFinalBattleSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	TArray<FFinalBattleEvent> GetBattleLogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	TArray<FFinalBattleEvent> GetBattleEventsSince(int32 LastSeenEventSequence) const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	int32 GetLatestBattleEventSequence() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	TArray<FFinalBattleGrowthFactBatch> GetGrowthFactBatchesSince(int32 LastSeenBatchSequence) const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	int32 GetLatestGrowthFactBatchSequence() const;

	bool RefreshCharacterRuntimeStats(const FFinalBattleCharacterRuntimeStats& RuntimeStats);
	int32 RefreshCardsForRunCardInstance(const FFinalBattleCardRefreshRequest& RefreshRequest);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	void ResetSession();

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	bool HasActiveBattle() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRuleConfig> RuleConfig;

	FFinalBattleState* State = nullptr;
	FFinalBattleResolver* Resolver = nullptr;
};
