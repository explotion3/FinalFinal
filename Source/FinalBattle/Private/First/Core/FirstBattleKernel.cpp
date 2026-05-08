#include "First/Core/FirstBattleKernel.h"

#include "First/Core/FirstBattleCommandResults.h"

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
	DeckService.ApplyEntryStatBonuses(State, EventLog);

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
		EnemyPartService.EnsureIntentSequence(PartState);
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
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.not_initialized"), NSLOCTEXT("FirstBattle", "FirstCommandRejectedNotInitialized", "First battle is not initialized."));
	}

	switch (Command.CommandType)
	{
	case EFirstBattleCommandType::PlayCard:
		return ResolvePlayCard(Command);
	case EFirstBattleCommandType::EndTurn:
		return ResolveEndTurn();
	default:
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.unsupported"), NSLOCTEXT("FirstBattle", "FirstUnsupportedCommand", "Unsupported First battle command."));
	}
}

FFirstBattleSnapshot FFirstBattleKernel::BuildSnapshot() const
{
	return SnapshotBuilder.BuildSnapshot(State);
}

FFirstBattleCommandResult FFirstBattleKernel::ResolvePlayCard(const FFirstBattleCommand& Command)
{
	if (State.bBattleEnded)
	{
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.battle_ended"), NSLOCTEXT("FirstBattle", "FirstPlayCardBattleEnded", "First battle has already ended."));
	}

	const int32 HandIndex = State.HandCards.IndexOfByPredicate(
		[&Command](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == Command.CardInstanceId;
		});
	if (HandIndex == INDEX_NONE)
	{
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.card_not_in_hand"), NSLOCTEXT("FirstBattle", "FirstPlayCardCardNotInHand", "Card is not in hand."));
	}

	FFirstEnemyPartState* TargetPart = EnemyPartService.FindEnemyPart(State, Command.TargetPartId);
	if (TargetPart == nullptr)
	{
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.target_not_found"), NSLOCTEXT("FirstBattle", "FirstPlayCardTargetNotFound", "Target enemy part was not found."));
	}
	if (!EnemyPartService.IsAlivePart(*TargetPart))
	{
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.target_destroyed"), NSLOCTEXT("FirstBattle", "FirstPlayCardTargetDestroyed", "Target enemy part is destroyed."));
	}

	const EFirstHandZone PlayedHandZone = HandService.ResolveCurrentHandZoneForCardIndex(State, HandIndex);
	if (State.HandCards[HandIndex].bRequiresHandZoneToPlay && State.HandCards[HandIndex].RequiredHandZone != PlayedHandZone)
	{
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.hand_zone_requirement_not_met"), NSLOCTEXT("FirstBattle", "FirstPlayCardHandZoneRequirementNotMet", "Card is not in the required hand zone."));
	}

	TMap<FName, int32> PrePlayInitiatives;
	for (const FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (EnemyPartService.IsAlivePart(Part))
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
	EventLog.AppendEvent(State, PlayedEvent);

	CardResolver.ExecuteCardEffects(State, PlayedCard, *TargetPart, EnemyPartService, HandService, EventLog);
	const bool bTriggeredPerfectRelease = CardResolver.ResolvePerfectReleaseEvents(State, PlayedCard, PrePlayInitiatives, EventLog);
	EnemyPartService.ResolveVictory(State, EventLog);

	if (!State.bBattleEnded && !CardResolver.HasSwiftKeyword(PlayedCard) && !CardResolver.ShouldSkipInitiativeReductionForZonePerfectRelease(PlayedCard, PlayedHandZone, bTriggeredPerfectRelease))
	{
		EnemyPartService.ResolveInitiativeAfterCard(State, PlayedCard, PrePlayInitiatives, EventLog);
	}

	CardResolver.ResolvePlayedCardDestination(State, PlayedCard, HandService, EventLog);

	return FFirstBattleCommandResults::Accepted(NSLOCTEXT("FirstBattle", "FirstPlayCardAccepted", "Card played."));
}

FFirstBattleCommandResult FFirstBattleKernel::ResolveEndTurn()
{
	if (State.bBattleEnded)
	{
		return FFirstBattleCommandResults::Rejected(TEXT("first.command.rejected.battle_ended"), NSLOCTEXT("FirstBattle", "FirstEndTurnBattleEnded", "First battle has already ended."));
	}

	for (FFirstEnemyPartState& Part : State.EnemyParts)
	{
		if (State.bBattleEnded)
		{
			break;
		}

		if (EnemyPartService.IsAlivePart(Part))
		{
			EnemyPartService.ResolveEnemyPartAction(State, Part, FGuid(), EventLog);
		}
	}

	if (!State.bBattleEnded)
	{
		State.CurrentRound += 1;
		EventLog.AppendPlayerTurnStartedEvent(State);
		DeckService.BeginPlayerTurnDraw(State, EventLog, HandService);
	}

	return FFirstBattleCommandResults::Accepted(NSLOCTEXT("FirstBattle", "FirstEndTurnAccepted", "Turn ended."));
}
