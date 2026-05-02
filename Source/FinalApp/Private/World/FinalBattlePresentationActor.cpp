#include "World/FinalBattlePresentationActor.h"

#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "PaperFlipbookComponent.h"
#include "TimerManager.h"
#include "UI/Widgets/Battle/FinalBattleEnemyOverheadWidget.h"
#include "World/FinalBattleOverheadWidgetComponent.h"

AFinalBattlePresentationActor::AFinalBattlePresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	EnemyOverheadWidgetComponent = CreateDefaultSubobject<UFinalBattleOverheadWidgetComponent>(TEXT("EnemyOverheadWidget"));
	EnemyOverheadWidgetComponent->SetupAttachment(GetRootComponent());
	EnemyOverheadWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyOverheadWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
	EnemyOverheadWidgetComponent->SetDrawSize(FVector2D(360.0f, 120.0f));
	EnemyOverheadWidgetComponent->SetDrawAtDesiredSize(true);
	EnemyOverheadWidgetComponent->SetWindowFocusable(false);
	EnemyOverheadWidgetComponent->SetVisibility(false);
	EnemyOverheadWidgetComponent->SetHiddenInGame(true);
}

void AFinalBattlePresentationActor::BeginPlay()
{
	Super::BeginPlay();
	if (EnemyOverheadWidgetComponent)
	{
		EnemyOverheadWidgetComponent->SetVisibility(false);
		EnemyOverheadWidgetComponent->SetHiddenInGame(true);
	}
	EnsureVisualDefaultsInitialized();
	ApplyPresentationVisualState(CurrentPresentationState);
}

void AFinalBattlePresentationActor::InitializePresentationActor(
	const FName InRuntimeUnitId,
	const EFinalBattlePresentationTeam InPresentationTeam)
{
	RuntimeUnitId = InRuntimeUnitId;
	PresentationTeam = InPresentationTeam;
	PresentationViewData.RuntimeUnitId = RuntimeUnitId;
	PresentationViewData.Team = PresentationTeam;
	RefreshEnemyOverheadWidget();
}

void AFinalBattlePresentationActor::ApplyPresentationView(const FFinalBattlePresentationUnitViewData& ViewData)
{
	const bool bWasAlive = bIsAlive;

	PresentationViewData = ViewData;
	RuntimeUnitId = ViewData.RuntimeUnitId;
	PresentationTeam = ViewData.Team;
	bIsAlive = ViewData.bIsAlive;
	bIsSelected = ViewData.bIsTargeted;

	if (bWasAlive && !bIsAlive)
	{
		PlayDefeatPresentation();
	}
	else
	{
		RefreshPresentationState();
	}

	OnPresentationViewApplied(PresentationViewData);
	RefreshEnemyOverheadWidget();
}

void AFinalBattlePresentationActor::PlayAttackPresentation()
{
	if (!bIsAlive)
	{
		return;
	}

	SetTransientPresentationState(
		EFinalBattlePresentationAnimState::Attack,
		AttackPresentationDuration,
		!bUseAnimationCompletionForAttack);
}

void AFinalBattlePresentationActor::PlayHitPresentation()
{
	if (!bIsAlive)
	{
		return;
	}

	SetTransientPresentationState(
		EFinalBattlePresentationAnimState::Hit,
		HitPresentationDuration,
		!bUseAnimationCompletionForHit);
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

void AFinalBattlePresentationActor::CompletePresentationTransientState()
{
	ClearTransientPresentationState();
}

void AFinalBattlePresentationActor::CompletePresentationTransientStateIfMatches(
	const EFinalBattlePresentationAnimState ExpectedPresentationState)
{
	if (!bHasTransientPresentationState || TransientPresentationState != ExpectedPresentationState)
	{
		return;
	}

	ClearTransientPresentationState();
}

void AFinalBattlePresentationActor::EnsureVisualDefaultsInitialized()
{
	if (bVisualDefaultsInitialized)
	{
		return;
	}

	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		BaseSpriteRelativeScale = SpriteComponent->GetRelativeScale3D();
		BaseSpriteColor = SpriteComponent->GetSpriteColor();
	}

	bVisualDefaultsInitialized = true;
}

void AFinalBattlePresentationActor::ApplyPresentationVisualState(const EFinalBattlePresentationAnimState NewPresentationState)
{
	EnsureVisualDefaultsInitialized();

	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (SpriteComponent == nullptr)
	{
		return;
	}

	SetActorHiddenInGame(false);
	SpriteComponent->SetRelativeScale3D(BaseSpriteRelativeScale);

	const auto ApplyTint = [SpriteComponent, this](const FLinearColor& TintColor)
	{
		const FLinearColor FinalColor(
			BaseSpriteColor.R * TintColor.R,
			BaseSpriteColor.G * TintColor.G,
			BaseSpriteColor.B * TintColor.B,
			BaseSpriteColor.A * TintColor.A);
		SpriteComponent->SetSpriteColor(FinalColor.ToFColor(true));
	};

	switch (NewPresentationState)
	{
	case EFinalBattlePresentationAnimState::Selected:
		SpriteComponent->SetRelativeScale3D(BaseSpriteRelativeScale * SelectedScaleMultiplier);
		ApplyTint(SelectedVisualTint);
		break;

	case EFinalBattlePresentationAnimState::Hit:
		ApplyTint(HitVisualTint);
		break;

	case EFinalBattlePresentationAnimState::Defeat:
		ApplyTint(DefeatVisualTint);
		if (bHideActorOnDefeat)
		{
			SetActorHiddenInGame(true);
		}
		break;

	case EFinalBattlePresentationAnimState::Attack:
	case EFinalBattlePresentationAnimState::Idle:
	default:
		ApplyTint(NormalVisualTint);
		break;
	}
}

void AFinalBattlePresentationActor::RefreshPresentationState()
{
	const EFinalBattlePresentationAnimState NewState = bHasTransientPresentationState
		? TransientPresentationState
		: ResolveBasePresentationState();

	PersistentPresentationState = ResolveBasePresentationState();
	ApplyPresentationVisualState(NewState);
	if (CurrentPresentationState != NewState)
	{
		CurrentPresentationState = NewState;
		OnPresentationStateChanged(CurrentPresentationState);
	}
}

void AFinalBattlePresentationActor::RefreshEnemyOverheadWidget()
{
	if (EnemyOverheadWidgetComponent == nullptr)
	{
		return;
	}

	const bool bShouldShow =
		PresentationTeam == EFinalBattlePresentationTeam::Enemy
		&& !PresentationViewData.RuntimeUnitId.IsNone()
		&& EnemyOverheadWidgetComponent->GetWidgetClass() != nullptr;

	EnemyOverheadWidgetComponent->SetVisibility(bShouldShow);
	EnemyOverheadWidgetComponent->SetHiddenInGame(!bShouldShow);

	if (!bShouldShow)
	{
		return;
	}

	EnemyOverheadWidgetComponent->InitWidget();

	if (UFinalBattleEnemyOverheadWidget* EnemyOverheadWidget = ResolveEnemyOverheadWidget())
	{
		EnemyOverheadWidget->ApplyEnemyOverheadView(PresentationViewData.EnemyOverheadView);
	}
}

UFinalBattleEnemyOverheadWidget* AFinalBattlePresentationActor::ResolveEnemyOverheadWidget() const
{
	return EnemyOverheadWidgetComponent
		? Cast<UFinalBattleEnemyOverheadWidget>(EnemyOverheadWidgetComponent->GetUserWidgetObject())
		: nullptr;
}

void AFinalBattlePresentationActor::SetTransientPresentationState(
	const EFinalBattlePresentationAnimState NewPresentationState,
	const float DurationSeconds,
	const bool bUseTimerFallback)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TransientPresentationTimerHandle);
	}

	bHasTransientPresentationState = true;
	TransientPresentationState = NewPresentationState;
	RefreshPresentationState();

	if (bUseTimerFallback)
	{
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
}

void AFinalBattlePresentationActor::ClearTransientPresentationState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TransientPresentationTimerHandle);
	}

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
