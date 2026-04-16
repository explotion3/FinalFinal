#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalSaveGameCoordinator.generated.h"

UCLASS()
class FINALAPP_API UFinalSaveGameCoordinator : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|Save")
	bool SaveCurrentRunToPrototypeSlot();

	UFUNCTION(BlueprintCallable, Category = "Final|Save")
	bool LoadRunFromPrototypeSlot();

	UFUNCTION(BlueprintPure, Category = "Final|Save")
	bool DoesPrototypeRunSaveExist() const;

	UFUNCTION(BlueprintPure, Category = "Final|Save")
	FText GetLastFailureReason() const;

	UFUNCTION(BlueprintPure, Category = "Final|Save")
	FText GetLastSaveLoadStatusText() const;

	UFUNCTION(BlueprintPure, Category = "Final|Save")
	FText GetPrototypeRunSaveDebugText() const;

	static const FString& GetPrototypeRunSlotName();

private:
	void SetFailureReason(const FText& Reason);
	void SetSuccessStatus(const FText& StatusText);
	void SetFailureStatus(const FText& OperationText, const FText& Reason);
	bool HasActiveBattleSession() const;

	UPROPERTY(Transient)
	FText LastFailureReason;

	UPROPERTY(Transient)
	FText LastSaveLoadStatusText;
};
