#include "App/FinalGameInstance.h"

#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalGameInstance, Log, All);

namespace FinalTestBootstrap
{
	const FName DefaultPrototypeBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));
	const FName TestPrototypeBootstrapId(TEXT("prototype.bootstrap.test"));

	void AppendMissingReference(TArray<FString>& OutMissingIds, const FString& StableId, const bool bIsPresent)
	{
		if (!bIsPresent)
		{
			OutMissingIds.Add(StableId);
		}
	}

	UFinalPrototypeBootstrapDefinition* ResolvePrototypeBootstrapDefinitionFromRegistry(
		UFinalDataRegistry* DataRegistry,
		const FName PrototypeBootstrapId,
		TArray<FString>& OutMissingIds)
	{
		if (DataRegistry == nullptr)
		{
			return nullptr;
		}

		UFinalPrototypeBootstrapDefinition* BootstrapDefinition = DataRegistry->FindPrototypeBootstrapDefinition(PrototypeBootstrapId);
		if (BootstrapDefinition == nullptr)
		{
			OutMissingIds.Add(PrototypeBootstrapId.ToString());
			return nullptr;
		}

		if (!BootstrapDefinition->IsValidDefinition())
		{
			OutMissingIds.Add(FString::Printf(TEXT("%s (invalid bootstrap definition)"), *BootstrapDefinition->BootstrapId.ToString()));
			return nullptr;
		}

		AppendMissingReference(OutMissingIds, BootstrapDefinition->RuleConfigId.ToString(), DataRegistry->FindRuleConfig(BootstrapDefinition->RuleConfigId) != nullptr);
		AppendMissingReference(OutMissingIds, BootstrapDefinition->EncounterId.ToString(), DataRegistry->FindEncounterDefinition(BootstrapDefinition->EncounterId) != nullptr);
		AppendMissingReference(OutMissingIds, BootstrapDefinition->RunRouteId.ToString(), DataRegistry->FindRunRouteDefinition(BootstrapDefinition->RunRouteId) != nullptr);

		for (const FFinalCharacterId& CharacterId : BootstrapDefinition->PartyCharacterIds)
		{
			AppendMissingReference(OutMissingIds, CharacterId.ToString(), DataRegistry->FindCharacterDefinition(CharacterId) != nullptr);
		}

		for (const FFinalPrototypeBootstrapCharacterState& CharacterState : BootstrapDefinition->InitialCharacterStates)
		{
			AppendMissingReference(OutMissingIds, CharacterState.CharacterId.ToString(), DataRegistry->FindCharacterDefinition(CharacterState.CharacterId) != nullptr);
		}

		for (const FFinalCardId& CardId : BootstrapDefinition->StarterDeckCardIds)
		{
			AppendMissingReference(OutMissingIds, CardId.ToString(), DataRegistry->FindCardDefinition(CardId) != nullptr);
		}

		TSet<FString> UniqueMissingIds(OutMissingIds);
		OutMissingIds = UniqueMissingIds.Array();
		OutMissingIds.Sort();
		return BootstrapDefinition;
	}
}

void UFinalGameInstance::Init()
{
	Super::Init();
	CurrentPrototypeBootstrapId = GetDefaultPrototypeBootstrapId();
	EnsureTestBattleBootstrapData();
}

bool UFinalGameInstance::EnsureTestBattleBootstrapData()
{
	LastTestFailureReason = FText::GetEmpty();

	if (bTestBattleBootstrapRegistered)
	{
		return true;
	}

	UFinalDataRegistry* DataRegistry = GetSubsystem<UFinalDataRegistry>();
	if (DataRegistry == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("FinalDataRegistry is unavailable."));
		return false;
	}

	TArray<FString> MissingDefinitionIds;
	UFinalPrototypeBootstrapDefinition* BootstrapDefinition =
		FinalTestBootstrap::ResolvePrototypeBootstrapDefinitionFromRegistry(DataRegistry, CurrentPrototypeBootstrapId, MissingDefinitionIds);

	if (BootstrapDefinition == nullptr || MissingDefinitionIds.Num() > 0)
	{
		LastTestFailureReason = FText::FromString(FString::Printf(
			TEXT("Prototype bootstrap content is missing or invalid in FinalDataRegistry: %s. Run FinalPrototypeContentBootstrap commandlet or fix the referenced assets."),
			*FString::Join(MissingDefinitionIds, TEXT(", "))));
		return false;
	}

	ActivePrototypeBootstrapDefinition = BootstrapDefinition;

	bTestBattleBootstrapRegistered = true;

	UE_LOG(LogFinalGameInstance, Log, TEXT("Resolved prototype bootstrap data from FinalDataRegistry bootstrap asset %s."), *BootstrapDefinition->BootstrapId.ToString());
	return true;
}

bool UFinalGameInstance::PrepareTestBattleRun()
{
	LastTestFailureReason = FText::GetEmpty();

	if (!EnsureTestBattleBootstrapData())
	{
		return false;
	}

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetSubsystem<UFinalGameFlowSubsystem>();
	if (GameFlowSubsystem == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable."));
		return false;
	}

	UFinalRunSession* RunSession = GameFlowSubsystem->BootstrapNewRun();
	if (RunSession == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("Failed to bootstrap a RunSession."));
		return false;
	}

	TArray<FFinalRunPersistentCharacterState> PartyStates;
	PartyStates.Reserve(ActivePrototypeBootstrapDefinition->InitialCharacterStates.Num());
	for (const FFinalPrototypeBootstrapCharacterState& BootstrapCharacterState : ActivePrototypeBootstrapDefinition->InitialCharacterStates)
	{
		FFinalRunPersistentCharacterState CharacterState;
		CharacterState.CharacterId = BootstrapCharacterState.CharacterId;
		CharacterState.CurrentStress = BootstrapCharacterState.CurrentStress;
		CharacterState.bCollapsed = BootstrapCharacterState.bCollapsed;
		CharacterState.CurrentAwakenCount = BootstrapCharacterState.CurrentAwakenCount;
		CharacterState.CollapseCount = BootstrapCharacterState.CollapseCount;
		PartyStates.Add(CharacterState);
	}

	RunSession->ConfigureBattleStartState(
		ActivePrototypeBootstrapDefinition->EncounterId,
		ActivePrototypeBootstrapDefinition->RuleConfigId,
		PartyStates,
		ActivePrototypeBootstrapDefinition->StarterDeckCardIds,
		ActivePrototypeBootstrapDefinition->InitialTeamCurrentHP);
	if (!RunSession->ConfigureRunRouteById(ActivePrototypeBootstrapDefinition->RunRouteId))
	{
		LastTestFailureReason = FText::Format(
			NSLOCTEXT("FinalGameInstance", "ConfigurePrototypeRunRouteFailed", "Failed to configure prototype run route {0}."),
			FText::FromName(ActivePrototypeBootstrapDefinition->RunRouteId));
		return false;
	}

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetSubsystem<UFinalRunFlowSubsystem>())
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}

	return true;
}

bool UFinalGameInstance::SetCurrentPrototypeBootstrapId(const FName NewBootstrapId, const bool bRestartPrototypeRun)
{
	LastTestFailureReason = FText::GetEmpty();

	if (NewBootstrapId.IsNone())
	{
		LastTestFailureReason = FText::FromString(TEXT("Prototype bootstrap id is invalid."));
		return false;
	}

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetSubsystem<UFinalGameFlowSubsystem>();
	if (GameFlowSubsystem != nullptr && GameFlowSubsystem->GetActiveBattleSession() != nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("Cannot switch prototype bootstrap while a battle session is active."));
		return false;
	}

	const FName PreviousBootstrapId = CurrentPrototypeBootstrapId;
	if (CurrentPrototypeBootstrapId == NewBootstrapId)
	{
		return bRestartPrototypeRun ? PrepareTestBattleRun() : true;
	}

	CurrentPrototypeBootstrapId = NewBootstrapId;
	bTestBattleBootstrapRegistered = false;
	ActivePrototypeBootstrapDefinition = nullptr;

	if (!EnsureTestBattleBootstrapData())
	{
		const FText SwitchFailureReason = LastTestFailureReason;
		CurrentPrototypeBootstrapId = PreviousBootstrapId;
		bTestBattleBootstrapRegistered = false;
		ActivePrototypeBootstrapDefinition = nullptr;
		EnsureTestBattleBootstrapData();
		LastTestFailureReason = SwitchFailureReason;
		return false;
	}

	return bRestartPrototypeRun ? PrepareTestBattleRun() : true;
}

bool UFinalGameInstance::StartTestBattle()
{
	LastTestFailureReason = FText::GetEmpty();

	if (!PrepareTestBattleRun())
	{
		return false;
	}

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetSubsystem<UFinalGameFlowSubsystem>();
	if (GameFlowSubsystem == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable."));
		return false;
	}

	if (GameFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		GameFlowSubsystem->TryAutoStartPreparedBattleFromRun();
	}

	if (GameFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastTestFailureReason = GameFlowSubsystem->GetLastBattleFailureReason();
		if (LastTestFailureReason.IsEmpty())
		{
			LastTestFailureReason = FText::FromString(TEXT("Failed to auto-start the prepared test battle run."));
		}
		return false;
	}

	return true;
}

FText UFinalGameInstance::GetLastTestFailureReason() const
{
	return LastTestFailureReason;
}

FName UFinalGameInstance::GetCurrentPrototypeBootstrapId() const
{
	return CurrentPrototypeBootstrapId;
}

FName UFinalGameInstance::GetDefaultPrototypeBootstrapId() const
{
	return FinalTestBootstrap::DefaultPrototypeBootstrapId;
}

FName UFinalGameInstance::GetTestPrototypeBootstrapId() const
{
	return FinalTestBootstrap::TestPrototypeBootstrapId;
}

FName UFinalGameInstance::GetCurrentPrototypeRunRouteId() const
{
	return ActivePrototypeBootstrapDefinition != nullptr
		? ActivePrototypeBootstrapDefinition->RunRouteId
		: NAME_None;
}
