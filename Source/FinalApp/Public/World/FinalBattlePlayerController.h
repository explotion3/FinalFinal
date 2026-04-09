#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalBattleCommand.h"
#include "GameFramework/PlayerController.h"
#include "FinalBattlePlayerController.generated.h"

UCLASS()
class FINALAPP_API AFinalBattlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Test")
	bool StartTestBattle();

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Test")
	FText GetLastTestBattleFailureReason() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool SubmitBattleCommand(const FFinalBattleCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool EndPlayerTurn();

	UFUNCTION(Exec)
	void FinalStartTestBattle();
};
