#include "First/Snapshots/FirstBattleSnapshotBuilder.h"

#include "First/Hand/FirstBattleHandService.h"

FFirstBattleSnapshot FFirstBattleSnapshotBuilder::BuildSnapshot(const FFirstBattleState& State) const
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
	FFirstBattleHandService::ResolveHandAnchorIndices(State.HandCards, LeftHandIndex, RightHandIndex);

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
		CardView.HandZone = FFirstBattleHandService::ResolveHandZoneForCard(State.HandCards, CardIndex, LeftHandIndex, RightHandIndex);
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
