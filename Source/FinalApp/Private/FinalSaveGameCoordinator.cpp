#include "Save/FinalSaveGameCoordinator.h"

#include "Facade/FinalRunSession.h"
#include "Kismet/GameplayStatics.h"
#include "Save/FinalRunSaveGame.h"
#include "Subsystems/FinalGameFlowSubsystem.h"

namespace
{
const FString PrototypeRunSlotName(TEXT("FinalPrototypeRun"));
constexpr int32 PrototypeRunUserIndex = 0;
}

const FString& UFinalSaveGameCoordinator::GetPrototypeRunSlotName()
{
	return PrototypeRunSlotName;
}

bool UFinalSaveGameCoordinator::SaveCurrentRunToPrototypeSlot()
{
	LastFailureReason = FText::GetEmpty();

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr)
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "SaveOperation", "Save"), FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable.")));
		return false;
	}

	if (HasActiveBattleSession())
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "SaveOperation", "Save"), FText::FromString(TEXT("Cannot save while a battle session is active.")));
		return false;
	}

	UFinalRunSession* RunSession = GameFlowSubsystem->GetRunSession();
	if (RunSession == nullptr)
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "SaveOperation", "Save"), FText::FromString(TEXT("RunSession is unavailable.")));
		return false;
	}

	UFinalRunSaveGame* SaveGame = Cast<UFinalRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UFinalRunSaveGame::StaticClass()));
	if (SaveGame == nullptr)
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "SaveOperation", "Save"), FText::FromString(TEXT("Failed to create FinalRunSaveGame.")));
		return false;
	}

	SaveGame->RunSaveData = RunSession->ExportSaveData();
	FText ValidationFailureReason;
	if (!SaveGame->RunSaveData.IsStructurallyValid(&ValidationFailureReason))
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "SaveOperation", "Save"), ValidationFailureReason);
		return false;
	}

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, PrototypeRunSlotName, PrototypeRunUserIndex))
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "SaveOperation", "Save"), FText::FromString(TEXT("Failed to write prototype run save slot.")));
		return false;
	}

	SetSuccessStatus(FText::Format(
		NSLOCTEXT("FinalSaveGameCoordinator", "SaveSucceeded", "Saved prototype run slot {0} with save version {1}."),
		FText::FromString(PrototypeRunSlotName),
		FText::AsNumber(SaveGame->RunSaveData.SaveVersion)));
	return true;
}

bool UFinalSaveGameCoordinator::LoadRunFromPrototypeSlot()
{
	LastFailureReason = FText::GetEmpty();

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr)
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable.")));
		return false;
	}

	if (HasActiveBattleSession())
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), FText::FromString(TEXT("Cannot load while a battle session is active.")));
		return false;
	}

	if (!DoesPrototypeRunSaveExist())
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), FText::FromString(TEXT("Prototype run save slot does not exist.")));
		return false;
	}

	USaveGame* LoadedSaveGame = UGameplayStatics::LoadGameFromSlot(PrototypeRunSlotName, PrototypeRunUserIndex);
	if (LoadedSaveGame == nullptr)
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), FText::FromString(TEXT("Failed to read prototype run save slot.")));
		return false;
	}

	UFinalRunSaveGame* SaveGame = Cast<UFinalRunSaveGame>(LoadedSaveGame);
	if (SaveGame == nullptr)
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), FText::FromString(TEXT("Prototype run save slot contains an unexpected SaveGame type.")));
		return false;
	}

	if (!SaveGame->RunSaveData.IsSupportedVersion())
	{
		SetFailureStatus(
			NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"),
			FText::Format(
				NSLOCTEXT("FinalSaveGameCoordinator", "UnsupportedLoadedVersion", "Unsupported run save version {0}; expected {1}."),
				FText::AsNumber(SaveGame->RunSaveData.SaveVersion),
				FText::AsNumber(FFinalRunSaveData::CurrentSaveVersion)));
		return false;
	}

	FText ValidationFailureReason;
	if (!SaveGame->RunSaveData.IsStructurallyValid(&ValidationFailureReason))
	{
		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), ValidationFailureReason);
		return false;
	}

	FText RestoreFailureReason;
	if (!GameFlowSubsystem->RestoreRunSessionFromSaveData(SaveGame->RunSaveData, RestoreFailureReason))
	{
		FText FlowFailureReason = !RestoreFailureReason.IsEmpty()
			? RestoreFailureReason
			: GameFlowSubsystem->GetLastBattleFailureReason();
		if (FlowFailureReason.IsEmpty())
		{
			FlowFailureReason = FText::FromString(TEXT("Failed to restore RunSession from save data."));
		}

		SetFailureStatus(NSLOCTEXT("FinalSaveGameCoordinator", "LoadOperation", "Load"), FlowFailureReason);
		return false;
	}

	SetSuccessStatus(FText::Format(
		NSLOCTEXT("FinalSaveGameCoordinator", "LoadSucceeded", "Loaded prototype run slot {0} with save version {1}."),
		FText::FromString(PrototypeRunSlotName),
		FText::AsNumber(SaveGame->RunSaveData.SaveVersion)));
	return true;
}

bool UFinalSaveGameCoordinator::DoesPrototypeRunSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(PrototypeRunSlotName, PrototypeRunUserIndex);
}

FText UFinalSaveGameCoordinator::GetLastFailureReason() const
{
	return LastFailureReason;
}

FText UFinalSaveGameCoordinator::GetLastSaveLoadStatusText() const
{
	return LastSaveLoadStatusText;
}

FText UFinalSaveGameCoordinator::GetPrototypeRunSaveDebugText() const
{
	const FText SlotExistsText = DoesPrototypeRunSaveExist()
		? NSLOCTEXT("FinalSaveGameCoordinator", "SlotExistsYes", "Yes")
		: NSLOCTEXT("FinalSaveGameCoordinator", "SlotExistsNo", "No");
	const FText StatusText = LastSaveLoadStatusText.IsEmpty()
		? NSLOCTEXT("FinalSaveGameCoordinator", "NoSaveLoadStatusYet", "No save/load operation has been attempted.")
		: LastSaveLoadStatusText;
	const FText FailureText = LastFailureReason.IsEmpty()
		? NSLOCTEXT("FinalSaveGameCoordinator", "NoLastFailure", "None")
		: LastFailureReason;

	return FText::Format(
		NSLOCTEXT("FinalSaveGameCoordinator", "PrototypeRunSaveDebugText", "Save Slot: {0}\nSlot Exists: {1}\nLast Status: {2}\nLast Failure: {3}"),
		FText::FromString(PrototypeRunSlotName),
		SlotExistsText,
		StatusText,
		FailureText);
}

void UFinalSaveGameCoordinator::SetFailureReason(const FText& Reason)
{
	LastFailureReason = Reason;
}

void UFinalSaveGameCoordinator::SetSuccessStatus(const FText& StatusText)
{
	LastFailureReason = FText::GetEmpty();
	LastSaveLoadStatusText = StatusText;
}

void UFinalSaveGameCoordinator::SetFailureStatus(const FText& OperationText, const FText& Reason)
{
	SetFailureReason(Reason);
	LastSaveLoadStatusText = FText::Format(
		NSLOCTEXT("FinalSaveGameCoordinator", "OperationFailedStatus", "{0} failed: {1}"),
		OperationText,
		Reason);
}

bool UFinalSaveGameCoordinator::HasActiveBattleSession() const
{
	const UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	return GameFlowSubsystem != nullptr && GameFlowSubsystem->GetActiveBattleSession() != nullptr;
}
