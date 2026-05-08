#include "First/FirstBattleKernel.h"

void FFirstBattleKernel::Initialize(const FFirstBattleStartParams& StartParams)
{
	State = FFirstBattleState();
	State.BattleId = StartParams.BattleId;
	State.CurrentRound = StartParams.StartingRound;
	State.RandomSeed = StartParams.RandomSeed;
	State.RandomStream.Initialize(State.RandomSeed);
	State.PlayerMaxHP = FMath::Max(StartParams.PlayerMaxHP, StartParams.PlayerCurrentHP);
	State.PlayerCurrentHP = StartParams.PlayerCurrentHP > 0 ? StartParams.PlayerCurrentHP : State.PlayerMaxHP;
	State.PlayerCurrentHP = FMath::Clamp(State.PlayerCurrentHP, 0, State.PlayerMaxHP);
	State.bInitialized = true;
	State.HandCards = StartParams.InitialHand;
	State.DrawPile = StartParams.InitialDrawPile;
	ApplyEntryStatBonuses();

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
	Snapshot.PlayerMaxHP = State.PlayerMaxHP;
	Snapshot.PlayerCurrentHP = State.PlayerCurrentHP;
	Snapshot.bInitialized = State.bInitialized;
	Snapshot.bBattleEnded = State.bBattleEnded;
	Snapshot.bPlayerVictory = State.bPlayerVictory;
	Snapshot.DrawPileCount = State.DrawPile.Num();
	Snapshot.DiscardPileCount = State.DiscardPile.Num();
	Snapshot.RecentEvents = State.Events;

	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(State.HandCards, LeftHandIndex, RightHandIndex);

	for (int32 CardIndex = 0; CardIndex < State.HandCards.Num(); ++CardIndex)
	{
		const FFirstCardInstance& CardInstance = State.HandCards[CardIndex];
		FFirstCardViewData& CardView = Snapshot.HandCards.AddDefaulted_GetRef();
		CardView.CardInstanceId = CardInstance.CardInstanceId;
		CardView.CardId = CardInstance.CardId;
		CardView.DisplayName = CardInstance.DisplayName;
		CardView.HandIndex = CardIndex;
		CardView.BaseCost = CardInstance.BaseCost;
		CardView.RuntimeCost = CardInstance.RuntimeCost;
		CardView.PlayerMaxHPBonusOnEnterBattle = CardInstance.PlayerMaxHPBonusOnEnterBattle;
		CardView.PlayDestination = CardInstance.PlayDestination;
		CardView.HandRole = CardInstance.HandRole;
		CardView.HandZone = ResolveHandZoneForCard(State.HandCards, CardIndex, LeftHandIndex, RightHandIndex);
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

	const EFirstHandZone PlayedHandZone = ResolveCurrentHandZoneForCardIndex(HandIndex);
	if (State.HandCards[HandIndex].bRequiresHandZoneToPlay && State.HandCards[HandIndex].RequiredHandZone != PlayedHandZone)
	{
		return MakeRejectedResult(TEXT("first.command.rejected.hand_zone_requirement_not_met"), NSLOCTEXT("FirstBattle", "FirstPlayCardHandZoneRequirementNotMet", "Card is not in the required hand zone."));
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

	ExecuteCardEffects(PlayedCard, *TargetPart);
	const bool bTriggeredPerfectRelease = ResolvePerfectReleaseEvents(PlayedCard, PrePlayInitiatives);
	ResolveVictory();

	if (!State.bBattleEnded && !HasSwiftKeyword(PlayedCard) && !ShouldSkipInitiativeReductionForZonePerfectRelease(PlayedCard, PlayedHandZone, bTriggeredPerfectRelease))
	{
		ResolveInitiativeAfterCard(PlayedCard, PrePlayInitiatives);
	}

	ResolvePlayedCardDestination(PlayedCard);

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
		if (State.bBattleEnded)
		{
			break;
		}

		if (IsAlivePart(Part))
		{
			ResolveEnemyPartAction(Part, FGuid());
		}
	}

	if (!State.bBattleEnded)
	{
		State.CurrentRound += 1;
		AppendPlayerTurnStartedEvent();
		BeginPlayerTurnDraw();
	}

	return MakeAcceptedResult(NSLOCTEXT("FirstBattle", "FirstEndTurnAccepted", "Turn ended."));
}

void FFirstBattleKernel::ExecuteCardEffects(const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart)
{
	for (const FFirstCardEffectInstance& Effect : Card.Effects)
	{
		if (Effect.EffectType == EFirstCardEffectType::Damage)
		{
			ApplyCardDamageEffect(Card, TargetPart, Effect);
		}
		else if (Effect.EffectType == EFirstCardEffectType::MoveHandCard)
		{
			ApplyMoveHandCardEffect(Card, Effect);
		}
	}
}

void FFirstBattleKernel::ApplyCardDamageEffect(const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart, const FFirstCardEffectInstance& Effect)
{
	if (Effect.Value <= 0 || !IsAlivePart(TargetPart))
	{
		return;
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

void FFirstBattleKernel::ApplyMoveHandCardEffect(const FFirstCardInstance& Card, const FFirstCardEffectInstance& Effect)
{
	const int32 MoveCount = FMath::Max(Effect.MoveCardCount, 0);
	for (int32 MoveIndex = 0; MoveIndex < MoveCount; ++MoveIndex)
	{
		if (!MoveRandomHandCard(Card, Effect))
		{
			break;
		}
	}
}

bool FFirstBattleKernel::ResolvePerfectReleaseEvents(const FFirstCardInstance& Card, const TMap<FName, int32>& PrePlayInitiatives)
{
	if (HasSwiftKeyword(Card))
	{
		return false;
	}

	bool bTriggeredAny = false;
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
		bTriggeredAny = true;
	}

	return bTriggeredAny;
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
		if (State.bBattleEnded)
		{
			break;
		}

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

	AppendEvent(Event);

	ExecuteIntentEffects(Part, CurrentIntent, SourceCardInstanceId);
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

void FFirstBattleKernel::ExecuteIntentEffects(const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FGuid& SourceCardInstanceId)
{
	for (const FFirstCardEffectInstance& Effect : Intent.Effects)
	{
		if (State.bBattleEnded)
		{
			break;
		}

		if (Effect.EffectType == EFirstCardEffectType::Damage)
		{
			ApplyIntentDamageEffect(Part, Intent, Effect, SourceCardInstanceId);
		}
	}
}

void FFirstBattleKernel::ApplyIntentDamageEffect(const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FFirstCardEffectInstance& Effect, const FGuid& SourceCardInstanceId)
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
	AppendEvent(Event);

	ResolvePlayerDefeat();
}

void FFirstBattleKernel::BeginPlayerTurnDraw()
{
	constexpr int32 DrawTargetCount = 5;

	TArray<FFirstCardInstance> DrawnCards;
	PullMissingCoreCards(DrawnCards, DrawTargetCount);
	DrawCardsFromDrawPile(DrawnCards, DrawTargetCount);

	if (DrawnCards.IsEmpty())
	{
		RebuildHandWithRandomZones(State.HandCards);
		return;
	}

	State.HandCards.Append(DrawnCards);
	RebuildHandWithRandomZones(State.HandCards);
}

void FFirstBattleKernel::ApplyEntryStatBonuses()
{
	for (const FFirstCardInstance& Card : State.HandCards)
	{
		ApplyCardEntryStatBonus(Card);
	}

	for (const FFirstCardInstance& Card : State.DrawPile)
	{
		ApplyCardEntryStatBonus(Card);
	}
}

void FFirstBattleKernel::ApplyCardEntryStatBonus(const FFirstCardInstance& Card)
{
	const int32 Bonus = FMath::Max(0, Card.PlayerMaxHPBonusOnEnterBattle);
	if (Bonus <= 0)
	{
		return;
	}

	const int32 PreviousMaxHP = State.PlayerMaxHP;
	State.PlayerMaxHP += Bonus;
	State.PlayerCurrentHP = FMath::Clamp(State.PlayerCurrentHP + Bonus, 0, State.PlayerMaxHP);
	AppendPlayerMaxHPChangedEvent(Card, PreviousMaxHP, State.PlayerMaxHP);
}

void FFirstBattleKernel::PullMissingCoreCards(TArray<FFirstCardInstance>& DrawnCards, const int32 DrawTargetCount)
{
	if (DrawnCards.Num() >= DrawTargetCount)
	{
		return;
	}

	PullMissingCoreCard(EFirstHandRole::LeftHandCore, DrawnCards);
	if (DrawnCards.Num() >= DrawTargetCount)
	{
		return;
	}

	PullMissingCoreCard(EFirstHandRole::RightHandCore, DrawnCards);
}

bool FFirstBattleKernel::PullMissingCoreCard(const EFirstHandRole HandRole, TArray<FFirstCardInstance>& DrawnCards)
{
	const bool bAlreadyInHand = State.HandCards.ContainsByPredicate(
		[HandRole](const FFirstCardInstance& Card)
		{
			return Card.HandRole == HandRole;
		});
	if (bAlreadyInHand)
	{
		return false;
	}

	FFirstCardInstance CoreCard;
	if (RemoveFirstCardWithRole(State.DrawPile, HandRole, CoreCard))
	{
		DrawnCards.Add(CoreCard);
		AppendCardDrawnEvent(CoreCard, EFirstCardDrawSource::ForcedCoreFromDrawPile);
		return true;
	}

	if (RemoveFirstCardWithRole(State.DiscardPile, HandRole, CoreCard))
	{
		DrawnCards.Add(CoreCard);
		AppendCardDrawnEvent(CoreCard, EFirstCardDrawSource::ForcedCoreFromDiscard);
		return true;
	}

	return false;
}

bool FFirstBattleKernel::RemoveFirstCardWithRole(TArray<FFirstCardInstance>& Cards, const EFirstHandRole HandRole, FFirstCardInstance& OutCard)
{
	const int32 CardIndex = Cards.IndexOfByPredicate(
		[HandRole](const FFirstCardInstance& Card)
		{
			return Card.HandRole == HandRole;
		});
	if (CardIndex == INDEX_NONE)
	{
		return false;
	}

	OutCard = Cards[CardIndex];
	Cards.RemoveAt(CardIndex);
	return true;
}

void FFirstBattleKernel::DrawCardsFromDrawPile(TArray<FFirstCardInstance>& DrawnCards, const int32 DrawTargetCount)
{
	bool bDrawingFromShuffledDiscard = false;
	while (DrawnCards.Num() < DrawTargetCount)
	{
		if (State.DrawPile.IsEmpty())
		{
			if (!ShuffleDiscardIntoDrawPile())
			{
				break;
			}

			bDrawingFromShuffledDiscard = true;
		}

		FFirstCardInstance DrawnCard = State.DrawPile[0];
		State.DrawPile.RemoveAt(0);
		DrawnCards.Add(DrawnCard);
		AppendCardDrawnEvent(DrawnCard, bDrawingFromShuffledDiscard ? EFirstCardDrawSource::ShuffledDiscard : EFirstCardDrawSource::DrawPile);
	}
}

bool FFirstBattleKernel::ShuffleDiscardIntoDrawPile()
{
	if (State.DiscardPile.IsEmpty())
	{
		return false;
	}

	TArray<FFirstCardInstance> ShuffledCards = State.DiscardPile;
	State.DiscardPile.Reset();
	ShuffleCards(ShuffledCards);
	AppendDrawPileShuffledEvent(ShuffledCards.Num());
	State.DrawPile.Append(ShuffledCards);
	return true;
}

void FFirstBattleKernel::ShuffleCards(TArray<FFirstCardInstance>& Cards)
{
	for (int32 Index = Cards.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = State.RandomStream.RandRange(0, Index);
		if (SwapIndex != Index)
		{
			Cards.Swap(Index, SwapIndex);
		}
	}
}

void FFirstBattleKernel::RebuildHandWithRandomZones(TArray<FFirstCardInstance>& Cards)
{
	TArray<FFirstCardInstance> LeftCoreCards;
	TArray<FFirstCardInstance> RightCoreCards;
	TArray<FFirstCardInstance> OrdinaryCards;

	for (const FFirstCardInstance& Card : Cards)
	{
		if (Card.HandRole == EFirstHandRole::LeftHandCore)
		{
			LeftCoreCards.Add(Card);
		}
		else if (Card.HandRole == EFirstHandRole::RightHandCore)
		{
			RightCoreCards.Add(Card);
		}
		else
		{
			OrdinaryCards.Add(Card);
		}
	}

	const bool bHasLeftCore = !LeftCoreCards.IsEmpty();
	const bool bHasRightCore = !RightCoreCards.IsEmpty();
	TArray<FFirstCardInstance> LeftZoneCards;
	TArray<FFirstCardInstance> BothZoneCards;
	TArray<FFirstCardInstance> RightZoneCards;
	TArray<FFirstCardInstance> UnzonedCards;

	for (const FFirstCardInstance& Card : OrdinaryCards)
	{
		switch (RollZoneBucket(bHasLeftCore, bHasRightCore))
		{
		case 0:
			LeftZoneCards.Add(Card);
			break;
		case 1:
			BothZoneCards.Add(Card);
			break;
		case 2:
			RightZoneCards.Add(Card);
			break;
		default:
			UnzonedCards.Add(Card);
			break;
		}
	}

	Cards.Reset();
	Cards.Append(LeftZoneCards);
	Cards.Append(LeftCoreCards);
	Cards.Append(BothZoneCards);
	Cards.Append(RightCoreCards);
	Cards.Append(RightZoneCards);
	Cards.Append(UnzonedCards);
}

int32 FFirstBattleKernel::RollZoneBucket(const bool bHasLeftCore, const bool bHasRightCore)
{
	if (bHasLeftCore && bHasRightCore)
	{
		return State.RandomStream.RandRange(0, 2);
	}

	if (bHasLeftCore)
	{
		return 0;
	}

	if (bHasRightCore)
	{
		return 2;
	}

	return INDEX_NONE;
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

void FFirstBattleKernel::ResolvePlayerDefeat()
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
	AppendEvent(Event);
}

void FFirstBattleKernel::AppendEvent(const FFirstBattleEvent& Event)
{
	State.Events.Add(Event);
}

void FFirstBattleKernel::AppendPlayerTurnStartedEvent()
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::PlayerTurnStarted;
	Event.PrimaryValue = State.CurrentRound;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstPlayerTurnStarted", "Player turn {0} started."),
		FText::AsNumber(State.CurrentRound));
	AppendEvent(Event);
}

void FFirstBattleKernel::AppendPlayerMaxHPChangedEvent(const FFirstCardInstance& Card, const int32 PreviousMaxHP, const int32 NewMaxHP)
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
	AppendEvent(Event);
}

void FFirstBattleKernel::AppendCardDrawnEvent(const FFirstCardInstance& Card, const EFirstCardDrawSource DrawSource)
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::CardDrawn;
	Event.RelatedId = Card.CardId;
	Event.CardInstanceId = Card.CardInstanceId;
	Event.PrimaryValue = static_cast<int32>(DrawSource);
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstCardDrawn", "{0} was drawn."),
		Card.DisplayName.IsEmpty() ? FText::FromName(Card.CardId) : Card.DisplayName);
	AppendEvent(Event);
}

void FFirstBattleKernel::AppendDrawPileShuffledEvent(const int32 ShuffledCount)
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::DrawPileShuffled;
	Event.PrimaryValue = ShuffledCount;
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstDrawPileShuffled", "Shuffled {0} cards into the draw pile."),
		FText::AsNumber(ShuffledCount));
	AppendEvent(Event);
}

void FFirstBattleKernel::ResolvePlayedCardDestination(const FFirstCardInstance& PlayedCard)
{
	if (PlayedCard.PlayDestination == EFirstCardPlayDestination::ReturnToHandRandomZone)
	{
		ReturnPlayedCardToHandRandomZone(PlayedCard);
	}
}

void FFirstBattleKernel::ReturnPlayedCardToHandRandomZone(const FFirstCardInstance& PlayedCard)
{
	const int32 DiscardIndex = State.DiscardPile.IndexOfByPredicate(
		[&PlayedCard](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == PlayedCard.CardInstanceId;
		});
	if (DiscardIndex == INDEX_NONE)
	{
		return;
	}

	FFirstCardInstance ReturningCard = State.DiscardPile[DiscardIndex];
	State.DiscardPile.RemoveAt(DiscardIndex);

	EFirstHandZone TargetZone = EFirstHandZone::None;
	const TArray<EFirstHandZone> TargetZones = ResolveValidHandMoveTargetZones();
	if (!TargetZones.IsEmpty())
	{
		TargetZone = TargetZones[State.RandomStream.RandRange(0, TargetZones.Num() - 1)];
		if (!InsertCardIntoZone(FFirstCardInstance(ReturningCard), TargetZone))
		{
			TargetZone = EFirstHandZone::None;
			State.HandCards.Add(ReturningCard);
		}
	}
	else
	{
		State.HandCards.Add(ReturningCard);
	}

	AppendCardReturnedToHandEvent(ReturningCard, TargetZone);
}

TArray<EFirstHandZone> FFirstBattleKernel::ResolveValidHandMoveTargetZones() const
{
	TArray<EFirstHandZone> TargetZones;
	for (const EFirstHandZone Zone : {EFirstHandZone::Left, EFirstHandZone::Both, EFirstHandZone::Right})
	{
		if (IsValidHandMoveTargetZone(State.HandCards, Zone))
		{
			TargetZones.Add(Zone);
		}
	}
	return TargetZones;
}

void FFirstBattleKernel::AppendCardReturnedToHandEvent(const FFirstCardInstance& Card, const EFirstHandZone TargetZone)
{
	FFirstBattleEvent Event;
	Event.EventType = EFirstBattleEventType::CardReturnedToHand;
	Event.RelatedId = Card.CardId;
	Event.CardInstanceId = Card.CardInstanceId;
	Event.PrimaryValue = static_cast<int32>(TargetZone);
	Event.Message = FText::Format(
		NSLOCTEXT("FirstBattle", "FirstCardReturnedToHand", "{0} returned to hand."),
		Card.DisplayName.IsEmpty() ? FText::FromName(Card.CardId) : Card.DisplayName);
	AppendEvent(Event);
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

EFirstHandZone FFirstBattleKernel::ResolveCurrentHandZoneForCardIndex(const int32 CardIndex) const
{
	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(State.HandCards, LeftHandIndex, RightHandIndex);
	return ResolveHandZoneForCard(State.HandCards, CardIndex, LeftHandIndex, RightHandIndex);
}

bool FFirstBattleKernel::MoveRandomHandCard(const FFirstCardInstance& SourceCard, const FFirstCardEffectInstance& Effect)
{
	const TArray<int32> CandidateIndices = CollectMoveCandidateIndices(Effect);
	if (CandidateIndices.IsEmpty())
	{
		return false;
	}

	const int32 CandidateListIndex = State.RandomStream.RandRange(0, CandidateIndices.Num() - 1);
	const int32 CardIndex = CandidateIndices[CandidateListIndex];
	if (!State.HandCards.IsValidIndex(CardIndex))
	{
		return false;
	}

	const EFirstHandZone SourceZone = ResolveCurrentHandZoneForCardIndex(CardIndex);
	const TArray<EFirstHandZone> TargetZones = ResolveMoveTargetZones(Effect, SourceZone);
	if (TargetZones.IsEmpty())
	{
		return false;
	}

	const EFirstHandZone TargetZone = TargetZones[State.RandomStream.RandRange(0, TargetZones.Num() - 1)];
	FFirstCardInstance MovedCard = State.HandCards[CardIndex];
	State.HandCards.RemoveAt(CardIndex);
	const int32 PreviousTargetCost = MovedCard.RuntimeCost;
	const int32 ActualTargetCostDelta = ApplyRuntimeCostDelta(MovedCard, Effect.MoveTargetCostDelta);

	if (!InsertCardIntoZone(FFirstCardInstance(MovedCard), TargetZone))
	{
		MovedCard.RuntimeCost = PreviousTargetCost;
		State.HandCards.Insert(MovedCard, FMath::Clamp(CardIndex, 0, State.HandCards.Num()));
		return false;
	}

	AppendHandCardMovedEvent(SourceCard, MovedCard, SourceZone, TargetZone);
	if (ActualTargetCostDelta != 0)
	{
		AppendCardRuntimeCostChangedEvent(MovedCard, PreviousTargetCost, MovedCard.RuntimeCost);
	}

	if (Effect.bTransferActualCostReductionToSourceCard && ActualTargetCostDelta < 0)
	{
		ApplyTransferredCostReductionToSourceCard(SourceCard, -ActualTargetCostDelta);
	}
	return true;
}

TArray<int32> FFirstBattleKernel::CollectMoveCandidateIndices(const FFirstCardEffectInstance& Effect) const
{
	TArray<int32> CandidateIndices;
	for (int32 CardIndex = 0; CardIndex < State.HandCards.Num(); ++CardIndex)
	{
		const FFirstCardInstance& Card = State.HandCards[CardIndex];
		if (Card.HandRole != EFirstHandRole::None)
		{
			continue;
		}

		const EFirstHandZone CardZone = ResolveCurrentHandZoneForCardIndex(CardIndex);
		if (Effect.bMoveRequiresSourceZone && CardZone != Effect.MoveSourceZone)
		{
			continue;
		}

		CandidateIndices.Add(CardIndex);
	}
	return CandidateIndices;
}

TArray<EFirstHandZone> FFirstBattleKernel::ResolveMoveTargetZones(const FFirstCardEffectInstance& Effect, const EFirstHandZone SourceZone) const
{
	TArray<EFirstHandZone> TargetZones;
	if (Effect.MoveTargetPolicy == EFirstHandMoveTargetPolicy::FixedZone)
	{
		if (IsValidHandMoveTargetZone(State.HandCards, Effect.MoveTargetZone))
		{
			TargetZones.Add(Effect.MoveTargetZone);
		}
		return TargetZones;
	}

	for (const EFirstHandZone Zone : {EFirstHandZone::Left, EFirstHandZone::Both, EFirstHandZone::Right})
	{
		if (!IsValidHandMoveTargetZone(State.HandCards, Zone))
		{
			continue;
		}
		if (Effect.MoveTargetPolicy == EFirstHandMoveTargetPolicy::RandomOtherThanSourceZone && Zone == SourceZone)
		{
			continue;
		}
		TargetZones.Add(Zone);
	}
	return TargetZones;
}

bool FFirstBattleKernel::InsertCardIntoZone(FFirstCardInstance&& Card, const EFirstHandZone TargetZone)
{
	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(State.HandCards, LeftHandIndex, RightHandIndex);

	if (TargetZone == EFirstHandZone::Left && LeftHandIndex != INDEX_NONE)
	{
		const int32 InsertIndex = State.RandomStream.RandRange(0, LeftHandIndex);
		State.HandCards.Insert(MoveTemp(Card), InsertIndex);
		return true;
	}

	if (TargetZone == EFirstHandZone::Both && LeftHandIndex != INDEX_NONE && RightHandIndex != INDEX_NONE && LeftHandIndex < RightHandIndex)
	{
		const int32 InsertIndex = State.RandomStream.RandRange(LeftHandIndex + 1, RightHandIndex);
		State.HandCards.Insert(MoveTemp(Card), InsertIndex);
		return true;
	}

	if (TargetZone == EFirstHandZone::Right && RightHandIndex != INDEX_NONE)
	{
		const int32 InsertIndex = State.RandomStream.RandRange(RightHandIndex + 1, State.HandCards.Num());
		State.HandCards.Insert(MoveTemp(Card), InsertIndex);
		return true;
	}

	return false;
}

void FFirstBattleKernel::AppendHandCardMovedEvent(const FFirstCardInstance& SourceCard, const FFirstCardInstance& MovedCard, const EFirstHandZone SourceZone, const EFirstHandZone TargetZone)
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
	AppendEvent(Event);
}

int32 FFirstBattleKernel::ApplyRuntimeCostDelta(FFirstCardInstance& Card, const int32 CostDelta)
{
	if (CostDelta == 0)
	{
		return 0;
	}

	const int32 PreviousCost = Card.RuntimeCost;
	Card.RuntimeCost = FMath::Max(0, Card.RuntimeCost + CostDelta);
	return Card.RuntimeCost - PreviousCost;
}

void FFirstBattleKernel::ApplyTransferredCostReductionToSourceCard(const FFirstCardInstance& SourceCard, const int32 ActualCostReduction)
{
	if (ActualCostReduction <= 0)
	{
		return;
	}

	FFirstCardInstance* DiscardedSourceCard = State.DiscardPile.FindByPredicate(
		[&SourceCard](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == SourceCard.CardInstanceId;
		});
	if (DiscardedSourceCard == nullptr)
	{
		return;
	}

	const int32 PreviousCost = DiscardedSourceCard->RuntimeCost;
	DiscardedSourceCard->RuntimeCost += ActualCostReduction;
	AppendCardRuntimeCostChangedEvent(*DiscardedSourceCard, PreviousCost, DiscardedSourceCard->RuntimeCost);
}

void FFirstBattleKernel::AppendCardRuntimeCostChangedEvent(const FFirstCardInstance& Card, const int32 PreviousCost, const int32 NewCost)
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
	AppendEvent(Event);
}

bool FFirstBattleKernel::ShouldSkipInitiativeReductionForZonePerfectRelease(const FFirstCardInstance& Card, const EFirstHandZone PlayedHandZone, const bool bTriggeredPerfectRelease) const
{
	return Card.bSkipInitiativeReductionOnPerfectReleaseInZone
		&& bTriggeredPerfectRelease
		&& Card.PerfectReleaseInitiativeSkipZone == PlayedHandZone;
}

void FFirstBattleKernel::ResolveHandAnchorIndices(const TArray<FFirstCardInstance>& HandCards, int32& OutLeftHandIndex, int32& OutRightHandIndex)
{
	OutLeftHandIndex = HandCards.IndexOfByPredicate(
		[](const FFirstCardInstance& Card)
		{
			return Card.HandRole == EFirstHandRole::LeftHandCore;
		});
	OutRightHandIndex = HandCards.IndexOfByPredicate(
		[](const FFirstCardInstance& Card)
		{
			return Card.HandRole == EFirstHandRole::RightHandCore;
		});
}

bool FFirstBattleKernel::IsValidHandMoveTargetZone(const TArray<FFirstCardInstance>& HandCards, const EFirstHandZone Zone)
{
	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(HandCards, LeftHandIndex, RightHandIndex);

	if (Zone == EFirstHandZone::Left)
	{
		return LeftHandIndex != INDEX_NONE;
	}
	if (Zone == EFirstHandZone::Both)
	{
		return LeftHandIndex != INDEX_NONE && RightHandIndex != INDEX_NONE && LeftHandIndex < RightHandIndex;
	}
	if (Zone == EFirstHandZone::Right)
	{
		return RightHandIndex != INDEX_NONE;
	}
	return false;
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

EFirstHandZone FFirstBattleKernel::ResolveHandZoneForCard(const TArray<FFirstCardInstance>& HandCards, const int32 CardIndex, const int32 LeftHandIndex, const int32 RightHandIndex)
{
	if (!HandCards.IsValidIndex(CardIndex) || HandCards[CardIndex].HandRole != EFirstHandRole::None)
	{
		return EFirstHandZone::None;
	}

	if (LeftHandIndex != INDEX_NONE && RightHandIndex != INDEX_NONE)
	{
		if (LeftHandIndex < RightHandIndex)
		{
			if (CardIndex < LeftHandIndex)
			{
				return EFirstHandZone::Left;
			}
			if (CardIndex > RightHandIndex)
			{
				return EFirstHandZone::Right;
			}
			if (CardIndex > LeftHandIndex && CardIndex < RightHandIndex)
			{
				return EFirstHandZone::Both;
			}
		}
		else
		{
			if (CardIndex < RightHandIndex)
			{
				return EFirstHandZone::Left;
			}
			if (CardIndex > LeftHandIndex)
			{
				return EFirstHandZone::Right;
			}
		}

		return EFirstHandZone::None;
	}

	if (LeftHandIndex != INDEX_NONE && CardIndex < LeftHandIndex)
	{
		return EFirstHandZone::Left;
	}

	if (RightHandIndex != INDEX_NONE && CardIndex > RightHandIndex)
	{
		return EFirstHandZone::Right;
	}

	return EFirstHandZone::None;
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
