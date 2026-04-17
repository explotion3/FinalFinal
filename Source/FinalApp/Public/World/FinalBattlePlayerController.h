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
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Test")
	bool StartTestBattle();

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Test")
	FText GetLastTestBattleFailureReason() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Test")
	bool DumpBattleSnapshotToLog() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Test")
	bool DumpBattleLogToLog() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Test")
	bool PlayFirstHandCard();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool PlayHandCardByIndex(int32 HandIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Test")
	bool CompleteResolvedBattle();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool SubmitBattleCommand(const FFinalBattleCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool EndPlayerTurn();

	UFUNCTION(Exec)
	void FinalStartTestBattle();

	UFUNCTION(Exec)
	void FinalSetPrototypeBootstrap(const FString& BootstrapId);

	UFUNCTION(Exec)
	void FinalDumpBattleSnapshot();

	UFUNCTION(Exec)
	void FinalDumpBattleLog();

	UFUNCTION(Exec)
	void FinalPlayFirstHandCard();

	UFUNCTION(Exec)
	void FinalEndTurnCommand();

	UFUNCTION(Exec)
	void FinalCompleteResolvedBattle();

	UFUNCTION(Exec)
	void FinalSavePrototypeRun();

	UFUNCTION(Exec)
	void FinalLoadPrototypeRun();

private:
	void RegisterUIBridge();
	void HandlePlayCardSlot1();
	void HandlePlayCardSlot2();
	void HandlePlayCardSlot3();
	void HandlePlayCardSlot4();
	void HandlePlayCardSlot5();
	void HandlePlayCardSlot6();
	void HandleQuickEndTurn();
};
