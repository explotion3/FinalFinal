#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "GameFramework/Actor.h"
#include "World/FinalBattlePresentationTypes.h"
#include "FinalBattleDirector.generated.h"

class AFinalBattlePresentationActor;
class AFinalBattleStageAnchorActor;
class UFinalBattleFlowSubsystem;
class USceneComponent;

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattlePresentationClassMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle|Presentation")
	FName UnitDefinitionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle|Presentation")
	TSubclassOf<AFinalBattlePresentationActor> PresentationClass;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API AFinalBattleDirector : public AActor
{
	GENERATED_BODY()

public:
	AFinalBattleDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	UFinalBattleFlowSubsystem* ResolveBattleFlowSubsystem() const;
	void RefreshPresentationFromSnapshot(const FFinalBattleSnapshot& Snapshot);
	void SyncPresentationActors();
	void RefreshStageAnchors();
	void UpdatePresentationActor(FName RuntimeUnitId, const FFinalBattlePresentationUnitViewData& UnitView);
	void ApplyEventPresentation(const FFinalBattleEvent& BattleEvent);
	void ClearPresentationActors();
	AFinalBattlePresentationActor* GetOrSpawnPresentationActor(FName RuntimeUnitId, const FFinalBattlePresentationUnitViewData& UnitView);
	FTransform ResolvePresentationTransform(const FFinalBattlePresentationUnitViewData& UnitView) const;
	AFinalBattleStageAnchorActor* ResolveStageAnchor(const FFinalBattlePresentationUnitViewData& UnitView) const;
	TSubclassOf<AFinalBattlePresentationActor> ResolvePresentationActorClass(const FFinalBattlePresentationUnitViewData& UnitView) const;

	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Presentation")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	TSubclassOf<AFinalBattlePresentationActor> PresentationActorClass;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	TSubclassOf<AFinalBattlePresentationActor> DefaultPlayerPresentationClass;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	TSubclassOf<AFinalBattlePresentationActor> DefaultEnemyPresentationClass;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	TArray<FFinalBattlePresentationClassMapping> PlayerPresentationClassMappings;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	TArray<FFinalBattlePresentationClassMapping> EnemyPresentationClassMappings;

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

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFlowSubsystem> CachedBattleFlowSubsystem;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<AFinalBattlePresentationActor>> PresentationActorsByRuntimeId;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<AFinalBattleStageAnchorActor>> PlayerStageAnchorsByIndex;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<AFinalBattleStageAnchorActor>> EnemyStageAnchorsByIndex;

	TMap<FName, FFinalBattlePresentationUnitViewData> PresentationUnitsByRuntimeId;
};
