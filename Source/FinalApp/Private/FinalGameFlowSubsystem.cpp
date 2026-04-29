#include "Subsystems/FinalGameFlowSubsystem.h"

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalCharacterGrowthConfig.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Templates/UnrealTemplate.h"

void UFinalGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr)
	{
		BindToBattleFlowSubsystem(BattleFlowSubsystem);
	}
}

void UFinalGameFlowSubsystem::Deinitialize()
{
	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr)
	{
		UnbindFromBattleFlowSubsystem(BattleFlowSubsystem);
	}

	Super::Deinitialize();
}

UFinalRunSession* UFinalGameFlowSubsystem::BootstrapNewRun()
{
	LastFlowFailureReason = FText::GetEmpty();

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr)
	{
		BattleFlowSubsystem->ClearActiveBattleSession();
	}

	RunSession = NewObject<UFinalRunSession>(this);
	RunSession->InitializeRun();
	LastProcessedGrowthFactBatchSequence = 0;
	bPendingGrowthChoiceDeferredFromEnemyPhase = false;

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
	{
		RunFlowSubsystem->HandleRunSessionChanged();
	}

	return RunSession;
}

UFinalBattleSession* UFinalGameFlowSubsystem::StartBattleFromRunSession()
{
	LastFlowFailureReason = FText::GetEmpty();

	if (RunSession == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("RunSession is unavailable."));
		return nullptr;
	}

	if (RunSession->HasPendingGrowthChoice())
	{
		LastFlowFailureReason = FText::FromString(TEXT("RunSession still has a pending growth choice to resolve before starting battle."));
		return nullptr;
	}

	if (!RunSession->HasValidBattleStartState())
	{
		LastFlowFailureReason = FText::FromString(TEXT("RunSession does not have a valid battle start state."));
		return nullptr;
	}

	UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (BattleFlowSubsystem == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("FinalBattleFlowSubsystem is unavailable."));
		return nullptr;
	}

	UFinalBattleSession* BattleSession = BattleFlowSubsystem->CreateBattleSessionFromStartRequest(RunSession->BuildBattleStartRequest());
	if (BattleSession == nullptr)
	{
		LastFlowFailureReason = BattleFlowSubsystem->GetLastFailureReason();
	}
	else
	{
		LastProcessedGrowthFactBatchSequence = 0;
		bPendingGrowthChoiceDeferredFromEnemyPhase = false;
		if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
		{
			RunFlowSubsystem->RefreshRunFlow(true);
		}
	}

	return BattleSession;
}

bool UFinalGameFlowSubsystem::TryAutoStartPreparedBattleFromRun()
{
	if (bAutoStartingPreparedBattle || RunSession == nullptr)
	{
		return false;
	}

	UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (BattleFlowSubsystem == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("FinalBattleFlowSubsystem is unavailable."));
		return false;
	}

	if (BattleFlowSubsystem->GetActiveBattleSession() != nullptr)
	{
		return false;
	}

	const FFinalRunSnapshot Snapshot = RunSession->GetSnapshot();
	if (Snapshot.Progression.FlowStage != EFinalRunFlowStage::PreparingBattle
		|| Snapshot.PendingGrowthChoice.bHasPendingChoice)
	{
		return false;
	}

	if (!RunSession->HasValidBattleStartState())
	{
		LastFlowFailureReason = FText::FromString(TEXT("RunSession entered PreparingBattle without a valid battle start state."));
		return false;
	}

	TGuardValue<bool> AutoStartGuard(bAutoStartingPreparedBattle, true);
	return StartBattleFromRunSession() != nullptr;
}

bool UFinalGameFlowSubsystem::CompleteBattleAndApplyResult(const FFinalBattleResult& Result)
{
	LastFlowFailureReason = FText::GetEmpty();

	if (RunSession == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("RunSession is unavailable."));
		return false;
	}

	UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (BattleFlowSubsystem == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("FinalBattleFlowSubsystem is unavailable."));
		return false;
	}

	if (BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("There is no active battle session to complete."));
		return false;
	}

	RunSession->ApplyBattleResult(Result);
	BattleFlowSubsystem->ClearActiveBattleSession();
	LastProcessedGrowthFactBatchSequence = 0;
	bPendingGrowthChoiceDeferredFromEnemyPhase = false;

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}

	return true;
}

bool UFinalGameFlowSubsystem::CompleteResolvedBattle()
{
	LastFlowFailureReason = FText::GetEmpty();

	FFinalBattleResult Result;
	if (!BuildResolvedBattleResult(Result))
	{
		return false;
	}

	return CompleteBattleAndApplyResult(Result);
}

UFinalRunSession* UFinalGameFlowSubsystem::GetRunSession() const
{
	return RunSession;
}

UFinalBattleSession* UFinalGameFlowSubsystem::GetActiveBattleSession() const
{
	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	return BattleFlowSubsystem ? BattleFlowSubsystem->GetActiveBattleSession() : nullptr;
}

FFinalBattleSnapshot UFinalGameFlowSubsystem::GetCurrentBattleSnapshot() const
{
	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	return BattleFlowSubsystem ? BattleFlowSubsystem->GetCurrentSnapshot() : FFinalBattleSnapshot{};
}

bool UFinalGameFlowSubsystem::RestoreRunSessionFromSaveData(const FFinalRunSaveData& SaveData, FText& OutFailureReason)
{
	LastFlowFailureReason = FText::GetEmpty();
	OutFailureReason = FText::GetEmpty();

	if (GetActiveBattleSession() != nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("Cannot restore RunSession while a battle session is active."));
		OutFailureReason = LastFlowFailureReason;
		return false;
	}

	UFinalRunSession* RestoredRunSession = NewObject<UFinalRunSession>(this);
	if (RestoredRunSession == nullptr || !RestoredRunSession->RestoreFromSaveData(SaveData, OutFailureReason))
	{
		LastFlowFailureReason = OutFailureReason.IsEmpty()
			? FText::FromString(TEXT("Failed to restore RunSession from save data."))
			: OutFailureReason;
		return false;
	}

	RunSession = RestoredRunSession;

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
	{
		RunFlowSubsystem->HandleRunSessionChanged();
	}

	return true;
}

FText UFinalGameFlowSubsystem::GetLastBattleFailureReason() const
{
	if (!LastFlowFailureReason.IsEmpty())
	{
		return LastFlowFailureReason;
	}

	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	return BattleFlowSubsystem ? BattleFlowSubsystem->GetLastFailureReason() : FText::GetEmpty();
}

bool UFinalGameFlowSubsystem::BuildResolvedBattleResult(FFinalBattleResult& OutResult)
{
	if (RunSession == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("RunSession is unavailable."));
		return false;
	}

	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (BattleFlowSubsystem == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("FinalBattleFlowSubsystem is unavailable."));
		return false;
	}

	if (BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastFlowFailureReason = FText::FromString(TEXT("There is no active battle session to resolve."));
		return false;
	}

	const FFinalBattleSnapshot Snapshot = BattleFlowSubsystem->GetCurrentSnapshot();
	if (!Snapshot.bBattleEnded)
	{
		LastFlowFailureReason = FText::FromString(TEXT("Battle is not resolved yet."));
		return false;
	}

	const FFinalRunState RunState = RunSession->GetRunState();

	OutResult = FFinalBattleResult{};
	OutResult.EncounterId = RunState.CurrentEncounterId;
	OutResult.Outcome = Snapshot.bPlayerVictory ? EFinalBattleOutcome::Victory : EFinalBattleOutcome::Defeat;
	OutResult.TeamCurrentHP = Snapshot.TeamCurrentHP;
	OutResult.RewardGold = Snapshot.bPlayerVictory ? 15 : 0;
	OutResult.UpdatedCharacterStates.Reset();
	OutResult.UpdatedCharacterStates.Reserve(Snapshot.Characters.Num());
	for (const FFinalBattleCharacterViewData& CharacterView : Snapshot.Characters)
	{
		FFinalRunPersistentCharacterState UpdatedCharacterState;
		UpdatedCharacterState.CharacterId = CharacterView.CharacterId;
		UpdatedCharacterState.CurrentStress = CharacterView.CurrentStress;
		UpdatedCharacterState.bCollapsed = CharacterView.bCollapsed;
		UpdatedCharacterState.CurrentAwakenCount = CharacterView.CurrentAwakenCount;
		UpdatedCharacterState.CollapseCount = CharacterView.CollapseCount;
		OutResult.UpdatedCharacterStates.Add(UpdatedCharacterState);
	}
	return true;
}

void UFinalGameFlowSubsystem::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	ProcessPendingBattleGrowthFacts(Snapshot);
}

void UFinalGameFlowSubsystem::ProcessPendingBattleGrowthFacts(const FFinalBattleSnapshot& Snapshot)
{
	UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (BattleFlowSubsystem == nullptr || RunSession == nullptr)
	{
		return;
	}

	UFinalBattleSession* BattleSession = BattleFlowSubsystem->GetActiveBattleSession();
	if (BattleSession == nullptr)
	{
		LastProcessedGrowthFactBatchSequence = 0;
		bPendingGrowthChoiceDeferredFromEnemyPhase = false;
		return;
	}

	const bool bHadPendingGrowthChoice = RunSession->HasPendingGrowthChoice();
	const TArray<FFinalBattleGrowthFactBatch> GrowthFactBatches = BattleSession->GetGrowthFactBatchesSince(LastProcessedGrowthFactBatchSequence);
	if (GrowthFactBatches.IsEmpty())
	{
		if (TryPresentPendingGrowthChoiceAtSafeWindow(Snapshot, false))
		{
			BattleFlowSubsystem->OnBattleSnapshotChanged.Broadcast(Snapshot);
		}
		return;
	}

	const UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	const FFinalRunState RunState = RunSession->GetRunState();
	const EFinalRunNodeType CurrentNodeType = RunSession->GetSnapshot().Progression.CurrentNodeType;
	bool bAppliedAnyGrowth = false;
	bool bPlayerCommandGrowthBatchProcessed = false;

	for (const FFinalBattleGrowthFactBatch& Batch : GrowthFactBatches)
	{
		LastProcessedGrowthFactBatchSequence = FMath::Max(LastProcessedGrowthFactBatchSequence, Batch.BatchSequence);
		if (DataRegistry == nullptr)
		{
			continue;
		}

		for (const FFinalBattleGrowthFact& Fact : Batch.Facts)
		{
			if (!Fact.CharacterId.IsValid())
			{
				continue;
			}

			const FFinalRunPersistentCharacterState* CharacterState = RunState.Characters.FindByPredicate([&Fact](const FFinalRunPersistentCharacterState& Candidate)
			{
				return Candidate.CharacterId == Fact.CharacterId;
			});
			if (CharacterState == nullptr)
			{
				continue;
			}

			int32 GainAmount = ResolveBreakthroughGainFromFact(Fact, *CharacterState);
			if (Fact.FactType == EFinalBattleGrowthFactType::BattleVictoryBaseReward)
			{
				const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(Fact.CharacterId);
				const UFinalCharacterGrowthConfig* GrowthConfig =
					(CharacterDefinition != nullptr && CharacterDefinition->GrowthConfigId.IsValid())
						? DataRegistry->FindCharacterGrowthConfig(CharacterDefinition->GrowthConfigId)
						: nullptr;
				if (GrowthConfig != nullptr && GrowthConfig->PreferredBreakthroughFactTypes.Contains(Fact.FactType))
				{
					GainAmount = ResolveBattleVictoryRewardForNode(CurrentNodeType, *GrowthConfig);
					const float InsightMultiplier = 1.0f + static_cast<float>(CharacterState->Insight) * GrowthConfig->InsightBreakthroughGainMultiplierPerPoint;
					GainAmount = FMath::Max(0, FMath::RoundToInt(static_cast<float>(GainAmount) * InsightMultiplier));
				}
				else
				{
					GainAmount = 0;
				}
			}

			if (GainAmount > 0 && RunSession->AddBreakthroughValue(Fact.CharacterId, GainAmount))
			{
				bAppliedAnyGrowth = true;
				bPlayerCommandGrowthBatchProcessed |= Batch.bCausedByPlayerCommand;
			}
		}
	}

	const bool bPendingCreatedThisTick = !bHadPendingGrowthChoice && RunSession->HasPendingGrowthChoice();
	if (bPendingCreatedThisTick && !bPlayerCommandGrowthBatchProcessed)
	{
		bPendingGrowthChoiceDeferredFromEnemyPhase = true;
	}

	const bool bPresentedGrowth = TryPresentPendingGrowthChoiceAtSafeWindow(Snapshot, bPendingCreatedThisTick);
	if (bAppliedAnyGrowth || bPresentedGrowth)
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.Broadcast(Snapshot);
	}
}

bool UFinalGameFlowSubsystem::TryPresentPendingGrowthChoiceAtSafeWindow(const FFinalBattleSnapshot& Snapshot, const bool bPendingCreatedThisTick)
{
	if (RunSession == nullptr || !RunSession->HasPendingGrowthChoice())
	{
		bPendingGrowthChoiceDeferredFromEnemyPhase = false;
		return false;
	}

	const bool bBattleReadyForPlayerInput = !Snapshot.bBattleEnded;
	const bool bShouldPresentImmediately = bPendingCreatedThisTick && !bPendingGrowthChoiceDeferredFromEnemyPhase;
	const bool bShouldPresentDeferred = bPendingGrowthChoiceDeferredFromEnemyPhase && bBattleReadyForPlayerInput;
	const bool bShouldPresentPostBattle = Snapshot.bBattleEnded;
	if (!bShouldPresentImmediately && !bShouldPresentDeferred && !bShouldPresentPostBattle)
	{
		return false;
	}

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}

	if (bShouldPresentDeferred || bShouldPresentPostBattle)
	{
		bPendingGrowthChoiceDeferredFromEnemyPhase = false;
	}

	return true;
}

int32 UFinalGameFlowSubsystem::ResolveBreakthroughGainFromFact(const FFinalBattleGrowthFact& Fact, const FFinalRunPersistentCharacterState& CharacterState) const
{
	const UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	if (DataRegistry == nullptr)
	{
		return 0;
	}

	const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(Fact.CharacterId);
	const UFinalCharacterGrowthConfig* GrowthConfig =
		(CharacterDefinition != nullptr && CharacterDefinition->GrowthConfigId.IsValid())
			? DataRegistry->FindCharacterGrowthConfig(CharacterDefinition->GrowthConfigId)
			: nullptr;
	if (GrowthConfig == nullptr)
	{
		return 0;
	}

	if (!GrowthConfig->PreferredBreakthroughFactTypes.Contains(Fact.FactType))
	{
		return 0;
	}

	const float* Scalar = GrowthConfig->BreakthroughGainScalarByFactType.Find(Fact.FactType);
	if (Scalar == nullptr || *Scalar <= 0.0f)
	{
		return 0;
	}

	const int32 BaseGain = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Fact.Magnitude) * *Scalar));
	if (BaseGain <= 0)
	{
		return 0;
	}

	const float InsightMultiplier = 1.0f + static_cast<float>(CharacterState.Insight) * GrowthConfig->InsightBreakthroughGainMultiplierPerPoint;
	return FMath::Max(0, FMath::RoundToInt(static_cast<float>(BaseGain) * InsightMultiplier));
}

int32 UFinalGameFlowSubsystem::ResolveBattleVictoryRewardForNode(const EFinalRunNodeType NodeType, const UFinalCharacterGrowthConfig& GrowthConfig) const
{
	switch (NodeType)
	{
	case EFinalRunNodeType::EliteBattle:
		return GrowthConfig.EliteBattleVictoryBreakthroughReward;
	case EFinalRunNodeType::BossBattle:
		return GrowthConfig.BossBattleVictoryBreakthroughReward;
	case EFinalRunNodeType::Battle:
	default:
		return GrowthConfig.NormalBattleVictoryBreakthroughReward;
	}
}

void UFinalGameFlowSubsystem::BindToBattleFlowSubsystem(UFinalBattleFlowSubsystem* BattleFlowSubsystem)
{
	if (BattleFlowSubsystem != nullptr)
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &UFinalGameFlowSubsystem::HandleBattleSnapshotChanged);
	}
}

void UFinalGameFlowSubsystem::UnbindFromBattleFlowSubsystem(UFinalBattleFlowSubsystem* BattleFlowSubsystem)
{
	if (BattleFlowSubsystem != nullptr)
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &UFinalGameFlowSubsystem::HandleBattleSnapshotChanged);
	}
}
