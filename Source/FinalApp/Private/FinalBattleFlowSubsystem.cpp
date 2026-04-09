#include "Subsystems/FinalBattleFlowSubsystem.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Facade/FinalBattleSession.h"
#include "Queries/FinalDataRegistry.h"

void UFinalBattleFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UFinalBattleSession* UFinalBattleFlowSubsystem::CreateBattleSessionFromStartRequest(const FFinalBattleStartRequest& StartRequest)
{
	LastFailureReason = FText::GetEmpty();
	LastStartRequest = StartRequest;

	UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	if (DataRegistry == nullptr)
	{
		LastFailureReason = FText::FromString(TEXT("FinalDataRegistry is unavailable."));
		return nullptr;
	}

	UFinalBattleEncounterDefinition* EncounterDefinition = DataRegistry->FindEncounterDefinition(StartRequest.EncounterId);
	if (EncounterDefinition == nullptr)
	{
		LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingEncounter", "Encounter {0} is not registered."), FText::FromName(StartRequest.EncounterId.Value));
		return nullptr;
	}

	UFinalBattleRuleConfig* RuleConfig = DataRegistry->FindRuleConfig(StartRequest.RuleConfigId);
	if (RuleConfig == nullptr && !StartRequest.RuleConfigId.IsValid())
	{
		RuleConfig = EncounterDefinition->RuleConfig.LoadSynchronous();
	}

	if (RuleConfig == nullptr)
	{
		LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingRuleConfig", "Rule config {0} is not registered."), FText::FromName(StartRequest.RuleConfigId.Value));
		return nullptr;
	}

	FFinalBattleInitContext InitContext;
	if (!BuildInitContext(StartRequest, InitContext))
	{
		if (LastFailureReason.IsEmpty())
		{
			LastFailureReason = FText::FromString(TEXT("Failed to build battle init context from run state."));
		}
		return nullptr;
	}

	LastStartRequest = StartRequest;
	if (ActiveBattleSession)
	{
		ClearActiveBattleSession();
	}

	ActiveBattleSession = NewObject<UFinalBattleSession>(this);
	ActiveBattleSession->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
	return ActiveBattleSession;
}

bool UFinalBattleFlowSubsystem::SubmitBattleCommand(const FFinalBattleCommand& Command)
{
	if (!ActiveBattleSession)
	{
		return false;
	}

	ActiveBattleSession->SubmitCommand(Command);
	return true;
}

void UFinalBattleFlowSubsystem::ClearActiveBattleSession()
{
	if (ActiveBattleSession)
	{
		ActiveBattleSession->ResetSession();
	}

	ActiveBattleSession = nullptr;
}

FFinalBattleSnapshot UFinalBattleFlowSubsystem::GetCurrentSnapshot() const
{
	return ActiveBattleSession ? ActiveBattleSession->GetSnapshot() : FFinalBattleSnapshot{};
}

UFinalBattleSession* UFinalBattleFlowSubsystem::GetActiveBattleSession() const
{
	return ActiveBattleSession;
}

FText UFinalBattleFlowSubsystem::GetLastFailureReason() const
{
	return LastFailureReason;
}

bool UFinalBattleFlowSubsystem::BuildInitContext(const FFinalBattleStartRequest& StartRequest, FFinalBattleInitContext& OutInitContext)
{
	UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	if (DataRegistry == nullptr)
	{
		LastFailureReason = FText::FromString(TEXT("FinalDataRegistry is unavailable."));
		return false;
	}

	OutInitContext = FFinalBattleInitContext{};
	OutInitContext.TeamCurrentHP = StartRequest.TeamCurrentHP;

	const TArray<FFinalRunPersistentCharacterState>& SourcePartyStates = StartRequest.PartyStates;
	for (const FFinalRunPersistentCharacterState& PartyState : SourcePartyStates)
	{
		UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(PartyState.CharacterId);
		if (CharacterDefinition == nullptr)
		{
			LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingCharacter", "Character {0} is not registered."), FText::FromName(PartyState.CharacterId.Value));
			return false;
		}

		FFinalBattleCharacterInitData InitData;
		InitData.CharacterDefinition = CharacterDefinition;
		InitData.CurrentStress = PartyState.CurrentStress;
		InitData.bCollapsed = PartyState.bCollapsed;
		InitData.CurrentAwakenCount = PartyState.CurrentAwakenCount;
		InitData.CollapseCount = PartyState.CollapseCount;
		OutInitContext.PartyMembers.Add(InitData);
	}

	for (const FFinalCardId& CardId : StartRequest.DeckCardIds)
	{
		UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(CardId);
		if (CardDefinition == nullptr)
		{
			LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingCard", "Card {0} is not registered."), FText::FromName(CardId.Value));
			return false;
		}

		OutInitContext.DeckDefinitions.Add(CardDefinition);
	}

	return OutInitContext.PartyMembers.Num() > 0 && OutInitContext.DeckDefinitions.Num() > 0;
}
