#include "App/FinalGameInstance.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalGameInstance, Log, All);

namespace FinalTestBootstrap
{
	struct FResolvedPrototypeDefinitions
	{
		UFinalBattleRuleConfig* RuleConfig = nullptr;
		UFinalBattleEncounterDefinition* EncounterDefinition = nullptr;
		UFinalCharacterDefinition* GuardianDefinition = nullptr;
		UFinalCharacterDefinition* SupportDefinition = nullptr;
		UFinalCardDefinition* GuardianStrikeCard = nullptr;
		UFinalCardDefinition* GuardianGuardCard = nullptr;
		UFinalCardDefinition* SupportShotCard = nullptr;
		UFinalCardDefinition* SupportFocusCard = nullptr;
		UFinalRunRouteDefinition* PrototypeRunRoute = nullptr;
	};

	const FName RuleConfigId(TEXT("rule.test.bootstrap"));
	const FName EncounterId(TEXT("encounter.test.bootstrap"));
	const FName GuardianCharacterId(TEXT("character.test.guardian"));
	const FName SupportCharacterId(TEXT("character.test.support"));
	const FName GuardianStrikeCardId(TEXT("card.test.guardian.strike"));
	const FName GuardianGuardCardId(TEXT("card.test.guardian.guard"));
	const FName SupportShotCardId(TEXT("card.test.support.shot"));
	const FName SupportFocusCardId(TEXT("card.test.support.focus"));
	const FName PrototypeRouteId(TEXT("run.route.test.prototype"));

	void AppendMissingDefinition(TArray<FString>& OutMissingIds, const FString& StableId, const bool bIsPresent)
	{
		if (!bIsPresent)
		{
			OutMissingIds.Add(StableId);
		}
	}

	FResolvedPrototypeDefinitions ResolvePrototypeDefinitionsFromRegistry(UFinalDataRegistry* DataRegistry, TArray<FString>& OutMissingIds)
	{
		FResolvedPrototypeDefinitions Definitions;
		if (DataRegistry == nullptr)
		{
			return Definitions;
		}

		Definitions.RuleConfig = DataRegistry->FindRuleConfig(FFinalRuleConfigId(RuleConfigId));
		Definitions.EncounterDefinition = DataRegistry->FindEncounterDefinition(FFinalEncounterId(EncounterId));
		Definitions.GuardianDefinition = DataRegistry->FindCharacterDefinition(FFinalCharacterId(GuardianCharacterId));
		Definitions.SupportDefinition = DataRegistry->FindCharacterDefinition(FFinalCharacterId(SupportCharacterId));
		Definitions.GuardianStrikeCard = DataRegistry->FindCardDefinition(FFinalCardId(GuardianStrikeCardId));
		Definitions.GuardianGuardCard = DataRegistry->FindCardDefinition(FFinalCardId(GuardianGuardCardId));
		Definitions.SupportShotCard = DataRegistry->FindCardDefinition(FFinalCardId(SupportShotCardId));
		Definitions.SupportFocusCard = DataRegistry->FindCardDefinition(FFinalCardId(SupportFocusCardId));
		Definitions.PrototypeRunRoute = DataRegistry->FindRunRouteDefinition(PrototypeRouteId);

		AppendMissingDefinition(OutMissingIds, RuleConfigId.ToString(), Definitions.RuleConfig != nullptr);
		AppendMissingDefinition(OutMissingIds, EncounterId.ToString(), Definitions.EncounterDefinition != nullptr);
		AppendMissingDefinition(OutMissingIds, GuardianCharacterId.ToString(), Definitions.GuardianDefinition != nullptr);
		AppendMissingDefinition(OutMissingIds, SupportCharacterId.ToString(), Definitions.SupportDefinition != nullptr);
		AppendMissingDefinition(OutMissingIds, GuardianStrikeCardId.ToString(), Definitions.GuardianStrikeCard != nullptr);
		AppendMissingDefinition(OutMissingIds, GuardianGuardCardId.ToString(), Definitions.GuardianGuardCard != nullptr);
		AppendMissingDefinition(OutMissingIds, SupportShotCardId.ToString(), Definitions.SupportShotCard != nullptr);
		AppendMissingDefinition(OutMissingIds, SupportFocusCardId.ToString(), Definitions.SupportFocusCard != nullptr);
		AppendMissingDefinition(OutMissingIds, PrototypeRouteId.ToString(), Definitions.PrototypeRunRoute != nullptr);

		return Definitions;
	}
}

void UFinalGameInstance::Init()
{
	Super::Init();
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
	const FinalTestBootstrap::FResolvedPrototypeDefinitions ResolvedDefinitions =
		FinalTestBootstrap::ResolvePrototypeDefinitionsFromRegistry(DataRegistry, MissingDefinitionIds);

	if (MissingDefinitionIds.Num() > 0)
	{
		LastTestFailureReason = FText::FromString(FString::Printf(
			TEXT("Prototype content definitions are missing from FinalDataRegistry: %s. Run FinalPrototypeContentBootstrap commandlet or add the missing assets to the project."),
			*FString::Join(MissingDefinitionIds, TEXT(", "))));
		return false;
	}

	TestRuleConfig = ResolvedDefinitions.RuleConfig;
	TestEncounterDefinition = ResolvedDefinitions.EncounterDefinition;
	TestGuardianDefinition = ResolvedDefinitions.GuardianDefinition;
	TestSupportDefinition = ResolvedDefinitions.SupportDefinition;
	TestGuardianStrikeCard = ResolvedDefinitions.GuardianStrikeCard;
	TestGuardianGuardCard = ResolvedDefinitions.GuardianGuardCard;
	TestSupportShotCard = ResolvedDefinitions.SupportShotCard;
	TestSupportFocusCard = ResolvedDefinitions.SupportFocusCard;
	TestPrototypeRunRoute = ResolvedDefinitions.PrototypeRunRoute;

	bTestBattleBootstrapRegistered = true;

	UE_LOG(LogFinalGameInstance, Log, TEXT("Resolved test battle bootstrap data from FinalDataRegistry assets."));
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

	FFinalRunPersistentCharacterState GuardianState;
	GuardianState.CharacterId = TestGuardianDefinition->CharacterId;
	GuardianState.CurrentStress = 0;
	GuardianState.bCollapsed = false;
	PartyStates.Add(GuardianState);

	FFinalRunPersistentCharacterState SupportState;
	SupportState.CharacterId = TestSupportDefinition->CharacterId;
	SupportState.CurrentStress = 1;
	SupportState.bCollapsed = false;
	PartyStates.Add(SupportState);

	TArray<FFinalCardId> DeckCardIds;
	DeckCardIds.Append({
		TestGuardianStrikeCard->CardId,
		TestGuardianGuardCard->CardId,
		TestSupportFocusCard->CardId,
		TestSupportShotCard->CardId,
		TestGuardianStrikeCard->CardId,
		TestSupportShotCard->CardId,
		TestGuardianGuardCard->CardId
	});

	const int32 TeamCurrentHP = TestGuardianDefinition->BaseVitalShare + TestSupportDefinition->BaseVitalShare;

	RunSession->ConfigureBattleStartState(
		TestEncounterDefinition->EncounterId,
		TestRuleConfig->RuleConfigId,
		PartyStates,
		DeckCardIds,
		TeamCurrentHP);
	if (!RunSession->ConfigureRunRouteById(FinalTestBootstrap::PrototypeRouteId))
	{
		LastTestFailureReason = FText::Format(
			NSLOCTEXT("FinalGameInstance", "ConfigurePrototypeRunRouteFailed", "Failed to configure prototype run route {0}."),
			FText::FromName(FinalTestBootstrap::PrototypeRouteId));
		return false;
	}

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetSubsystem<UFinalRunFlowSubsystem>())
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}

	return true;
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
