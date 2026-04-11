#pragma once

#include "CoreMinimal.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Requests/FinalBattleResult.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalGameFlowSubsystem.generated.h"

UCLASS()
class FINALAPP_API UFinalGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	UFinalRunSession* BootstrapNewRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	UFinalBattleSession* StartBattleFromRunSession();

	bool TryAutoStartPreparedBattleFromRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	bool CompleteBattleAndApplyResult(const FFinalBattleResult& Result);

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	bool CompleteResolvedBattle();

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	UFinalRunSession* GetRunSession() const;

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	UFinalBattleSession* GetActiveBattleSession() const;

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	FFinalBattleSnapshot GetCurrentBattleSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	FText GetLastBattleFailureReason() const;

private:
	bool BuildResolvedBattleResult(FFinalBattleResult& OutResult);

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunSession> RunSession;

	UPROPERTY(Transient)
	FText LastFlowFailureReason;

	bool bAutoStartingPreparedBattle = false;
};
