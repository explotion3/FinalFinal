#include "First/Enemy/FirstBattleEnemyPartService.h"

#include "First/Events/FirstBattleEventLog.h"

FFirstEnemyPartState* FFirstBattleEnemyPartService::FindEnemyPart(FFirstBattleState& State, const FName PartId) const
{
	return State.EnemyParts.FindByPredicate(
		[PartId](const FFirstEnemyPartState& Part)
		{
			return Part.PartId == PartId;
		});
}

void FFirstBattleEnemyPartService::EnsureIntentSequence(FFirstEnemyPartState& Part) const
{
	if (Part.IntentSequence.IsEmpty())
	{
		FFirstEnemyPartIntentInstance& Intent = Part.IntentSequence.AddDefaulted_GetRef();
		Intent.IntentId = Part.CurrentIntentId;
		Intent.DisplayName = Part.CurrentIntentDisplayName;
		Intent.InitialInitiative = Part.CurrentInitiative;
	}

	if (!Part.IntentSequence.IsEmpty())
	{
		Part.CurrentIntentIndex = FMath::Clamp(Part.CurrentIntentIndex, 0, Part.IntentSequence.Num() - 1);
		const FFirstEnemyPartIntentInstance& CurrentIntent = Part.IntentSequence[Part.CurrentIntentIndex];
		Part.CurrentIntentId = CurrentIntent.IntentId;
		Part.CurrentIntentDisplayName = CurrentIntent.DisplayName;
	}
}

void FFirstBattleEnemyPartService::ResolveInitiativeAfterCard(FFirstBattleState& State, const FFirstCardInstance& Card, const TMap<FName, int32>& PreInitiativeByPart, const FFirstBattleEventLog& EventLog) const
{
	TArray<FName> QueuedPartIds;
	const int32 Cost = FMath::Max(Card.RuntimeCost, 0);

	for (FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (!IsAlivePart(Part))
		{
			continue;
		}

		const int32 PreviousInitiative = Part.CurrentInitiative;
		Part.CurrentInitiative = FMath::Max(0, Part.CurrentInitiative - Cost);

		FFirstBattleEvent Event;
		Event.EventType = EFirstBattleEventType::InitiativeChanged;
		Event.RelatedId = Part.PartId;
		Event.CardInstanceId = Card.CardInstanceId;
		Event.PartId = Part.PartId;
		Event.PrimaryValue = PreviousInitiative;
		Event.SecondaryValue = Part.CurrentInitiative;
		Event.Message = FText::Format(
			NSLOCTEXT("FirstBattle", "FirstInitiativeChanged", "{0} initiative {1} -> {2}."),
			Part.DisplayName.IsEmpty() ? FText::FromName(Part.PartId) : Part.DisplayName,
			FText::AsNumber(PreviousInitiative),
			FText::AsNumber(Part.CurrentInitiative));
		EventLog.AppendEvent(State, Event);

		const int32* PrePlayInitiative = PreInitiativeByPart.Find(Part.PartId);
		const bool bWasAboveZeroBeforeThisCard = PrePlayInitiative != nullptr && *PrePlayInitiative > 0;
		if (bWasAboveZeroBeforeThisCard && PreviousInitiative > 0 && Part.CurrentInitiative <= 0)
		{
			QueuedPartIds.Add(Part.PartId);
		}
	}

	ResolveQueuedEnemyPartActions(State, QueuedPartIds, Card.CardInstanceId, EventLog);
}

void FFirstBattleEnemyPartService::ResolveQueuedEnemyPartActions(FFirstBattleState& State, const TArray<FName>& QueuedPartIds, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const
{
	for (FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (State.bBattleEnded)
		{
			break;
		}

		if (!QueuedPartIds.Contains(Part.PartId) || !IsAlivePart(Part))
		{
			continue;
		}

		ResolveEnemyPartAction(State, Part, SourceCardInstanceId, EventLog);
	}
}

void FFirstBattleEnemyPartService::ResolveEnemyPartAction(FFirstBattleState& State, FFirstEnemyPartState& Part, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const
{
	if (!IsAlivePart(Part))
	{
		return;
	}

	const FName ActedIntentId = Part.CurrentIntentId;
	const int32 PreviousInitiative = Part.CurrentInitiative;
	FFirstEnemyPartIntentInstance CurrentIntent;
	if (Part.IntentSequence.IsValidIndex(Part.CurrentIntentIndex))
	{
		CurrentIntent = Part.IntentSequence[Part.CurrentIntentIndex];
	}
	else
	{
		CurrentIntent.IntentId = Part.CurrentIntentId;
		CurrentIntent.DisplayName = Part.CurrentIntentDisplayName;
		CurrentIntent.InitialInitiative = Part.CurrentInitiative;
	}

	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::EnemyPartActed;
	Event.RelatedId = ActedIntentId;
	Event.CardInstanceId = SourceCardInstanceId;
	Event.PartId = Part.PartId;
	Event.PrimaryValue = PreviousInitiative;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstEnemyPartActed", "{0} acted."),
		Part.DisplayName.IsEmpty() ? FText::FromName(Part.PartId) : Part.DisplayName);
	EventLog.AppendEvent(State, Event);

	ExecuteIntentEffects(State, Part, CurrentIntent, SourceCardInstanceId, EventLog);
	if (State.bBattleEnded)
	{
		return;
	}

	if (!Part.IntentSequence.IsEmpty())
	{
		Part.CurrentIntentIndex = (Part.CurrentIntentIndex + 1) % Part.IntentSequence.Num();
		ApplyIntentFromSequence(Part);
	}
}

void FFirstBattleEnemyPartService::ResolveVictory(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const
{
	if (State.EnemyParts.IsEmpty())
	{
		return;
	}

	const bool bAllPartsDestroyed = State.EnemyParts.ContainsByPredicate(
		[](const FFirstEnemyPartState& Part)
		{
			return IsAlivePart(Part);
		}) == false;

	if (!bAllPartsDestroyed || State.bBattleEnded)
	{
		return;
	}

	State.bBattleEnded = true;
	State.bPlayerVictory = true;

	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::BattleWon;
	Event.Message = NSLOCTEXT("FirstBattle", "FirstBattleWon", "Battle won.");
	EventLog.AppendEvent(State, Event);
}

void FFirstBattleEnemyPartService::ResolvePlayerDefeat(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const
{
	if (State.bBattleEnded || State.PlayerCurrentHP > 0)
	{
		return;
	}

	State.bBattleEnded = true;
	State.bPlayerVictory = false;

	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::BattleLost;
	Event.PrimaryValue = State.PlayerCurrentHP;
	Event.Message = NSLOCTEXT("FirstBattle", "FirstBattleLost", "Battle lost.");
	EventLog.AppendEvent(State, Event);
}

bool FFirstBattleEnemyPartService::IsAlivePart(const FFirstEnemyPartState& Part)
{
	return !Part.bDestroyed && Part.CurrentHP > 0;
}

void FFirstBattleEnemyPartService::ExecuteIntentEffects(FFirstBattleState& State, const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const
{
	for (const FFirstCardEffectInstance& Effect : Intent.Effects)
	{
		if (State.bBattleEnded)
		{
			break;
		}

		if (Effect.EffectType == EFirstCardEffectType::Damage)
		{
			ApplyIntentDamageEffect(State, Part, Intent, Effect, SourceCardInstanceId, EventLog);
		}
	}
}

void FFirstBattleEnemyPartService::ApplyIntentDamageEffect(FFirstBattleState& State, const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FFirstCardEffectInstance& Effect, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const
{
	if (Effect.Value <= 0 || State.PlayerCurrentHP <= 0)
	{
		return;
	}

	const int32 PreviousHP = State.PlayerCurrentHP;
	State.PlayerCurrentHP = FMath::Max(0, State.PlayerCurrentHP - Effect.Value);

	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::PlayerDamaged;
	Event.RelatedId = Intent.IntentId;
	Event.CardInstanceId = SourceCardInstanceId;
	Event.PartId = Part.PartId;
	Event.PrimaryValue = PreviousHP;
	Event.SecondaryValue = State.PlayerCurrentHP;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstPlayerDamaged", "{0} dealt {1} damage to the player."),
		Part.DisplayName.IsEmpty() ? FText::FromName(Part.PartId) : Part.DisplayName,
		FText::AsNumber(PreviousHP - State.PlayerCurrentHP));
	EventLog.AppendEvent(State, Event);

	ResolvePlayerDefeat(State, EventLog);
}

void FFirstBattleEnemyPartService::ApplyIntentFromSequence(FFirstEnemyPartState& Part) const
{
	if (Part.IntentSequence.IsEmpty())
	{
		return;
	}

	Part.CurrentIntentIndex = FMath::Clamp(Part.CurrentIntentIndex, 0, Part.IntentSequence.Num() - 1);
	const FFirstEnemyPartIntentInstance& CurrentIntent = Part.IntentSequence[Part.CurrentIntentIndex];
	Part.CurrentIntentId = CurrentIntent.IntentId;
	Part.CurrentIntentDisplayName = CurrentIntent.DisplayName;
	Part.CurrentInitiative = CurrentIntent.InitialInitiative;
}
