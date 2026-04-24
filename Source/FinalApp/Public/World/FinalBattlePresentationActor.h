#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "FinalBattlePresentationActor.generated.h"

class UTextRenderComponent;

UENUM(BlueprintType)
enum class EFinalBattlePresentationTeam : uint8
{
	Player,
	Enemy
};

UENUM(BlueprintType)
enum class EFinalBattlePresentationAnimState : uint8
{
	Idle,
	Selected,
	Attack,
	Hit,
	Defeat
};

UCLASS()
class FINALAPP_API AFinalBattlePresentationActor : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AFinalBattlePresentationActor();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void InitializePresentationActor(FName InRuntimeUnitId, EFinalBattlePresentationTeam InPresentationTeam);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void ApplySnapshotView(const FText& InDisplayName, const FText& InDetailText, bool bInIsAlive, bool bInIsSelected);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void PlayAttackPresentation();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void PlayHitPresentation();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void PlayDefeatPresentation();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void CompletePresentationTransientState();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Presentation")
	void CompletePresentationTransientStateIfMatches(EFinalBattlePresentationAnimState ExpectedPresentationState);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	FName GetRuntimeUnitId() const { return RuntimeUnitId; }

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	EFinalBattlePresentationTeam GetPresentationTeam() const { return PresentationTeam; }

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	EFinalBattlePresentationAnimState GetPresentationState() const { return CurrentPresentationState; }

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	bool IsPresentationAlive() const { return bIsAlive; }

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	bool IsPresentationSelected() const { return bIsSelected; }

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	FText GetDisplayNameText() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Presentation")
	FText GetDetailText() const { return DetailText; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|Presentation")
	void OnPresentationStateChanged(EFinalBattlePresentationAnimState NewPresentationState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|Presentation")
	void OnSnapshotViewApplied(const FText& InDisplayName, const FText& InDetailText, bool bInIsAlive, bool bInIsSelected);

private:
	void EnsureVisualDefaultsInitialized();
	void ApplyPresentationVisualState(EFinalBattlePresentationAnimState NewPresentationState);
	void RefreshPresentationState();
	void SetTransientPresentationState(EFinalBattlePresentationAnimState NewPresentationState, float DurationSeconds, bool bUseTimerFallback);
	void ClearTransientPresentationState();
	EFinalBattlePresentationAnimState ResolveBasePresentationState() const;
	void UpdateDebugLabel();

	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Presentation")
	TObjectPtr<UTextRenderComponent> DebugLabelComponent;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	bool bShowDebugLabel = true;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	FVector DebugLabelOffset = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	float AttackPresentationDuration = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	float HitPresentationDuration = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	bool bUseAnimationCompletionForAttack = true;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation")
	bool bUseAnimationCompletionForHit = true;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation|Visual")
	FLinearColor NormalVisualTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation|Visual")
	FLinearColor SelectedVisualTint = FLinearColor(1.1f, 1.1f, 1.1f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation|Visual")
	FLinearColor HitVisualTint = FLinearColor(1.0f, 0.55f, 0.55f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation|Visual")
	FLinearColor DefeatVisualTint = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation|Visual", meta = (ClampMin = "1.0"))
	float SelectedScaleMultiplier = 1.08f;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Presentation|Visual")
	bool bHideActorOnDefeat = false;

	UPROPERTY(Transient)
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(Transient)
	EFinalBattlePresentationTeam PresentationTeam = EFinalBattlePresentationTeam::Player;

	UPROPERTY(Transient)
	EFinalBattlePresentationAnimState CurrentPresentationState = EFinalBattlePresentationAnimState::Idle;

	UPROPERTY(Transient)
	EFinalBattlePresentationAnimState PersistentPresentationState = EFinalBattlePresentationAnimState::Idle;

	UPROPERTY(Transient)
	bool bHasTransientPresentationState = false;

	UPROPERTY(Transient)
	EFinalBattlePresentationAnimState TransientPresentationState = EFinalBattlePresentationAnimState::Idle;

	UPROPERTY(Transient)
	bool bIsAlive = true;

	UPROPERTY(Transient)
	bool bIsSelected = false;

	UPROPERTY(Transient)
	FText DisplayName;

	UPROPERTY(Transient)
	FText DetailText;

	UPROPERTY(Transient)
	bool bVisualDefaultsInitialized = false;

	UPROPERTY(Transient)
	FVector BaseSpriteRelativeScale = FVector::OneVector;

	UPROPERTY(Transient)
	FLinearColor BaseSpriteColor = FLinearColor::White;

	FTimerHandle TransientPresentationTimerHandle;
};
