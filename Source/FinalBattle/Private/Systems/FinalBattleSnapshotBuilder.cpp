#include "Systems/FinalBattleSnapshotBuilder.h"

#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleStatusService.h"

namespace
{
bool CanActivateUltimate(const FFinalBattleState& State, const FFinalBattleCharacterState& CharacterState)
{
	return CharacterState.UltimateDefinition != nullptr
		&& !CharacterState.bCollapsed
		&& !CharacterState.bUltimateUsedThisBattle
		&& CharacterState.UltimateId.IsValid()
		&& State.CurrentEP >= CharacterState.UltimateCostEP;
}

FFinalBattlePhaseProgressViewData BuildPhaseProgress(const FFinalBattleEnemyState& EnemyState)
{
	FFinalBattlePhaseProgressViewData PhaseProgress;
	PhaseProgress.TotalPhases = EnemyState.PhaseSequence.Num();

	if (!EnemyState.PhaseSequence.IsValidIndex(EnemyState.CurrentPhaseIndex) || EnemyState.MaxHP <= 0)
	{
		return PhaseProgress;
	}

	PhaseProgress.CurrentPhaseNumber = EnemyState.CurrentPhaseIndex + 1;

	const float CurrentHpPercent = FMath::Clamp(static_cast<float>(EnemyState.CurrentHP) / static_cast<float>(EnemyState.MaxHP), 0.0f, 1.0f);
	const float UpperBound = EnemyState.CurrentPhaseIndex > 0
		? EnemyState.PhaseSequence[EnemyState.CurrentPhaseIndex - 1].MaxHpPercent
		: 1.0f;
	const float LowerBound = EnemyState.CurrentPhaseIndex + 1 < EnemyState.PhaseSequence.Num()
		? EnemyState.PhaseSequence[EnemyState.CurrentPhaseIndex + 1].MaxHpPercent
		: 0.0f;
	const float PhaseSpan = FMath::Max(UpperBound - LowerBound, KINDA_SMALL_NUMBER);
	PhaseProgress.ProgressWithinPhase = FMath::Clamp((UpperBound - CurrentHpPercent) / PhaseSpan, 0.0f, 1.0f);
	return PhaseProgress;
}
}

FFinalBattleSnapshot FFinalBattleSnapshotBuilder::BuildSnapshot(
	const FFinalBattleState& State,
	const FFinalBattleCardService& CardService,
	const FFinalBattleStatusService& StatusService) const
{
	FFinalBattleSnapshot Snapshot;
	Snapshot.BattleId = State.BattleId;
	Snapshot.EncounterId = State.EncounterId;
	Snapshot.RuleConfigId = State.RuleConfigId;
	Snapshot.EncounterDisplayName = State.EncounterDisplayName;
	Snapshot.CurrentRound = State.CurrentRound;
	Snapshot.CurrentAP = State.CurrentAP;
	Snapshot.CurrentEP = State.CurrentEP;
	Snapshot.MaxEP = State.MaxEP;
	Snapshot.TeamCurrentHP = State.TeamCurrentHP;
	Snapshot.TeamMaxHP = State.TeamMaxHP;
	Snapshot.TeamShield = State.TeamShield;
	Snapshot.bBattleEnded = State.bBattleEnded;
	Snapshot.bPlayerVictory = State.bPlayerVictory;
	Snapshot.CurrentTargetUnitId = State.CurrentTargetUnitId;
	Snapshot.DeckState.DrawPileCount = State.DeckState.DrawPileCardInstanceIds.Num();
	Snapshot.DeckState.HandCount = State.DeckState.HandCardInstanceIds.Num();
	Snapshot.DeckState.DiscardPileCount = State.DeckState.DiscardPileCardInstanceIds.Num();
	Snapshot.DeckState.OngoingZoneCount = State.DeckState.OngoingZoneCardInstanceIds.Num();
	Snapshot.DeckState.ConsumePileCount = State.DeckState.ConsumePileCardInstanceIds.Num();

	for (const FFinalBattleCharacterState& CharacterState : State.Characters)
	{
		FFinalBattleCharacterViewData CharacterView;
		CharacterView.RuntimeUnitId = CharacterState.RuntimeUnitId;
		CharacterView.CharacterId = CharacterState.CharacterId;
		CharacterView.DisplayName = CharacterState.DisplayName;
		CharacterView.CurrentStress = CharacterState.CurrentStress;
		CharacterView.StressCap = CharacterState.StressCap;
		CharacterView.bCollapsed = CharacterState.bCollapsed;
		CharacterView.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
		CharacterView.CurrentAwakenThreshold = CharacterState.CurrentAwakenThreshold;
		CharacterView.CollapseCount = CharacterState.CollapseCount;
		CharacterView.VitalShare = CharacterState.VitalShare;
		Snapshot.Characters.Add(MoveTemp(CharacterView));

		FFinalBattleUltimateViewData UltimateView;
		UltimateView.OwnerUnitId = CharacterState.RuntimeUnitId;
		UltimateView.CharacterId = CharacterState.CharacterId;
		UltimateView.UltimateId = CharacterState.UltimateId;
		UltimateView.DisplayName = CharacterState.UltimateDisplayName;
		UltimateView.CostEP = CharacterState.UltimateCostEP;
		UltimateView.bDefinitionReady = CharacterState.UltimateDefinition != nullptr;
		UltimateView.bBlockedByCollapse = CharacterState.bCollapsed;
		UltimateView.bCanActivate = CanActivateUltimate(State, CharacterState);
		UltimateView.bUsedThisBattle = CharacterState.bUltimateUsedThisBattle;
		Snapshot.CharacterUltimates.Add(MoveTemp(UltimateView));
	}

	Snapshot.ActiveRelics = State.ActiveRelics;

	for (const FFinalBattleEnemyState& EnemyState : State.Enemies)
	{
		FFinalBattleEnemyViewData EnemyView;
		EnemyView.RuntimeUnitId = EnemyState.RuntimeUnitId;
		EnemyView.EnemyId = EnemyState.EnemyId;
		EnemyView.DisplayName = EnemyState.DisplayName;
		EnemyView.PositionIndex = EnemyState.PositionIndex;
		EnemyView.MaxHP = EnemyState.MaxHP;
		EnemyView.CurrentHP = EnemyState.CurrentHP;
		EnemyView.CurrentShield = EnemyState.CurrentShield;
		EnemyView.MaxBreakValue = EnemyState.MaxBreakValue;
		EnemyView.CurrentBreakValue = EnemyState.CurrentBreakValue;
		EnemyView.CurrentInitiative = EnemyState.CurrentInitiative;
		EnemyView.CurrentPhaseTag = EnemyState.CurrentPhaseTag;
		EnemyView.CurrentIntentId = EnemyState.CurrentIntentId;
		EnemyView.PhaseProgress = BuildPhaseProgress(EnemyState);
		EnemyView.IntentText = EnemyState.CurrentIntentText;
		EnemyView.bActedThisRound = EnemyState.bActedThisRound;
		Snapshot.Enemies.Add(MoveTemp(EnemyView));
	}

	StatusService.BuildStatusSnapshotData(State, Snapshot.CharacterStatuses, Snapshot.TeamStatuses, Snapshot.Statuses);
	CardService.BuildHandCardViews(State, Snapshot.HandCards);

	return Snapshot;
}
