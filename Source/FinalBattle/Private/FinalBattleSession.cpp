#include "Facade/FinalBattleSession.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Resolver/FinalBattleResolver.h"
#include "Runtime/FinalBattleState.h"

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
