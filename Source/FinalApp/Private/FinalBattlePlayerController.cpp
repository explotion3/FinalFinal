#include "World/FinalBattlePlayerController.h"

#include "App/FinalGameInstance.h"
#include "Engine/GameInstance.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalBattlePlayerController, Log, All);

namespace
{
const TCHAR* LexToString(const EFinalBattleEventType EventType)
{
	switch (EventType)
	{
	case EFinalBattleEventType::Info:
		return TEXT("Info");

	case EFinalBattleEventType::CommandAccepted:
		return TEXT("CommandAccepted");

	case EFinalBattleEventType::CommandRejected:
		return TEXT("CommandRejected");

	case EFinalBattleEventType::StateChanged:
		return TEXT("StateChanged");

	case EFinalBattleEventType::PhaseChanged:
		return TEXT("PhaseChanged");

	default:
		return TEXT("Unknown");
	}
}
}

bool AFinalBattlePlayerController::StartTestBattle()
{
	if (UFinalGameInstance* FinalGameInstance = Cast<UFinalGameInstance>(GetGameInstance()))
	{
		return FinalGameInstance->StartTestBattle();
	}

	return false;
}

FText AFinalBattlePlayerController::GetLastTestBattleFailureReason() const
{
	if (const UFinalGameInstance* FinalGameInstance = Cast<UFinalGameInstance>(GetGameInstance()))
	{
		return FinalGameInstance->GetLastTestFailureReason();
	}

	return FText::FromString(TEXT("FinalGameInstance is unavailable."));
}

bool AFinalBattlePlayerController::DumpBattleSnapshotToLog() const
{
	const UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("FinalGameFlowSubsystem is unavailable."));
		return false;
	}

	const FFinalBattleSnapshot Snapshot = GameFlowSubsystem->GetCurrentBattleSnapshot();
	if (GameFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("There is no active battle session."));
		return false;
	}

	UE_LOG(
		LogFinalBattlePlayerController,
		Log,
		TEXT("Battle Snapshot | Round=%d AP=%d EP=%d TeamHP=%d/%d Shield=%d Ended=%s Victory=%s Characters=%d Enemies=%d Hand=%d"),
		Snapshot.CurrentRound,
		Snapshot.CurrentAP,
		Snapshot.CurrentEP,
		Snapshot.TeamCurrentHP,
		Snapshot.TeamMaxHP,
		Snapshot.TeamShield,
		Snapshot.bBattleEnded ? TEXT("true") : TEXT("false"),
		Snapshot.bPlayerVictory ? TEXT("true") : TEXT("false"),
		Snapshot.Characters.Num(),
		Snapshot.Enemies.Num(),
		Snapshot.HandCards.Num());

	for (const FFinalBattleEnemyViewData& EnemyView : Snapshot.Enemies)
	{
		UE_LOG(
			LogFinalBattlePlayerController,
			Log,
			TEXT("Enemy | Unit=%s HP=%d Shield=%d Break=%d Initiative=%d Phase=%s Intent=%s"),
			*EnemyView.RuntimeUnitId.ToString(),
			EnemyView.CurrentHP,
			EnemyView.CurrentShield,
			EnemyView.CurrentBreakValue,
			EnemyView.CurrentInitiative,
			EnemyView.CurrentPhaseTag == NAME_None ? TEXT("None") : *EnemyView.CurrentPhaseTag.ToString(),
			*EnemyView.IntentText.ToString());
	}

	for (const FFinalBattleCardViewData& CardView : Snapshot.HandCards)
	{
		UE_LOG(
			LogFinalBattlePlayerController,
			Log,
			TEXT("Hand Card | Instance=%s CardId=%s Cost=%d"),
			*CardView.CardInstanceId.ToString(EGuidFormats::DigitsWithHyphens),
			*CardView.CardId.ToString(),
			CardView.RuntimeCostAP);
	}

	return true;
}

bool AFinalBattlePlayerController::DumpBattleLogToLog() const
{
	const UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr || BattleFlowSubsystem == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("Required battle subsystems are unavailable."));
		return false;
	}

	if (GameFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("There is no active battle session."));
		return false;
	}

	const TArray<FFinalBattleEvent> BattleLogEntries = BattleFlowSubsystem->GetBattleLogEntries();
	UE_LOG(LogFinalBattlePlayerController, Log, TEXT("Battle Log | Entries=%d"), BattleLogEntries.Num());

	for (const FFinalBattleEvent& BattleEvent : BattleLogEntries)
	{
		UE_LOG(
			LogFinalBattlePlayerController,
			Log,
			TEXT("Battle Log Entry | Type=%s Round=%d Message=%s"),
			LexToString(BattleEvent.EventType),
			BattleEvent.Round,
			*BattleEvent.Message.ToString());
	}

	return true;
}

bool AFinalBattlePlayerController::PlayFirstHandCard()
{
	UGameInstance* GameInstance = GetGameInstance();
	UFinalGameFlowSubsystem* GameFlowSubsystem = GameInstance ? GameInstance->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	UFinalBattleFlowSubsystem* BattleFlowSubsystem = GameInstance ? GameInstance->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr || BattleFlowSubsystem == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("Required battle subsystems are unavailable."));
		return false;
	}

	const FFinalBattleSnapshot Snapshot = GameFlowSubsystem->GetCurrentBattleSnapshot();
	if (Snapshot.HandCards.Num() == 0)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("No cards are currently in hand."));
		return false;
	}

	const FFinalBattleEnemyViewData* TargetEnemy = Snapshot.Enemies.FindByPredicate(
		[](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.CurrentHP > 0;
		});

	if (TargetEnemy == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("No alive enemy is available as a target."));
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = Snapshot.HandCards[0].CardInstanceId;
	Command.TargetUnitId = TargetEnemy->RuntimeUnitId;

	const bool bAccepted = BattleFlowSubsystem->SubmitBattleCommand(Command);
	const FFinalBattleEvent Event = BattleFlowSubsystem->GetLastCommandEvent();
	if (bAccepted)
	{
		UE_LOG(
			LogFinalBattlePlayerController,
			Log,
			TEXT("PlayFirstHandCard | Accepted=%s Round=%d Message=%s"),
			TEXT("true"),
			Event.Round,
			*Event.Message.ToString());
	}
	else
	{
		UE_LOG(
			LogFinalBattlePlayerController,
			Warning,
			TEXT("PlayFirstHandCard | Accepted=%s Round=%d Message=%s"),
			TEXT("false"),
			Event.Round,
			*Event.Message.ToString());
	}

	return bAccepted;
}

bool AFinalBattlePlayerController::CompleteResolvedBattle()
{
	UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr)
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("FinalGameFlowSubsystem is unavailable."));
		return false;
	}

	const bool bCompleted = GameFlowSubsystem->CompleteResolvedBattle();
	const FText Feedback = bCompleted
		? FText::FromString(TEXT("Resolved battle was applied back to RunSession."))
		: GameFlowSubsystem->GetLastBattleFailureReason();

	if (bCompleted)
	{
		UE_LOG(LogFinalBattlePlayerController, Log, TEXT("CompleteResolvedBattle | %s"), *Feedback.ToString());
	}
	else
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("CompleteResolvedBattle | %s"), *Feedback.ToString());
	}

	return bCompleted;
}

bool AFinalBattlePlayerController::SubmitBattleCommand(const FFinalBattleCommand& Command)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalBattleFlowSubsystem* BattleFlow = GameInstance->GetSubsystem<UFinalBattleFlowSubsystem>())
		{
			return BattleFlow->SubmitBattleCommand(Command);
		}
	}

	return false;
}

bool AFinalBattlePlayerController::EndPlayerTurn()
{
	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::EndTurn;
	return SubmitBattleCommand(Command);
}

void AFinalBattlePlayerController::FinalStartTestBattle()
{
	const bool bStarted = StartTestBattle();
	const FText Feedback = bStarted
		? FText::FromString(TEXT("Test battle started."))
		: GetLastTestBattleFailureReason();

	if (bStarted)
	{
		UE_LOG(LogFinalBattlePlayerController, Log, TEXT("%s"), *Feedback.ToString());
	}
	else
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("%s"), *Feedback.ToString());
	}
}

void AFinalBattlePlayerController::FinalDumpBattleSnapshot()
{
	DumpBattleSnapshotToLog();
}

void AFinalBattlePlayerController::FinalDumpBattleLog()
{
	DumpBattleLogToLog();
}

void AFinalBattlePlayerController::FinalPlayFirstHandCard()
{
	PlayFirstHandCard();
}

void AFinalBattlePlayerController::FinalEndTurnCommand()
{
	const bool bAccepted = EndPlayerTurn();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = GameInstance->GetSubsystem<UFinalBattleFlowSubsystem>())
		{
			const FFinalBattleEvent Event = BattleFlowSubsystem->GetLastCommandEvent();
			if (bAccepted)
			{
				UE_LOG(
					LogFinalBattlePlayerController,
					Log,
					TEXT("EndTurn | Accepted=%s Round=%d Message=%s"),
					TEXT("true"),
					Event.Round,
					*Event.Message.ToString());
			}
			else
			{
				UE_LOG(
					LogFinalBattlePlayerController,
					Warning,
					TEXT("EndTurn | Accepted=%s Round=%d Message=%s"),
					TEXT("false"),
					Event.Round,
					*Event.Message.ToString());
			}
			return;
		}
	}

	UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("Failed to read last battle command event after EndTurn."));
}

void AFinalBattlePlayerController::FinalCompleteResolvedBattle()
{
	CompleteResolvedBattle();
}
