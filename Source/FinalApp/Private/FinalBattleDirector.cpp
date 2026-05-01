#include "World/FinalBattleDirector.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "World/FinalBattlePresentationActor.h"
#include "World/FinalBattleStageAnchorActor.h"

AFinalBattleDirector::AFinalBattleDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootSceneComponent);

	PresentationActorClass = AFinalBattlePresentationActor::StaticClass();
	DefaultPlayerPresentationClass = PresentationActorClass;
	DefaultEnemyPresentationClass = PresentationActorClass;
}

void AFinalBattleDirector::BeginPlay()
{
	Super::BeginPlay();

	RefreshStageAnchors();

	CachedBattleFlowSubsystem = ResolveBattleFlowSubsystem();
	if (CachedBattleFlowSubsystem)
	{
		CachedBattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &AFinalBattleDirector::HandleBattleSnapshotChanged);
		CachedBattleFlowSubsystem->OnBattleEventBroadcast.AddDynamic(this, &AFinalBattleDirector::HandleBattleEventBroadcast);
		HandleBattleSnapshotChanged(CachedBattleFlowSubsystem->GetCurrentSnapshot());
	}
}

void AFinalBattleDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedBattleFlowSubsystem)
	{
		CachedBattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &AFinalBattleDirector::HandleBattleSnapshotChanged);
		CachedBattleFlowSubsystem->OnBattleEventBroadcast.RemoveDynamic(this, &AFinalBattleDirector::HandleBattleEventBroadcast);
	}

	ClearPresentationActors();
	PlayerStageAnchorsByIndex.Reset();
	EnemyStageAnchorsByIndex.Reset();
	Super::EndPlay(EndPlayReason);
}

void AFinalBattleDirector::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	RefreshPresentationFromSnapshot(Snapshot);
}

void AFinalBattleDirector::HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent)
{
	ApplyEventPresentation(BattleEvent);
}

UFinalBattleFlowSubsystem* AFinalBattleDirector::ResolveBattleFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
}

void AFinalBattleDirector::RefreshPresentationFromSnapshot(const FFinalBattleSnapshot& Snapshot)
{
	PresentationUnitsByRuntimeId.Reset();

	for (int32 CharacterIndex = 0; CharacterIndex < Snapshot.Characters.Num(); ++CharacterIndex)
	{
		const FFinalBattleCharacterViewData& CharacterView = Snapshot.Characters[CharacterIndex];
		if (CharacterView.RuntimeUnitId.IsNone())
		{
			continue;
		}

		FFinalBattlePresentationUnitViewData& UnitView = PresentationUnitsByRuntimeId.FindOrAdd(CharacterView.RuntimeUnitId);
		UnitView.RuntimeUnitId = CharacterView.RuntimeUnitId;
		UnitView.UnitDefinitionId = CharacterView.CharacterId.Value;
		UnitView.Team = EFinalBattlePresentationTeam::Player;
		UnitView.SlotIndex = CharacterIndex;
		UnitView.DisplayName = !CharacterView.DisplayName.IsEmpty()
			? CharacterView.DisplayName
			: FText::FromName(CharacterView.RuntimeUnitId);
		UnitView.bIsAlive = true;
		UnitView.bIsTargeted = Snapshot.CurrentTargetUnitId == CharacterView.RuntimeUnitId;
		UnitView.CurrentStress = CharacterView.CurrentStress;
		UnitView.StressCap = CharacterView.StressCap;
		UnitView.VitalShare = CharacterView.VitalShare;
		UnitView.CurrentAwakenCount = CharacterView.CurrentAwakenCount;
		UnitView.CurrentAwakenThreshold = CharacterView.CurrentAwakenThreshold;
		UnitView.CollapseCount = CharacterView.CollapseCount;
		UnitView.bCollapsed = CharacterView.bCollapsed;
	}

	for (const FFinalBattleEnemyViewData& EnemyView : Snapshot.Enemies)
	{
		if (EnemyView.RuntimeUnitId.IsNone())
		{
			continue;
		}

		FFinalBattlePresentationUnitViewData& UnitView = PresentationUnitsByRuntimeId.FindOrAdd(EnemyView.RuntimeUnitId);
		UnitView.RuntimeUnitId = EnemyView.RuntimeUnitId;
		UnitView.UnitDefinitionId = EnemyView.EnemyId.Value;
		UnitView.Team = EFinalBattlePresentationTeam::Enemy;
		UnitView.SlotIndex = EnemyView.PositionIndex;
		UnitView.DisplayName = !EnemyView.DisplayName.IsEmpty()
			? EnemyView.DisplayName
			: FText::FromName(EnemyView.RuntimeUnitId);
		UnitView.bIsAlive = EnemyView.CurrentHP > 0;
		UnitView.bIsTargeted = Snapshot.CurrentTargetUnitId == EnemyView.RuntimeUnitId;
		UnitView.CurrentHP = EnemyView.CurrentHP;
		UnitView.MaxHP = EnemyView.MaxHP;
		UnitView.CurrentShield = EnemyView.CurrentShield;
		UnitView.CurrentBreakValue = EnemyView.CurrentBreakValue;
		UnitView.MaxBreakValue = EnemyView.MaxBreakValue;
		UnitView.CurrentInitiative = EnemyView.CurrentInitiative;
		UnitView.IntentText = EnemyView.IntentText;
	}

	SyncPresentationActors();
}

void AFinalBattleDirector::SyncPresentationActors()
{
	TSet<FName> ActiveRuntimeUnitIds;

	for (const TPair<FName, FFinalBattlePresentationUnitViewData>& Pair : PresentationUnitsByRuntimeId)
	{
		ActiveRuntimeUnitIds.Add(Pair.Key);
		UpdatePresentationActor(Pair.Key, Pair.Value);
	}

	TArray<FName> StaleRuntimeUnitIds;
	PresentationActorsByRuntimeId.GenerateKeyArray(StaleRuntimeUnitIds);
	for (const FName RuntimeUnitId : StaleRuntimeUnitIds)
	{
		if (ActiveRuntimeUnitIds.Contains(RuntimeUnitId))
		{
			continue;
		}

		if (AFinalBattlePresentationActor* Actor = PresentationActorsByRuntimeId.FindRef(RuntimeUnitId))
		{
			Actor->Destroy();
		}

		PresentationActorsByRuntimeId.Remove(RuntimeUnitId);
	}
}

void AFinalBattleDirector::RefreshStageAnchors()
{
	PlayerStageAnchorsByIndex.Reset();
	EnemyStageAnchorsByIndex.Reset();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<AFinalBattleStageAnchorActor> It(World); It; ++It)
	{
		AFinalBattleStageAnchorActor* AnchorActor = *It;
		if (AnchorActor == nullptr)
		{
			continue;
		}

		TMap<int32, TObjectPtr<AFinalBattleStageAnchorActor>>& TargetMap =
			AnchorActor->GetAnchorSide() == EFinalBattleStageAnchorSide::Enemy
				? EnemyStageAnchorsByIndex
				: PlayerStageAnchorsByIndex;

		TargetMap.Add(AnchorActor->GetSlotIndex(), AnchorActor);
	}
}

void AFinalBattleDirector::UpdatePresentationActor(const FName RuntimeUnitId, const FFinalBattlePresentationUnitViewData& UnitView)
{
	AFinalBattlePresentationActor* PresentationActor = GetOrSpawnPresentationActor(RuntimeUnitId, UnitView);
	if (PresentationActor == nullptr)
	{
		return;
	}

	PresentationActor->ApplyPresentationView(UnitView);

	PresentationActor->SetActorTransform(ResolvePresentationTransform(UnitView));
}

void AFinalBattleDirector::ApplyEventPresentation(const FFinalBattleEvent& BattleEvent)
{
	AFinalBattlePresentationActor* SourceActor = PresentationActorsByRuntimeId.FindRef(BattleEvent.SourceUnitId);
	AFinalBattlePresentationActor* TargetActor = PresentationActorsByRuntimeId.FindRef(BattleEvent.TargetUnitId);

	switch (BattleEvent.EventType)
	{
	case EFinalBattleEventType::CardResolved:
	case EFinalBattleEventType::UltimateResolved:
	case EFinalBattleEventType::EnemyActed:
		if (SourceActor)
		{
			SourceActor->PlayAttackPresentation();
		}

		if (TargetActor && BattleEvent.TargetUnitId != BattleEvent.SourceUnitId)
		{
			TargetActor->PlayHitPresentation();
		}
		break;

	default:
		break;
	}
}

void AFinalBattleDirector::ClearPresentationActors()
{
	TArray<TObjectPtr<AFinalBattlePresentationActor>> PresentationActors;
	PresentationActorsByRuntimeId.GenerateValueArray(PresentationActors);
	for (AFinalBattlePresentationActor* Actor : PresentationActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	PresentationActorsByRuntimeId.Reset();
	PresentationUnitsByRuntimeId.Reset();
}

AFinalBattlePresentationActor* AFinalBattleDirector::GetOrSpawnPresentationActor(
	const FName RuntimeUnitId,
	const FFinalBattlePresentationUnitViewData& UnitView)
{
	const TSubclassOf<AFinalBattlePresentationActor> DesiredPresentationClass = ResolvePresentationActorClass(UnitView);

	if (AFinalBattlePresentationActor* ExistingActor = PresentationActorsByRuntimeId.FindRef(RuntimeUnitId))
	{
		if (DesiredPresentationClass == nullptr || ExistingActor->IsA(DesiredPresentationClass))
		{
			return ExistingActor;
		}

		ExistingActor->Destroy();
		PresentationActorsByRuntimeId.Remove(RuntimeUnitId);
	}

	UWorld* World = GetWorld();
	if (World == nullptr || DesiredPresentationClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFinalBattlePresentationActor* SpawnedActor = World->SpawnActor<AFinalBattlePresentationActor>(
		DesiredPresentationClass,
		ResolvePresentationTransform(UnitView),
		SpawnParameters);
	if (SpawnedActor)
	{
		SpawnedActor->InitializePresentationActor(
			RuntimeUnitId,
			UnitView.Team);
		PresentationActorsByRuntimeId.Add(RuntimeUnitId, SpawnedActor);
	}

	return SpawnedActor;
}

FTransform AFinalBattleDirector::ResolvePresentationTransform(const FFinalBattlePresentationUnitViewData& UnitView) const
{
	if (const AFinalBattleStageAnchorActor* StageAnchor = ResolveStageAnchor(UnitView))
	{
		return StageAnchor->GetActorTransform();
	}

	const bool bIsEnemy = UnitView.Team == EFinalBattlePresentationTeam::Enemy;
	const FVector BaseOffset = bIsEnemy
		? EnemyPresentationOrigin
		: PlayerPresentationOrigin;
	const float SlotSpacing = bIsEnemy ? EnemySlotSpacing : PlayerSlotSpacing;
	const FVector LocalOffset = BaseOffset + FVector(0.0f, SlotSpacing * UnitView.SlotIndex, 0.0f);
	return FTransform(GetActorRotation() + PresentationRotation, GetActorTransform().TransformPosition(LocalOffset));
}

AFinalBattleStageAnchorActor* AFinalBattleDirector::ResolveStageAnchor(const FFinalBattlePresentationUnitViewData& UnitView) const
{
	const TMap<int32, TObjectPtr<AFinalBattleStageAnchorActor>>& SourceMap =
		UnitView.Team == EFinalBattlePresentationTeam::Enemy ? EnemyStageAnchorsByIndex : PlayerStageAnchorsByIndex;

	if (const TObjectPtr<AFinalBattleStageAnchorActor>* AnchorPtr = SourceMap.Find(UnitView.SlotIndex))
	{
		return AnchorPtr->Get();
	}

	return nullptr;
}

TSubclassOf<AFinalBattlePresentationActor> AFinalBattleDirector::ResolvePresentationActorClass(
	const FFinalBattlePresentationUnitViewData& UnitView) const
{
	const TArray<FFinalBattlePresentationClassMapping>& SourceMappings =
		UnitView.Team == EFinalBattlePresentationTeam::Enemy ? EnemyPresentationClassMappings : PlayerPresentationClassMappings;

	if (!UnitView.UnitDefinitionId.IsNone())
	{
		if (const FFinalBattlePresentationClassMapping* Mapping = SourceMappings.FindByPredicate(
				[&UnitView](const FFinalBattlePresentationClassMapping& Candidate)
				{
					return Candidate.UnitDefinitionId == UnitView.UnitDefinitionId && Candidate.PresentationClass != nullptr;
				}))
		{
			return Mapping->PresentationClass;
		}
	}

	if (UnitView.Team == EFinalBattlePresentationTeam::Enemy && DefaultEnemyPresentationClass != nullptr)
	{
		return DefaultEnemyPresentationClass;
	}

	if (UnitView.Team == EFinalBattlePresentationTeam::Player && DefaultPlayerPresentationClass != nullptr)
	{
		return DefaultPlayerPresentationClass;
	}

	return PresentationActorClass;
}
