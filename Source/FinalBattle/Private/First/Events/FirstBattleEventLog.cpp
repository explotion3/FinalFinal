#include "First/Events/FirstBattleEventLog.h"

void FFirstBattleEventLog::AppendEvent(FFirstBattleState& State, const FFirstBattleEvent& Event) const
{
	State.Events.Add(Event);
}

void FFirstBattleEventLog::AppendPlayerTurnStartedEvent(FFirstBattleState& State) const
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::PlayerTurnStarted;
	Event.PrimaryValue = State.CurrentRound;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstPlayerTurnStarted", "Player turn {0} started."),
		FText::AsNumber(State.CurrentRound));
	AppendEvent(State, Event);
}

void FFirstBattleEventLog::AppendPlayerMaxHPChangedEvent(FFirstBattleState& State, const FFirstCardInstance& Card, const int32 PreviousMaxHP, const int32 NewMaxHP) const
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::PlayerMaxHPChanged;
	Event.RelatedId = Card.CardId;
	Event.CardInstanceId = Card.CardInstanceId;
	Event.PrimaryValue = PreviousMaxHP;
	Event.SecondaryValue = NewMaxHP;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstPlayerMaxHPChanged", "{0} changed player max HP {1} -> {2}."),
		Card.DisplayName.IsEmpty() ? FText::FromName(Card.CardId) : Card.DisplayName,
		FText::AsNumber(PreviousMaxHP),
		FText::AsNumber(NewMaxHP));
	AppendEvent(State, Event);
}

void FFirstBattleEventLog::AppendCardDrawnEvent(FFirstBattleState& State, const FFirstCardInstance& Card, const EFirstCardDrawSource DrawSource) const
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::CardDrawn;
	Event.RelatedId = Card.CardId;
	Event.CardInstanceId = Card.CardInstanceId;
	Event.PrimaryValue = static_cast<int32>(DrawSource);
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstCardDrawn", "{0} was drawn."),
		Card.DisplayName.IsEmpty() ? FText::FromName(Card.CardId) : Card.DisplayName);
	AppendEvent(State, Event);
}

void FFirstBattleEventLog::AppendDrawPileShuffledEvent(FFirstBattleState& State, const int32 ShuffledCount) const
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::DrawPileShuffled;
	Event.PrimaryValue = ShuffledCount;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstDrawPileShuffled", "Shuffled {0} cards into the draw pile."),
		FText::AsNumber(ShuffledCount));
	AppendEvent(State, Event);
}

void FFirstBattleEventLog::AppendCardReturnedToHandEvent(FFirstBattleState& State, const FFirstCardInstance& Card, const EFirstHandZone TargetZone) const
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::CardReturnedToHand;
	Event.RelatedId = Card.CardId;
	Event.CardInstanceId = Card.CardInstanceId;
	Event.PrimaryValue = static_cast<int32>(TargetZone);
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstCardReturnedToHand", "{0} returned to hand."),
		Card.DisplayName.IsEmpty() ? FText::FromName(Card.CardId) : Card.DisplayName);
	AppendEvent(State, Event);
}

void FFirstBattleEventLog::AppendHandCardMovedEvent(FFirstBattleState& State, const FFirstCardInstance& SourceCard, const FFirstCardInstance& MovedCard, const EFirstHandZone SourceZone, const EFirstHandZone TargetZone) const
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::HandCardMoved;
	Event.RelatedId = SourceCard.CardId;
	Event.CardInstanceId = MovedCard.CardInstanceId;
	Event.PrimaryValue = static_cast<int32>(SourceZone);
	Event.SecondaryValue = static_cast<int32>(TargetZone);
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstHandCardMoved", "Moved {0}."),
		MovedCard.DisplayName.IsEmpty() ? FText::FromName(MovedCard.CardId) : MovedCard.DisplayName);
	AppendEvent(State, Event);
}

void FFirstBattleEventLog::AppendCardRuntimeCostChangedEvent(FFirstBattleState& State, const FFirstCardInstance& Card, const int32 PreviousCost, const int32 NewCost) const
{
	if (PreviousCost == NewCost)
	{
		return;
	}

	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::CardRuntimeCostChanged;
	Event.RelatedId = Card.CardId;
	Event.CardInstanceId = Card.CardInstanceId;
	Event.PrimaryValue = PreviousCost;
	Event.SecondaryValue = NewCost;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstCardRuntimeCostChanged", "{0} cost {1} -> {2}."),
		Card.DisplayName.IsEmpty() ? FText::FromName(Card.CardId) : Card.DisplayName,
		FText::AsNumber(PreviousCost),
		FText::AsNumber(NewCost));
	AppendEvent(State, Event);
}
