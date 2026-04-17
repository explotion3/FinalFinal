#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FinalGameInstance.generated.h"

class UFinalPrototypeBootstrapDefinition;

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
	TObjectPtr<UFinalPrototypeBootstrapDefinition> TestPrototypeBootstrapDefinition;

	UPROPERTY(Transient)
	FText LastTestFailureReason;

	bool bTestBattleBootstrapRegistered = false;
};
