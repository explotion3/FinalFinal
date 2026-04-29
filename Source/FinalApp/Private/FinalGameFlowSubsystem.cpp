#include "Subsystems/FinalGameFlowSubsystem.h"

#include "Facade/FinalRunSession.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Templates/UnrealTemplate.h"

void UFinalGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
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
	else if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
	{
		RunFlowSubsystem->RefreshRunFlow(true);
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
