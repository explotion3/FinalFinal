#include "First/FirstBattleKernel.h"

void FFirstBattleKernel::Initialize(const FFirstBattleStartParams& StartParams)
{
	State = FFirstBattleState();
	State.BattleId = StartParams.BattleId;
	State.CurrentRound = StartParams.StartingRound;
	State.bInitialized = true;
	State.HandCards = StartParams.InitialHand;

	for (const FFirstEnemyPartStartData& PartStartData : StartParams.EnemyParts)
	{
		FFirstEnemyPartState& PartState = State.EnemyParts.AddDefaulted_GetRef();
		PartState.PartId = PartStartData.PartId;
		PartState.DisplayName = PartStartData.DisplayName;
		PartState.PositionIndex = PartStartData.PositionIndex;
		PartState.MaxHP = PartStartData.MaxHP;
		PartState.CurrentHP = PartStartData.CurrentHP > 0 ? PartStartData.CurrentHP : PartStartData.MaxHP;
		PartState.bDestroyed = PartState.CurrentHP <= 0;
		PartState.CurrentIntentId = PartStartData.CurrentIntentId;
		PartState.CurrentIntentDisplayName = PartStartData.CurrentIntentDisplayName;
		PartState.IntentSequence = PartStartData.IntentSequence;
		PartState.CurrentIntentIndex = PartStartData.CurrentIntentIndex;
		PartState.CurrentInitiative = PartStartData.CurrentInitiative;
		EnsureIntentSequence(PartState);
	}

	State.EnemyParts.Sort(
		[](const FFirstEnemyPartState& Left, const FFirstEnemyPartState& Right)
		{
			return Left.PositionIndex < Right.PositionIndex;
		});
}

FFirstBattleCommandResult FFirstBattleKernel::SubmitCommand(const FFirstBattleCommand& Command)
{
	if (!State.bInitialized)
	{
		return MakeRejectedResult(TEXT("first.command.rejected.not_initialized"), NSLOCTEXT("FirstBattle", "FirstCommandRejectedNotInitialized", "First battle is not initialized."));
	}

	switch (Command.CommandType)
	{
	case EFirstBattleCommandType::PlayCard:
		return ResolvePlayCard(Command);
	case EFirstBattleCommandType::EndTurn:
		return ResolveEndTurn();
	default:
		return MakeRejectedResult(TEXT("first.command.rejected.unsupported"), NSLOCTEXT("FirstBattle", "FirstUnsupportedCommand", "Unsupported First battle command."));
	}
}

FFirstBattleSnapshot FFirstBattleKernel::BuildSnapshot() const
{
	FFirstBattleSnapshot Snapshot;
	Snapshot.BattleId = State.BattleId;
	Snapshot.CurrentRound = State.CurrentRound;
	Snapshot.bInitialized = State.bInitialized;
	Snapshot.bBattleEnded = State.bBattleEnded;
	Snapshot.bPlayerVictory = State.bPlayerVictory;
	Snapshot.DiscardPileCount = State.DiscardPile.Num();
	Snapshot.RecentEvents = State.Events;

	for (const FFirstCardInstance& CardInstance : State.HandCards)
	{
		FFirstCardViewData& CardView = Snapshot.HandCards.AddDefaulted_GetRef();
		CardView.CardInstanceId = CardInstance.CardInstanceId;
		CardView.CardId = CardInstance.CardId;
		CardView.DisplayName = CardInstance.DisplayName;
		CardView.BaseCost = CardInstance.BaseCost;
		CardView.RuntimeCost = CardInstance.RuntimeCost;
		CardView.Keywords = CardInstance.Keywords;
		CardView.Effects = CardInstance.Effects;
	}

	for (const FFirstEnemyPartState& PartState : State.EnemyParts)
	{
		FFirstEnemyPartViewData& PartView = Snapshot.EnemyParts.AddDefaulted_GetRef();
		PartView.PartId = PartState.PartId;
		PartView.DisplayName = PartState.DisplayName;
		PartView.PositionIndex = PartState.PositionIndex;
		PartView.MaxHP = PartState.MaxHP;
		PartView.CurrentHP = PartState.CurrentHP;
		PartView.bDestroyed = PartState.bDestroyed;
		PartView.CurrentIntentId = PartState.CurrentIntentId;
		PartView.CurrentIntentDisplayName = PartState.CurrentIntentDisplayName;
		PartView.CurrentIntentIndex = PartState.CurrentIntentIndex;
		PartView.CurrentInitiative = PartState.CurrentInitiative;
	}

	return Snapshot;
}

FFirstBattleCommandResult FFirstBattleKernel::ResolvePlayCard(const FFirstBattleCommand& Command)
{
	if (State.bBattleEnded)
	{
		return MakeRejectedResult(TEXT("first.command.rejected.battle_ended"), NSLOCTEXT("FirstBattle", "FirstPlayCardBattleEnded", "First battle has already ended."));
	}

	const int32 HandIndex = State.HandCards.IndexOfByPredicate(
		[&Command](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == Command.CardInstanceId;
		});
	if (HandIndex == INDEX_NONE)
	{
		return MakeRejectedResult(TEXT("first.command.rejected.card_not_in_hand"), NSLOCTEXT("FirstBattle", "FirstPlayCardCardNotInHand", "Card is not in hand."));
	}

	FFirstEnemyPartState* TargetPart = FindEnemyPart(Command.TargetPartId);
	if (TargetPart == nullptr)
	{
		return MakeRejectedResult(TEXT("first.command.rejected.target_not_found"), NSLOCTEXT("FirstBattle", "FirstPlayCardTargetNotFound", "Target enemy part was not found."));
	}
	if (!IsAlivePart(*TargetPart))
	{
		return MakeRejectedResult(TEXT("first.command.rejected.target_destroyed"), NSLOCTEXT("FirstBattle", "FirstPlayCardTargetDestroyed", "Target enemy part is destroyed."));
	}

	TMap<FName, int32> PrePlayInitiatives;
	for (const FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (IsAlivePart(Part))
		{
			PrePlayInitiatives.Add(Part.PartId, Part.CurrentInitiative);
		}
	}

	FFirstCardInstance PlayedCard = State.HandCards[HandIndex];
	State.HandCards.RemoveAt(HandIndex);
	State.DiscardPile.Add(PlayedCard);

	FFirstBattleEvent PlayedEvent;
	PlayedEvent.EventType = EFirstBattleEventType::CardPlayed;
	PlayedEvent.RelatedId = PlayedCard.CardId;
	PlayedEvent.CardInstanceId = PlayedCard.CardInstanceId;
	PlayedEvent.PartId = TargetPart->PartId;
	PlayedEvent.PrimaryValue = PlayedCard.RuntimeCost;
	PlayedEvent.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstCardPlayed", "Played {0}."),
		PlayedCard.DisplayName.IsEmpty() ? FText::FromName(PlayedCard.CardId) : PlayedCard.DisplayName);
	AppendEvent(PlayedEvent);

	ApplyDamageEffects(PlayedCard, *TargetPart);
	ResolvePerfectReleaseEvents(PlayedCard, PrePlayInitiatives);
	ResolveVictory();

	if (!State.bBattleEnded && !HasSwiftKeyword(PlayedCard))
	{
		ResolveInitiativeAfterCard(PlayedCard, PrePlayInitiatives);
	}

	return MakeAcceptedResult(NSLOCTEXT("FirstBattle", "FirstPlayCardAccepted", "Card played."));
}

FFirstBattleCommandResult FFirstBattleKernel::ResolveEndTurn()
{
	if (State.bBattleEnded)
	{
		return MakeRejectedResult(TEXT("first.command.rejected.battle_ended"), NSLOCTEXT("FirstBattle", "FirstEndTurnBattleEnded", "First battle has already ended."));
	}

	for (FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (IsAlivePart(Part))
		{
			ResolveEnemyPartAction(Part, FGuid());
		}
	}

	State.CurrentRound += 1;
	return MakeAcceptedResult(NSLOCTEXT("FirstBattle", "FirstEndTurnAccepted", "Turn ended."));
}

void FFirstBattleKernel::ApplyDamageEffects(const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart)
{
	if (!IsAlivePart(TargetPart))
	{
		return;
	}

	for (const FFirstCardEffectInstance& Effect : Card.Effects)
	{
		if (Effect.EffectType != EFirstCardEffectType::Damage || Effect.Value <= 0 || !IsAlivePart(TargetPart))
		{
			continue;
		}

		const int32 PreviousHP = TargetPart.CurrentHP;
		TargetPart.CurrentHP = FMath::Max(0, TargetPart.CurrentHP - Effect.Value);
		if (PreviousHP > 0 && TargetPart.CurrentHP <= 0)
		{
			TargetPart.bDestroyed = true;

			FFirstBattleEvent DestroyedEvent;
			DestroyedEvent.EventType = EFirstBattleEventType::EnemyPartDestroyed;
			DestroyedEvent.RelatedId = TargetPart.PartId;
			DestroyedEvent.CardInstanceId = Card.CardInstanceId;
			DestroyedEvent.PartId = TargetPart.PartId;
			DestroyedEvent.PrimaryValue = PreviousHP;
			DestroyedEvent.SecondaryValue = TargetPart.CurrentHP;
			DestroyedEvent.Message = FText::Format(
				NSLOCTEXT("FirstBattle", "FirstEnemyPartDestroyed", "{0} was destroyed."),
				TargetPart.DisplayName.IsEmpty() ? FText::FromName(TargetPart.PartId) : TargetPart.DisplayName);
			AppendEvent(DestroyedEvent);
		}
	}
}

void FFirstBattleKernel::ResolvePerfectReleaseEvents(const FFirstCardInstance& Card, const TMap<FName, int32>& PrePlayInitiatives)
{
	if (HasSwiftKeyword(Card))
	{
		return;
	}

	for (const FFirstEnemyPartState& Part : State.EnemyParts)
	{
		const int32* PreInitiative = PrePlayInitiatives.Find(Part.PartId);
		if (PreInitiative == nullptr || *PreInitiative != Card.RuntimeCost)
		{
			continue;
		}

		FFirstBattleEvent Event;
		Event.EventType = EFirstBattleEventType::PerfectReleaseTriggered;
		Event.RelatedId = Part.PartId;
		Event.CardInstanceId = Card.CardInstanceId;
		Event.PartId = Part.PartId;
		Event.PrimaryValue = Card.RuntimeCost;
		Event.SecondaryValue = *PreInitiative;
		Event.Message = FText::Format(
			NSLOCTEXT("FirstBattle", "FirstPerfectReleaseTriggered", "{0} triggered perfect release."),
			Part.DisplayName.IsEmpty() ? FText::FromName(Part.PartId) : Part.DisplayName);
		AppendEvent(Event);
	}
}

void FFirstBattleKernel::ResolveInitiativeAfterCard(const FFirstCardInstance& Card, const TMap<FName, int32>& PreInitiativeByPart)
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
		AppendEvent(Event);

		const int32* PrePlayInitiative = PreInitiativeByPart.Find(Part.PartId);
		const bool bWasAboveZeroBeforeThisCard = PrePlayInitiative != nullptr && *PrePlayInitiative > 0;
		if (bWasAboveZeroBeforeThisCard && PreviousInitiative > 0 && Part.CurrentInitiative <= 0)
		{
			QueuedPartIds.Add(Part.PartId);
		}
	}

	ResolveQueuedEnemyPartActions(QueuedPartIds, Card.CardInstanceId);
}

void FFirstBattleKernel::ResolveQueuedEnemyPartActions(const TArray<FName>& QueuedPartIds, const FGuid& SourceCardInstanceId)
{
	for (FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (!QueuedPartIds.Contains(Part.PartId) || !IsAlivePart(Part))
		{
			continue;
		}

		ResolveEnemyPartAction(Part, SourceCardInstanceId);
	}
}

void FFirstBattleKernel::ResolveEnemyPartAction(FFirstEnemyPartState& Part, const FGuid& SourceCardInstanceId)
{
	if (!IsAlivePart(Part))
	{
		return;
	}

	const FName ActedIntentId = Part.CurrentIntentId;
	const int32 PreviousInitiative = Part.CurrentInitiative;

	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::EnemyPartActed;
	Event.RelatedId = ActedIntentId;
	Event.CardInstanceId = SourceCardInstanceId;
	Event.PartId = Part.PartId;
	Event.PrimaryValue = PreviousInitiative;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstEnemyPartActed", "{0} acted."),
		Part.DisplayName.IsEmpty() ? FText::FromName(Part.PartId) : Part.DisplayName);

	if (!Part.IntentSequence.IsEmpty())
	{
		Part.CurrentIntentIndex = (Part.CurrentIntentIndex + 1) % Part.IntentSequence.Num();
		ApplyIntentFromSequence(Part);
		Event.SecondaryValue = Part.CurrentInitiative;
	}

	AppendEvent(Event);
}

void FFirstBattleKernel::ResolveVictory()
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
	AppendEvent(Event);
}

void FFirstBattleKernel::AppendEvent(const FFirstBattleEvent& Event)
{
	State.Events.Add(Event);
}

FFirstCardInstance* FFirstBattleKernel::FindHandCard(const FGuid& CardInstanceId)
{
	return State.HandCards.FindByPredicate(
		[&CardInstanceId](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == CardInstanceId;
		});
}

FFirstEnemyPartState* FFirstBattleKernel::FindEnemyPart(FName PartId)
{
	return State.EnemyParts.FindByPredicate(
		[PartId](const FFirstEnemyPartState& Part)
		{
			return Part.PartId == PartId;
		});
}

void FFirstBattleKernel::EnsureIntentSequence(FFirstEnemyPartState& Part)
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

void FFirstBattleKernel::ApplyIntentFromSequence(FFirstEnemyPartState& Part)
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

bool FFirstBattleKernel::IsAlivePart(const FFirstEnemyPartState& Part)
{
	return !Part.bDestroyed && Part.CurrentHP > 0;
}

bool FFirstBattleKernel::HasSwiftKeyword(const FFirstCardInstance& Card)
{
	const FGameplayTag SwiftKeyword = FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift"), false);
	return SwiftKeyword.IsValid() && Card.Keywords.HasTagExact(SwiftKeyword);
}

FFirstBattleCommandResult FFirstBattleKernel::MakeRejectedResult(FName ReasonTag, const FText& Message)
{
	FFirstBattleCommandResult Result;
	Result.ResultCode = EFirstBattleCommandResultCode::Rejected;
	Result.ReasonTag = ReasonTag;
	Result.Message = Message;
	return Result;
}

FFirstBattleCommandResult FFirstBattleKernel::MakeAcceptedResult(const FText& Message)
{
	FFirstBattleCommandResult Result;
	Result.ResultCode = EFirstBattleCommandResultCode::Accepted;
	Result.ReasonTag = NAME_None;
	Result.Message = Message;
	return Result;
}
