#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FinalGameInstance.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
class UFinalCardDefinition;
class UFinalCharacterDefinition;
class UFinalRunRouteDefinition;

UCLASS()
class FINALAPP_API UFinalGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Test")
	bool EnsureTestBattleBootstrapData();

	UFUNCTION(BlueprintCallable, Category = "Final|Test")
	bool PrepareTestBattleRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Test")
	bool StartTestBattle();

	UFUNCTION(BlueprintPure, Category = "Final|Test")
	FText GetLastTestFailureReason() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRuleConfig> TestRuleConfig;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEncounterDefinition> TestEncounterDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UFinalCharacterDefinition> TestGuardianDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UFinalCharacterDefinition> TestSupportDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UFinalCardDefinition> TestGuardianStrikeCard;

	UPROPERTY(Transient)
	TObjectPtr<UFinalCardDefinition> TestGuardianGuardCard;

	UPROPERTY(Transient)
	TObjectPtr<UFinalCardDefinition> TestSupportShotCard;

	UPROPERTY(Transient)
	TObjectPtr<UFinalCardDefinition> TestSupportFocusCard;

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunRouteDefinition> TestPrototypeRunRoute;

	UPROPERTY(Transient)
	FText LastTestFailureReason;

	bool bTestBattleBootstrapRegistered = false;
};
