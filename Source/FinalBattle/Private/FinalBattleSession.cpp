#include "Facade/FinalBattleSession.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Resolver/FinalBattleResolver.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"

UFinalBattleSession::UFinalBattleSession()
{
	Resolver = new FFinalBattleResolver();
}

UFinalBattleSession::~UFinalBattleSession()
{
	delete State;
	State = nullptr;

	delete Resolver;
	Resolver = nullptr;
}

void UFinalBattleSession::InitializeSession(UFinalBattleEncounterDefinition* InEncounterDefinition, UFinalBattleRuleConfig* InRuleConfig, const FFinalBattleInitContext& InitContext)
{
	EncounterDefinition = InEncounterDefinition;
	RuleConfig = InRuleConfig;
	delete State;
	State = new FFinalBattleState();

	delete Resolver;
	Resolver = new FFinalBattleResolver();
	Resolver->Initialize(*State, EncounterDefinition, RuleConfig, InitContext);
}

FFinalBattleEvent UFinalBattleSession::SubmitCommand(const FFinalBattleCommand& Command)
{
	if (State == nullptr || Resolver == nullptr)
	{
		FFinalBattleEvent Event;
		Event.EventType = EFinalBattleEventType::CommandRejected;
		Event.RejectReason = EFinalBattleCommandRejectReason::BattleNotInitialized;
		Event.ReasonTag = TEXT("battle.not_initialized");
		Event.Message = FText::FromString(TEXT("Battle session is not initialized."));
		return Event;
	}

	return Resolver->ExecuteCommand(*State, Command, RuleConfig);
}

FFinalBattleSnapshot UFinalBattleSession::GetSnapshot() const
{
	if (State == nullptr || Resolver == nullptr)
	{
		return FFinalBattleSnapshot{};
	}

	return Resolver->BuildSnapshot(*State);
}

TArray<FFinalBattleEvent> UFinalBattleSession::GetBattleLogEntries() const
{
	return State != nullptr ? State->BattleLogEntries : TArray<FFinalBattleEvent>{};
}

TArray<FFinalBattleEvent> UFinalBattleSession::GetBattleEventsSince(const int32 LastSeenEventSequence) const
{
	if (State == nullptr)
	{
		return {};
	}

	TArray<FFinalBattleEvent> Events;
	for (const FFinalBattleEvent& Event : State->BattleLogEntries)
	{
		if (Event.EventSequence > LastSeenEventSequence)
		{
			Events.Add(Event);
		}
	}

	return Events;
}

int32 UFinalBattleSession::GetLatestBattleEventSequence() const
{
	return State != nullptr ? State->LastEventSequence : 0;
}

TArray<FFinalBattleGrowthFactBatch> UFinalBattleSession::GetGrowthFactBatchesSince(const int32 LastSeenBatchSequence) const
{
	if (State == nullptr)
	{
		return {};
	}

	TArray<FFinalBattleGrowthFactBatch> Batches;
	for (const FFinalBattleGrowthFactBatch& Batch : State->GrowthFactBatches)
	{
		if (Batch.BatchSequence > LastSeenBatchSequence)
		{
			Batches.Add(Batch);
		}
	}

	return Batches;
}

int32 UFinalBattleSession::GetLatestGrowthFactBatchSequence() const
{
	return State != nullptr ? State->LastGrowthFactBatchSequence : 0;
}

bool UFinalBattleSession::RefreshCharacterRuntimeStats(const FFinalBattleCharacterRuntimeStats& RuntimeStats)
{
	if (State == nullptr || !RuntimeStats.CharacterId.IsValid())
	{
		return false;
	}

	FFinalBattleCharacterState* CharacterState = State->Characters.FindByPredicate([&RuntimeStats](const FFinalBattleCharacterState& Candidate)
	{
		return Candidate.CharacterId == RuntimeStats.CharacterId;
	});
	if (CharacterState == nullptr)
	{
		return false;
	}

	CharacterState->VitalShare = RuntimeStats.VitalShare;
	CharacterState->StressCap = RuntimeStats.StressCap;
	CharacterState->CurrentStress = FMath::Min(CharacterState->CurrentStress, CharacterState->StressCap);
	CharacterState->RuntimeAttack = RuntimeStats.RuntimeAttack;
	CharacterState->RuntimeDefense = RuntimeStats.RuntimeDefense;
	CharacterState->RuntimeBreakRate = RuntimeStats.RuntimeBreakRate;
	CharacterState->RuntimeCritChance = RuntimeStats.RuntimeCritChance;
	CharacterState->RuntimeCritDamage = RuntimeStats.RuntimeCritDamage;

	int32 UpdatedTeamMaxHP = 0;
	for (const FFinalBattleCharacterState& TeamCharacterState : State->Characters)
	{
		if (!TeamCharacterState.bCollapsed)
		{
			UpdatedTeamMaxHP += TeamCharacterState.VitalShare;
		}
	}

	State->TeamMaxHP = FMath::Max(UpdatedTeamMaxHP, 0);
	State->TeamCurrentHP = FMath::Min(State->TeamCurrentHP, State->TeamMaxHP);
	return true;
}

int32 UFinalBattleSession::RefreshCardsForRunCardInstance(const FFinalBattleCardRefreshRequest& RefreshRequest)
{
	if (State == nullptr)
	{
		return 0;
	}

	static const FFinalBattleCardService CardService;
	return CardService.RefreshCardsForRunCardInstance(*State, RefreshRequest);
}

void UFinalBattleSession::ResetSession()
{
	delete State;
	State = nullptr;

	delete Resolver;
	Resolver = new FFinalBattleResolver();
	EncounterDefinition = nullptr;
	RuleConfig = nullptr;
}

bool UFinalBattleSession::HasActiveBattle() const
{
	return State != nullptr;
}
