#include "Systems/FinalBattleEventService.h"

#include "Runtime/FinalBattleState.h"

void FFinalBattleEventService::AppendBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event) const
{
	FFinalBattleEvent EventToAppend = Event;
	EventToAppend.EventSequence = ++State.LastEventSequence;
	EventToAppend.BattleId = State.BattleId;
	EventToAppend.Round = EventToAppend.Round > 0 ? EventToAppend.Round : State.CurrentRound;
	EventToAppend.bBattleEnded = State.bBattleEnded;
	EventToAppend.bPlayerVictory = State.bPlayerVictory;
	State.BattleLogEntries.Add(MoveTemp(EventToAppend));
}

FFinalBattleEvent FFinalBattleEventService::FinalizeBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event) const
{
	AppendBattleEvent(State, Event);
	return State.BattleLogEntries.Last();
}
