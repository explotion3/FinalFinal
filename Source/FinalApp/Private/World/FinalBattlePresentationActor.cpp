#include "World/FinalBattlePresentationActor.h"

#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AFinalBattlePresentationActor::AFinalBattlePresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DebugLabelComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugLabel"));
	DebugLabelComponent->SetupAttachment(GetRootComponent());
	DebugLabelComponent->SetHorizontalAlignment(EHTA_Center);
	DebugLabelComponent->SetVerticalAlignment(EVRTA_TextCenter);
	DebugLabelComponent->SetTextRenderColor(FColor(232, 239, 255));
	DebugLabelComponent->SetWorldSize(34.0f);
	DebugLabelComponent->SetRelativeLocation(DebugLabelOffset);
	DebugLabelComponent->SetText(FText::FromString(TEXT("BattlePresentation")));
}

void AFinalBattlePresentationActor::InitializePresentationActor(
	const FName InRuntimeUnitId,
	const EFinalBattlePresentationTeam InPresentationTeam)
{
	RuntimeUnitId = InRuntimeUnitId;
	PresentationTeam = InPresentationTeam;
	UpdateDebugLabel();
}

void AFinalBattlePresentationActor::ApplySnapshotView(
	const FText& InDisplayName,
	const FText& InDetailText,
	const bool bInIsAlive,
	const bool bInIsSelected)
{
	const bool bWasAlive = bIsAlive;

	DisplayName = InDisplayName;
	DetailText = InDetailText;
	bIsAlive = bInIsAlive;
	bIsSelected = bInIsSelected;

	if (bWasAlive && !bIsAlive)
	{
		PlayDefeatPresentation();
	}
	else
	{
		RefreshPresentationState();
	}

	OnSnapshotViewApplied(DisplayName, DetailText, bIsAlive, bIsSelected);
	UpdateDebugLabel();
}

void AFinalBattlePresentationActor::PlayAttackPresentation()
{
	if (!bIsAlive)
	{
		return;
	}

	SetTransientPresentationState(EFinalBattlePresentationAnimState::Attack, AttackPresentationDuration);
}

void AFinalBattlePresentationActor::PlayHitPresentation()
{
	if (!bIsAlive)
	{
		return;
	}

	SetTransientPresentationState(EFinalBattlePresentationAnimState::Hit, HitPresentationDuration);
}

void AFinalBattlePresentationActor::PlayDefeatPresentation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TransientPresentationTimerHandle);
	}

	bHasTransientPresentationState = false;
	PersistentPresentationState = EFinalBattlePresentationAnimState::Defeat;
	RefreshPresentationState();
}

void AFinalBattlePresentationActor::RefreshPresentationState()
{
	const EFinalBattlePresentationAnimState NewState = bHasTransientPresentationState
		? TransientPresentationState
		: ResolveBasePresentationState();

	PersistentPresentationState = ResolveBasePresentationState();
	if (CurrentPresentationState != NewState)
	{
		CurrentPresentationState = NewState;
		OnPresentationStateChanged(CurrentPresentationState);
	}
}

void AFinalBattlePresentationActor::SetTransientPresentationState(
	const EFinalBattlePresentationAnimState NewPresentationState,
	const float DurationSeconds)
{
	bHasTransientPresentationState = true;
	TransientPresentationState = NewPresentationState;
	RefreshPresentationState();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TransientPresentationTimerHandle,
			this,
			&AFinalBattlePresentationActor::ClearTransientPresentationState,
			FMath::Max(DurationSeconds, 0.01f),
			false);
	}
}

void AFinalBattlePresentationActor::ClearTransientPresentationState()
{
	bHasTransientPresentationState = false;
	RefreshPresentationState();
}

EFinalBattlePresentationAnimState AFinalBattlePresentationActor::ResolveBasePresentationState() const
{
	if (!bIsAlive)
	{
		return EFinalBattlePresentationAnimState::Defeat;
	}

	return bIsSelected
		? EFinalBattlePresentationAnimState::Selected
		: EFinalBattlePresentationAnimState::Idle;
}

void AFinalBattlePresentationActor::UpdateDebugLabel()
{
	if (DebugLabelComponent == nullptr)
	{
		return;
	}

	DebugLabelComponent->SetVisibility(bShowDebugLabel);
	DebugLabelComponent->SetRelativeLocation(DebugLabelOffset);
	if (!bShowDebugLabel)
	{
		return;
	}

	const FString StateString = StaticEnum<EFinalBattlePresentationAnimState>()->GetNameStringByValue(static_cast<int64>(CurrentPresentationState));
	const FString TeamString = PresentationTeam == EFinalBattlePresentationTeam::Enemy ? TEXT("Enemy") : TEXT("Player");
	const FString NameString = DisplayName.IsEmpty()
		? RuntimeUnitId.ToString()
		: DisplayName.ToString();
	const FString DetailString = DetailText.IsEmpty()
		? FString()
		: DetailText.ToString();

	DebugLabelComponent->SetText(FText::FromString(FString::Printf(
		TEXT("[%s] %s\nState: %s\n%s"),
		*TeamString,
		*NameString,
		*StateString,
		*DetailString)));
}
