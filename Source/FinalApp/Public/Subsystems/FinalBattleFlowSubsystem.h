#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Requests/FinalBattleStartRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalBattleFlowSubsystem.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;

UCLASS()
class FINALAPP_API UFinalBattleFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	UFinalBattleSession* CreateBattleSessionFromStartRequest(const FFinalBattleStartRequest& StartRequest);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool SubmitBattleCommand(const FFinalBattleCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	void ClearActiveBattleSession();

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FFinalBattleSnapshot GetCurrentSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	UFinalBattleSession* GetActiveBattleSession() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FText GetLastFailureReason() const;

private:
	bool BuildInitContext(const FFinalBattleStartRequest& StartRequest, FFinalBattleInitContext& OutInitContext);

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleSession> ActiveBattleSession;

	UPROPERTY(Transient)
	FFinalBattleStartRequest LastStartRequest;

	UPROPERTY(Transient)
	FText LastFailureReason;
};
