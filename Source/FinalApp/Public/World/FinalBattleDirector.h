#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "GameFramework/Actor.h"
#include "FinalBattleDirector.generated.h"

class ATextRenderActor;
class UFinalBattleFlowSubsystem;
class USceneComponent;
class UTextRenderComponent;

UCLASS()
class FINALAPP_API AFinalBattleDirector : public AActor
{
	GENERATED_BODY()

public:
	AFinalBattleDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FPresentationUnitState
	{
		bool bIsEnemy = false;
		int32 SlotIndex = INDEX_NONE;
		FText DisplayName;
		FText DetailText;
		bool bIsAlive = true;
		bool bIsTargeted = false;
	};

	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	UFinalBattleFlowSubsystem* ResolveBattleFlowSubsystem() const;
	void RefreshPresentationFromSnapshot(const FFinalBattleSnapshot& Snapshot);
	void SyncPresentationActors();
	void UpdatePresentationActor(FName RuntimeUnitId, const FPresentationUnitState& UnitState);
	void ClearPresentationActors();
	void UpdateSummaryText();
	ATextRenderActor* GetOrSpawnPresentationActor(FName RuntimeUnitId);
	FVector ResolvePresentationLocation(const FPresentationUnitState& UnitState) const;
	FLinearColor ResolvePresentationColor(FName RuntimeUnitId, const FPresentationUnitState& UnitState) const;

	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Presentation")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Presentation")
	TObjectPtr<UTextRenderComponent> SummaryTextComponent;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	FVector SummaryTextOffset = FVector(0.0f, 0.0f, 320.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	FVector PlayerPresentationOrigin = FVector(-420.0f, -180.0f, 120.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	FVector EnemyPresentationOrigin = FVector(420.0f, -180.0f, 120.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	float PlayerSlotSpacing = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	float EnemySlotSpacing = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	FRotator PresentationRotation = FRotator(0.0f, 180.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	float UnitTextWorldSize = 38.0f;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFlowSubsystem> CachedBattleFlowSubsystem;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<ATextRenderActor>> PresentationActorsByRuntimeId;

	TMap<FName, FPresentationUnitState> PresentationUnitsByRuntimeId;
	FFinalBattleSnapshot CachedSnapshot;
	FFinalBattleEvent LastBattleEvent;
};
