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
		SetFailureReason(FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable.")));
		return false;
	}

	if (HasActiveBattleSession())
	{
		SetFailureReason(FText::FromString(TEXT("Cannot save while a battle session is active.")));
		return false;
	}

	UFinalRunSession* RunSession = GameFlowSubsystem->GetRunSession();
	if (RunSession == nullptr)
	{
		SetFailureReason(FText::FromString(TEXT("RunSession is unavailable.")));
		return false;
	}

	UFinalRunSaveGame* SaveGame = Cast<UFinalRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UFinalRunSaveGame::StaticClass()));
	if (SaveGame == nullptr)
	{
		SetFailureReason(FText::FromString(TEXT("Failed to create FinalRunSaveGame.")));
		return false;
	}

	SaveGame->RunSaveData = RunSession->ExportSaveData();
	if (!UGameplayStatics::SaveGameToSlot(SaveGame, PrototypeRunSlotName, PrototypeRunUserIndex))
	{
		SetFailureReason(FText::FromString(TEXT("Failed to write prototype run save slot.")));
		return false;
	}

	return true;
}

bool UFinalSaveGameCoordinator::LoadRunFromPrototypeSlot()
{
	LastFailureReason = FText::GetEmpty();

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	if (GameFlowSubsystem == nullptr)
	{
		SetFailureReason(FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable.")));
		return false;
	}

	if (HasActiveBattleSession())
	{
		SetFailureReason(FText::FromString(TEXT("Cannot load while a battle session is active.")));
		return false;
	}

	if (!DoesPrototypeRunSaveExist())
	{
		SetFailureReason(FText::FromString(TEXT("Prototype run save slot does not exist.")));
		return false;
	}

	UFinalRunSaveGame* SaveGame = Cast<UFinalRunSaveGame>(UGameplayStatics::LoadGameFromSlot(PrototypeRunSlotName, PrototypeRunUserIndex));
	if (SaveGame == nullptr)
	{
		SetFailureReason(FText::FromString(TEXT("Failed to read prototype run save slot.")));
		return false;
	}

	if (!GameFlowSubsystem->RestoreRunSessionFromSaveData(SaveGame->RunSaveData))
	{
		FText FlowFailureReason = GameFlowSubsystem->GetLastBattleFailureReason();
		if (FlowFailureReason.IsEmpty())
		{
			FlowFailureReason = FText::FromString(TEXT("Failed to restore RunSession from save data."));
		}

		SetFailureReason(FlowFailureReason);
		return false;
	}

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

void UFinalSaveGameCoordinator::SetFailureReason(const FText& Reason)
{
	LastFailureReason = Reason;
}

bool UFinalSaveGameCoordinator::HasActiveBattleSession() const
{
	const UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
	return GameFlowSubsystem != nullptr && GameFlowSubsystem->GetActiveBattleSession() != nullptr;
}
