#include "Resolver/FinalBattleResolver.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalEnemyIntentService.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalBattleResolver, Log, All);

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));

struct FFinalEffectExecutionSummary
{
	int32 TotalDamageToEnemies = 0;
	int32 TotalDamageToTeam = 0;
	int32 TotalTeamShieldGained = 0;
	int32 TotalEnemyShieldGained = 0;
	int32 TotalCardsDrawn = 0;
	int32 ResolvedEffectCount = 0;
};

void AppendBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event)
{
	FFinalBattleEvent EventToAppend = Event;
	EventToAppend.EventSequence = ++State.LastEventSequence;
	EventToAppend.BattleId = State.BattleId;
	EventToAppend.Round = EventToAppend.Round > 0 ? EventToAppend.Round : State.CurrentRound;
	EventToAppend.bBattleEnded = State.bBattleEnded;
	EventToAppend.bPlayerVictory = State.bPlayerVictory;
	State.BattleLogEntries.Add(MoveTemp(EventToAppend));
}

FFinalBattleEvent FinalizeBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event)
{
	AppendBattleEvent(State, Event);
	return State.BattleLogEntries.Last();
}

FFinalBattleCardInstance* FindCardInstance(FFinalBattleState& State, const FGuid& CardInstanceId)
{
	return State.CardInstances.FindByPredicate(
		[&CardInstanceId](const FFinalBattleCardInstance& Candidate)
		{
			return Candidate.CardInstanceId == CardInstanceId;
		});
}

const FFinalBattleCharacterState* FindCharacterState(const FFinalBattleState& State, const FName RuntimeUnitId)
{
	return State.Characters.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

FFinalBattleEnemyState* FindEnemyState(FFinalBattleState& State, const FName RuntimeUnitId)
{
	return State.Enemies.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

FFinalBattleEnemyState* FindFirstAliveEnemy(FFinalBattleState& State)
{
	return State.Enemies.FindByPredicate(
		[](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.CurrentHP > 0;
		});
}

bool AreAllEnemiesDefeated(const FFinalBattleState& State)
{
	if (State.Enemies.Num() == 0)
	{
		return false;
	}

	for (const FFinalBattleEnemyState& EnemyState : State.Enemies)
	{
		if (EnemyState.CurrentHP > 0)
		{
			return false;
		}
	}

	return true;
}

int32 ResolveAwakenThreshold(const FFinalBattleCharacterState& CharacterState, const UFinalBattleRuleConfig* RuleConfig)
{
	if (RuleConfig == nullptr)
	{
		return 0;
	}

	if (const int32* DirectThreshold = RuleConfig->AwakenThresholdByCollapseCount.Find(CharacterState.CollapseCount))
	{
		return *DirectThreshold;
	}

	if (RuleConfig->AwakenThresholdByCollapseCount.Num() == 0)
	{
		return 0;
	}

	int32 FallbackThreshold = 0;
	int32 FallbackCollapseCount = MIN_int32;
	for (const TPair<int32, int32>& Entry : RuleConfig->AwakenThresholdByCollapseCount)
	{
		if (Entry.Key <= CharacterState.CollapseCount && Entry.Key >= FallbackCollapseCount)
		{
			FallbackCollapseCount = Entry.Key;
			FallbackThreshold = Entry.Value;
		}
	}

	if (FallbackCollapseCount != MIN_int32)
	{
		return FallbackThreshold;
	}

	int32 LowestCollapseCount = MAX_int32;
	for (const TPair<int32, int32>& Entry : RuleConfig->AwakenThresholdByCollapseCount)
	{
		if (Entry.Key < LowestCollapseCount)
		{
			LowestCollapseCount = Entry.Key;
			FallbackThreshold = Entry.Value;
		}
	}

	return FallbackThreshold;
}

bool CanActivateUltimate(const FFinalBattleState& State, const FFinalBattleCharacterState& CharacterState)
{
	return CharacterState.UltimateDefinition != nullptr
		&& !CharacterState.bCollapsed
		&& CharacterState.UltimateId.IsValid()
		&& State.CurrentEP >= CharacterState.UltimateCostEP;
}

void DrawCards(FFinalBattleState& State, const int32 DrawCount)
{
	for (int32 DrawIndex = 0; DrawIndex < DrawCount; ++DrawIndex)
	{
		if (State.DeckState.DrawPileCardInstanceIds.Num() == 0)
		{
			if (State.DeckState.DiscardPileCardInstanceIds.Num() == 0)
			{
				return;
			}

			State.DeckState.DrawPileCardInstanceIds.Append(State.DeckState.DiscardPileCardInstanceIds);
			State.DeckState.DiscardPileCardInstanceIds.Reset();
		}

		const FGuid DrawnCardId = State.DeckState.DrawPileCardInstanceIds[0];
		State.DeckState.DrawPileCardInstanceIds.RemoveAt(0);
		State.DeckState.HandCardInstanceIds.Add(DrawnCardId);
	}
}

void MarkBattleResolved(FFinalBattleState& State, const bool bPlayerVictory)
{
	State.bBattleEnded = true;
	State.bPlayerVictory = bPlayerVictory;
}

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}

void RefreshEnemyIntentState(FFinalBattleEnemyState& EnemyState, const int32 PreviewRound)
{
	GetEnemyIntentService().RefreshIntent(EnemyState, PreviewRound);
}

void RefreshEnemyIntentState(FFinalBattleState& State, FFinalBattleEnemyState& EnemyState, const int32 PreviewRound, const bool bEmitPhaseChangeEvent)
{
	const FName PreviousPhaseTag = EnemyState.CurrentPhaseTag;
	GetEnemyIntentService().RefreshIntent(EnemyState, PreviewRound);

	if (!bEmitPhaseChangeEvent
		|| EnemyState.CurrentHP <= 0
		|| PreviousPhaseTag == EnemyState.CurrentPhaseTag
		|| EnemyState.CurrentPhaseTag == NAME_None)
	{
		return;
	}

	FFinalBattleEvent PhaseChangedEvent;
	PhaseChangedEvent.EventType = EFinalBattleEventType::PhaseChanged;
	PhaseChangedEvent.SourceUnitId = EnemyState.RuntimeUnitId;
	PhaseChangedEvent.RelatedTag = EnemyState.CurrentPhaseTag;
	PhaseChangedEvent.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "EnemyPhaseChanged", "{0} shifted from {1} to {2}."),
		EnemyState.DisplayName.IsEmpty() ? FText::FromName(EnemyState.RuntimeUnitId) : EnemyState.DisplayName,
		PreviousPhaseTag == NAME_None ? FText::FromString(TEXT("none")) : FText::FromName(PreviousPhaseTag),
		FText::FromName(EnemyState.CurrentPhaseTag));
	AppendBattleEvent(State, PhaseChangedEvent);
}

void AdvanceEnemyIntentState(FFinalBattleEnemyState& EnemyState, const int32 CurrentRound)
{
	GetEnemyIntentService().CommitCurrentIntentExecution(EnemyState, CurrentRound);
	GetEnemyIntentService().RefreshIntent(EnemyState, CurrentRound + 1);
}

bool HasSupportedEffectList(const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects)
{
	for (const UFinalBattleEffectDefinition* EffectDefinition : Effects)
	{
		if (Cast<UFinalBattleEffectDamage>(EffectDefinition)
			|| Cast<UFinalBattleEffectGainShield>(EffectDefinition)
			|| Cast<UFinalBattleEffectDrawCards>(EffectDefinition))
		{
			return true;
		}
	}

	return false;
}

bool HasSupportedEffect(const UFinalCardDefinition* CardDefinition)
{
	return CardDefinition != nullptr && HasSupportedEffectList(CardDefinition->Effects);
}

bool HasSupportedEffect(const UFinalUltimateDefinition* UltimateDefinition)
{
	return UltimateDefinition != nullptr && HasSupportedEffectList(UltimateDefinition->Effects);
}

int32 ResolveSourceStatValue(const FFinalBattleCharacterState* SourceCharacterState, const FFinalBattleEnemyState* SourceEnemyState, const EFinalBattleSourceStat SourceStat)
{
	switch (SourceStat)
	{
	case EFinalBattleSourceStat::Attack:
		return SourceCharacterState ? SourceCharacterState->RuntimeAttack : 0;

	case EFinalBattleSourceStat::Defense:
		return SourceCharacterState ? SourceCharacterState->RuntimeDefense : 0;

	case EFinalBattleSourceStat::BaseDamagePower:
		return SourceEnemyState ? SourceEnemyState->RuntimeDamagePower : 0;

	default:
		return 0;
	}
}

int32 ResolveScalarValue(const FFinalBattleScalarValue& Scalar, const FFinalBattleCharacterState* SourceCharacterState, const FFinalBattleEnemyState* SourceEnemyState)
{
	float ResultValue = Scalar.FlatBonus;

	switch (Scalar.ScaleMode)
	{
	case EFinalBattleScalarMode::Flat:
		ResultValue += Scalar.BaseValue;
		break;

	case EFinalBattleScalarMode::SourceStatMultiplier:
		ResultValue += static_cast<float>(ResolveSourceStatValue(SourceCharacterState, SourceEnemyState, Scalar.SourceStat)) * Scalar.BaseValue;
		break;

	default:
		break;
	}

	if (Scalar.Cap > 0.0f)
	{
		ResultValue = FMath::Min(ResultValue, Scalar.Cap);
	}

	return FMath::Max(FMath::RoundToInt(ResultValue), 0);
}

FFinalBattleEnemyState* ResolvePrimaryEnemyTarget(FFinalBattleState& State, const FFinalBattleCommand* Command, const EFinalBattleUnitTargetRule TargetRule)
{
	switch (TargetRule)
	{
	case EFinalBattleUnitTargetRule::SelectedEnemy:
		{
			FFinalBattleEnemyState* SelectedEnemyState = nullptr;
			if (Command && Command->TargetUnitId != NAME_None)
			{
				SelectedEnemyState = FindEnemyState(State, Command->TargetUnitId);
			}
			else if (State.CurrentTargetUnitId != NAME_None)
			{
				SelectedEnemyState = FindEnemyState(State, State.CurrentTargetUnitId);
			}

			return (SelectedEnemyState != nullptr && SelectedEnemyState->CurrentHP > 0)
				? SelectedEnemyState
				: FindFirstAliveEnemy(State);
		}

	case EFinalBattleUnitTargetRule::FirstAliveEnemy:
	case EFinalBattleUnitTargetRule::AllEnemies:
		return FindFirstAliveEnemy(State);

	default:
		return nullptr;
	}
}

void ApplyDamageToEnemy(FFinalBattleState& State, FFinalBattleEnemyState& EnemyState, const int32 DamageAmount)
{
	const int32 ShieldAbsorbed = FMath::Min(EnemyState.CurrentShield, FMath::Max(DamageAmount, 0));
	EnemyState.CurrentShield -= ShieldAbsorbed;
	const int32 HpDamage = FMath::Max(DamageAmount - ShieldAbsorbed, 0);
	EnemyState.CurrentHP = FMath::Max(0, EnemyState.CurrentHP - HpDamage);
	EnemyState.CurrentInitiative = FMath::Max(0, EnemyState.CurrentInitiative - 1);

	if (EnemyState.CurrentHP <= 0)
	{
		EnemyState.CurrentIntentText = FText::FromString(TEXT("Defeated"));
		if (State.CurrentTargetUnitId == EnemyState.RuntimeUnitId)
		{
			State.CurrentTargetUnitId = NAME_None;
			if (FFinalBattleEnemyState* NextTarget = FindFirstAliveEnemy(State))
			{
				State.CurrentTargetUnitId = NextTarget->RuntimeUnitId;
			}
		}
		return;
	}

	RefreshEnemyIntentState(State, EnemyState, State.CurrentRound, true);
}

int32 ApplyTeamIncomingDamage(FFinalBattleState& State, const int32 TotalIncomingDamage)
{
	const int32 ShieldAbsorbed = FMath::Min(State.TeamShield, FMath::Max(TotalIncomingDamage, 0));
	State.TeamShield -= ShieldAbsorbed;

	const int32 HpDamage = FMath::Max(TotalIncomingDamage - ShieldAbsorbed, 0);
	State.TeamCurrentHP = FMath::Max(0, State.TeamCurrentHP - HpDamage);
	return HpDamage;
}

bool ExecuteEffectList(
	FFinalBattleState& State,
	const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	FFinalEffectExecutionSummary& Summary)
{
	for (UFinalBattleEffectDefinition* EffectDefinition : Effects)
	{
		if (EffectDefinition == nullptr)
		{
			continue;
		}

		if (const UFinalBattleEffectDamage* DamageEffect = Cast<UFinalBattleEffectDamage>(EffectDefinition))
		{
			const int32 HitCount = FMath::Max(DamageEffect->HitCount, 1);
			const int32 DamagePerHit = ResolveScalarValue(DamageEffect->Scalar, SourceCharacterState, SourceEnemyState);
			if (DamagePerHit <= 0)
			{
				continue;
			}

			if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::TeamPlayer)
			{
				for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
				{
					Summary.TotalDamageToTeam += ApplyTeamIncomingDamage(State, DamagePerHit);
				}

				++Summary.ResolvedEffectCount;
				continue;
			}

			if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::AllEnemies)
			{
				for (FFinalBattleEnemyState& EnemyState : State.Enemies)
				{
					if (EnemyState.CurrentHP <= 0)
					{
						continue;
					}

					for (int32 HitIndex = 0; HitIndex < HitCount && EnemyState.CurrentHP > 0; ++HitIndex)
					{
						ApplyDamageToEnemy(State, EnemyState, DamagePerHit);
						Summary.TotalDamageToEnemies += DamagePerHit;
					}
				}

				++Summary.ResolvedEffectCount;
				continue;
			}

			FFinalBattleEnemyState* TargetEnemyState = nullptr;
			if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::Self && SourceEnemyState)
			{
				TargetEnemyState = SourceEnemyState;
			}
			else
			{
				TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, DamageEffect->UnitTargetRule);
			}

			if (TargetEnemyState == nullptr)
			{
				continue;
			}

			for (int32 HitIndex = 0; HitIndex < HitCount && TargetEnemyState->CurrentHP > 0; ++HitIndex)
			{
				ApplyDamageToEnemy(State, *TargetEnemyState, DamagePerHit);
				Summary.TotalDamageToEnemies += DamagePerHit;
			}

			++Summary.ResolvedEffectCount;
			continue;
		}

		if (const UFinalBattleEffectGainShield* ShieldEffect = Cast<UFinalBattleEffectGainShield>(EffectDefinition))
		{
			const int32 ShieldAmount = ResolveScalarValue(ShieldEffect->Scalar, SourceCharacterState, SourceEnemyState);
			if (ShieldAmount <= 0)
			{
				continue;
			}

			switch (ShieldEffect->UnitTargetRule)
			{
			case EFinalBattleUnitTargetRule::TeamPlayer:
				State.TeamShield += ShieldAmount;
				Summary.TotalTeamShieldGained += ShieldAmount;
				break;

			case EFinalBattleUnitTargetRule::Self:
				if (SourceEnemyState)
				{
					SourceEnemyState->CurrentShield += ShieldAmount;
					Summary.TotalEnemyShieldGained += ShieldAmount;
				}
				else
				{
					State.TeamShield += ShieldAmount;
					Summary.TotalTeamShieldGained += ShieldAmount;
				}
				break;

			case EFinalBattleUnitTargetRule::AllEnemies:
				for (FFinalBattleEnemyState& EnemyState : State.Enemies)
				{
					if (EnemyState.CurrentHP <= 0)
					{
						continue;
					}

					EnemyState.CurrentShield += ShieldAmount;
					Summary.TotalEnemyShieldGained += ShieldAmount;
				}
				break;

			default:
				break;
			}

			++Summary.ResolvedEffectCount;
			continue;
		}

		if (const UFinalBattleEffectDrawCards* DrawCardsEffect = Cast<UFinalBattleEffectDrawCards>(EffectDefinition))
		{
			const int32 HandCountBeforeDraw = State.DeckState.HandCardInstanceIds.Num();
			DrawCards(State, FMath::Max(DrawCardsEffect->DrawCount, 0));
			Summary.TotalCardsDrawn += FMath::Max(State.DeckState.HandCardInstanceIds.Num() - HandCountBeforeDraw, 0);
			++Summary.ResolvedEffectCount;
		}
	}

	return Summary.ResolvedEffectCount > 0;
}
}

void FFinalBattleResolver::Initialize(FFinalBattleState& State, const UFinalBattleEncounterDefinition* EncounterDefinition, const UFinalBattleRuleConfig* RuleConfig, const FFinalBattleInitContext& InitContext) const
{
	State = FFinalBattleState{};
	State.BattleId = FGuid::NewGuid();
	State.CurrentRound = 1;
	State.CurrentAP = RuleConfig ? RuleConfig->InitialAP : 0;
	State.CurrentEP = 0;
	State.MaxEP = RuleConfig ? RuleConfig->MaxEP : 0;
	State.TeamCurrentHP = 0;
	State.TeamMaxHP = 0;
	TMap<FName, FName> TemplateToRuntimeUnitMap;

	if (EncounterDefinition)
	{
		State.EncounterId = EncounterDefinition->EncounterId;
		State.EncounterDisplayName = EncounterDefinition->DisplayName;
	}

	if (RuleConfig)
	{
		State.RuleConfigId = RuleConfig->RuleConfigId;
	}

	for (int32 Index = 0; Index < InitContext.PartyMembers.Num(); ++Index)
	{
		const FFinalBattleCharacterInitData& PartyEntry = InitContext.PartyMembers[Index];
		if (PartyEntry.CharacterDefinition == nullptr || !PartyEntry.CharacterDefinition->CharacterId.IsValid())
		{
			continue;
		}

		FFinalBattleCharacterState CharacterState;
		CharacterState.RuntimeUnitId = MakePlayerUnitId(Index);
		CharacterState.CharacterId = PartyEntry.CharacterDefinition->CharacterId;
		CharacterState.DisplayName = PartyEntry.CharacterDefinition->DisplayName;
		CharacterState.CurrentStress = PartyEntry.CurrentStress;
		CharacterState.StressCap = PartyEntry.CharacterDefinition->BaseStressCap;
		CharacterState.bCollapsed = PartyEntry.bCollapsed;
		CharacterState.CurrentAwakenCount = PartyEntry.CurrentAwakenCount;
		CharacterState.CollapseCount = PartyEntry.CollapseCount;
		CharacterState.CurrentAwakenThreshold = ResolveAwakenThreshold(CharacterState, RuleConfig);
		CharacterState.VitalShare = PartyEntry.CharacterDefinition->BaseVitalShare;
		CharacterState.RuntimeAttack = PartyEntry.CharacterDefinition->BaseAttack;
		CharacterState.RuntimeDefense = PartyEntry.CharacterDefinition->BaseDefense;
		CharacterState.RuntimeBreakRate = PartyEntry.CharacterDefinition->BaseBreakRate;
		CharacterState.UltimateId = PartyEntry.CharacterDefinition->UltimateId;
		if (PartyEntry.UltimateDefinition != nullptr)
		{
			CharacterState.UltimateDefinition = PartyEntry.UltimateDefinition;
			CharacterState.UltimateDisplayName = PartyEntry.UltimateDefinition->DisplayName;
			CharacterState.UltimateCostEP = PartyEntry.UltimateDefinition->BaseCostEP;
		}
		State.Characters.Add(MoveTemp(CharacterState));

		TemplateToRuntimeUnitMap.Add(PartyEntry.CharacterDefinition->CharacterId.Value, State.Characters.Last().RuntimeUnitId);

		if (!PartyEntry.bCollapsed)
		{
			State.TeamMaxHP += PartyEntry.CharacterDefinition->BaseVitalShare;
		}
	}

	State.TeamCurrentHP = InitContext.TeamCurrentHP > 0
		? FMath::Min(InitContext.TeamCurrentHP, State.TeamMaxHP)
		: State.TeamMaxHP;

	for (UFinalCardDefinition* CardDefinition : InitContext.DeckDefinitions)
	{
		if (CardDefinition == nullptr || !CardDefinition->CardId.IsValid())
		{
			continue;
		}

		FFinalBattleCardInstance CardInstance;
		CardInstance.CardInstanceId = FGuid::NewGuid();
		CardInstance.CardId = CardDefinition->CardId;
		CardInstance.RuntimeCostAP = CardDefinition->BaseCostAP;
		CardInstance.RuntimeKeywords = CardDefinition->Keywords;
		CardInstance.SourceDefinition = CardDefinition;

		if (const FName* RuntimeOwnerUnitId = TemplateToRuntimeUnitMap.Find(CardDefinition->OwnerUnitId))
		{
			CardInstance.RuntimeOwnerUnitId = *RuntimeOwnerUnitId;
		}
		else
		{
			CardInstance.RuntimeOwnerUnitId = CardDefinition->OwnerUnitId;
		}

		State.CardInstances.Add(CardInstance);
		State.DeckState.DrawPileCardInstanceIds.Add(CardInstance.CardInstanceId);
	}

	const int32 InitialHandSize = RuleConfig ? FMath::Max(RuleConfig->InitialHandSize, 0) : 0;
	const int32 CardsToDraw = FMath::Min(InitialHandSize, State.DeckState.DrawPileCardInstanceIds.Num());
	for (int32 DrawIndex = 0; DrawIndex < CardsToDraw; ++DrawIndex)
	{
		const FGuid DrawnCardId = State.DeckState.DrawPileCardInstanceIds[0];
		State.DeckState.DrawPileCardInstanceIds.RemoveAt(0);
		State.DeckState.HandCardInstanceIds.Add(DrawnCardId);
	}

	if (!EncounterDefinition)
	{
		return;
	}

	for (int32 Index = 0; Index < EncounterDefinition->EnemyRoster.Num(); ++Index)
	{
		const FFinalEnemyRosterEntry& Entry = EncounterDefinition->EnemyRoster[Index];
		UFinalEnemyDefinition* LoadedEnemy = Entry.EnemyDefinition.LoadSynchronous();

		FFinalBattleEnemyState EnemyState;
		EnemyState.RuntimeUnitId = MakeEnemyUnitId(Index);
		EnemyState.PositionIndex = Entry.PositionIndex;
		EnemyState.SpawnWave = Entry.SpawnWave;

		if (LoadedEnemy)
		{
			EnemyState.EnemyId = LoadedEnemy->EnemyId;
			EnemyState.DisplayName = LoadedEnemy->DisplayName;
			EnemyState.RoleTags = LoadedEnemy->RoleTags;
			EnemyState.MaxHP = LoadedEnemy->MaxHP;
			EnemyState.CurrentHP = LoadedEnemy->MaxHP;
			EnemyState.CurrentShield = 0;
			EnemyState.MaxBreakValue = LoadedEnemy->MaxBreakValue;
			EnemyState.CurrentBreakValue = LoadedEnemy->MaxBreakValue;
			EnemyState.CurrentInitiative = LoadedEnemy->InitialInitiativeValue;
			EnemyState.RuntimeDamagePower = LoadedEnemy->BaseDamagePower;
			EnemyState.IntentSelectRule = LoadedEnemy->IntentSelectRule;
			EnemyState.PhaseSequence = LoadedEnemy->PhaseSequence;
			EnemyState.PhaseSequence.Sort(
				[](const FFinalEnemyPhaseDefinition& Left, const FFinalEnemyPhaseDefinition& Right)
				{
					return Left.MaxHpPercent > Right.MaxHpPercent;
				});

			for (const TSoftObjectPtr<UFinalEnemyIntentDefinition>& IntentReference : LoadedEnemy->IntentPool)
			{
				if (UFinalEnemyIntentDefinition* LoadedIntent = IntentReference.LoadSynchronous())
				{
					FFinalBattleEnemyIntentRuntimeState& IntentState = EnemyState.IntentRuntimeStates.AddDefaulted_GetRef();
					IntentState.Definition = LoadedIntent;
					IntentState.IntentId = LoadedIntent->IntentId;
				}
			}

			RefreshEnemyIntentState(EnemyState, State.CurrentRound);
		}
		else
		{
			EnemyState.DisplayName = FText::FromString(TEXT("Missing Enemy Definition"));
		}

		State.Enemies.Add(MoveTemp(EnemyState));
	}

	if (FFinalBattleEnemyState* DefaultTargetEnemy = FindFirstAliveEnemy(State))
	{
		State.CurrentTargetUnitId = DefaultTargetEnemy->RuntimeUnitId;
	}

	FFinalBattleEvent SessionStartedEvent;
	SessionStartedEvent.EventType = EFinalBattleEventType::SessionStarted;
	SessionStartedEvent.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "SessionStarted", "Battle started: {0}."),
		State.EncounterDisplayName.IsEmpty() ? FText::FromName(State.EncounterId.Value) : State.EncounterDisplayName);
	FinalizeBattleEvent(State, SessionStartedEvent);

	UE_LOG(LogFinalBattleResolver, Log, TEXT("Initialized battle session with %d enemy entries."), State.Enemies.Num());
}

FName FFinalBattleResolver::MakePlayerUnitId(int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_player_%d"), Index + 1));
}

FFinalBattleEvent FFinalBattleResolver::ExecuteCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	if (State.bBattleEnded)
	{
		Event.EventType = EFinalBattleEventType::CommandRejected;
		Event.Message = FText::FromString(TEXT("Battle is already resolved."));
		return FinalizeBattleEvent(State, Event);
	}

	switch (Command.CommandType)
	{
	case EFinalBattleCommandType::PlayCard:
		{
			FFinalBattleCardInstance* CardInstance = FindCardInstance(State, Command.CardInstanceId);
			if (CardInstance == nullptr)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.CardInstanceId = Command.CardInstanceId;
				Event.Message = FText::FromString(TEXT("Card instance was not found."));
				return FinalizeBattleEvent(State, Event);
			}

			if (CardInstance->SourceDefinition == nullptr)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.CardInstanceId = Command.CardInstanceId;
				Event.CardId = CardInstance->CardId;
				Event.Message = FText::FromString(TEXT("Card definition is missing for the selected card."));
				return FinalizeBattleEvent(State, Event);
			}

			if (!State.DeckState.HandCardInstanceIds.Contains(Command.CardInstanceId))
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.CardInstanceId = Command.CardInstanceId;
				Event.CardId = CardInstance->CardId;
				Event.Message = FText::FromString(TEXT("Card instance is not in hand."));
				return FinalizeBattleEvent(State, Event);
			}

			if (State.CurrentAP < CardInstance->RuntimeCostAP)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.CardInstanceId = Command.CardInstanceId;
				Event.CardId = CardInstance->CardId;
				Event.PrimaryValue = CardInstance->RuntimeCostAP;
				Event.SecondaryValue = State.CurrentAP;
				Event.Message = FText::FromString(TEXT("Not enough AP to play the selected card."));
				return FinalizeBattleEvent(State, Event);
			}

			if (!HasSupportedEffect(CardInstance->SourceDefinition))
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.CardInstanceId = Command.CardInstanceId;
				Event.CardId = CardInstance->CardId;
				Event.Message = FText::FromString(TEXT("Selected card has no supported effects."));
				return FinalizeBattleEvent(State, Event);
			}

			const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, CardInstance->RuntimeOwnerUnitId);

			State.CurrentAP -= CardInstance->RuntimeCostAP;
			State.CurrentEP = RuleConfig ? FMath::Min(State.CurrentEP + RuleConfig->BaseCardEpGain, RuleConfig->MaxEP) : State.CurrentEP;
			State.DeckState.HandCardInstanceIds.RemoveSingle(Command.CardInstanceId);
			State.DeckState.DiscardPileCardInstanceIds.Add(Command.CardInstanceId);

			FFinalEffectExecutionSummary Summary;
			ExecuteEffectList(State, CardInstance->SourceDefinition->Effects, &Command, OwnerCharacterState, nullptr, Summary);

			Event.EventType = EFinalBattleEventType::CardResolved;
			Event.SourceUnitId = CardInstance->RuntimeOwnerUnitId;
			Event.TargetUnitId = Command.TargetUnitId != NAME_None ? Command.TargetUnitId : State.CurrentTargetUnitId;
			Event.CardInstanceId = CardInstance->CardInstanceId;
			Event.CardId = CardInstance->CardId;
			Event.PrimaryValue = Summary.TotalDamageToEnemies;
			Event.SecondaryValue = Summary.TotalCardsDrawn;

			if (AreAllEnemiesDefeated(State))
			{
				MarkBattleResolved(State, true);
				Event.Message = FText::Format(
					NSLOCTEXT("FinalBattleResolver", "CardPlayVictory", "Resolved {0} effects. Damage {1}, Shield {2}, Draw {3}. Battle won."),
					FText::AsNumber(Summary.ResolvedEffectCount),
					FText::AsNumber(Summary.TotalDamageToEnemies),
					FText::AsNumber(Summary.TotalTeamShieldGained),
					FText::AsNumber(Summary.TotalCardsDrawn));
				return FinalizeBattleEvent(State, Event);
			}

			Event.Message = FText::Format(
				NSLOCTEXT("FinalBattleResolver", "CardPlayAccepted", "Resolved {0} effects. Damage {1}, Shield {2}, Draw {3}."),
				FText::AsNumber(Summary.ResolvedEffectCount),
				FText::AsNumber(Summary.TotalDamageToEnemies),
				FText::AsNumber(Summary.TotalTeamShieldGained),
				FText::AsNumber(Summary.TotalCardsDrawn));
			break;
		}

	case EFinalBattleCommandType::PlayUltimate:
		{
			const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, Command.UltimateOwnerUnitId);
			if (OwnerCharacterState == nullptr)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.SourceUnitId = Command.UltimateOwnerUnitId;
				Event.Message = FText::FromString(TEXT("Ultimate owner was not found."));
				return FinalizeBattleEvent(State, Event);
			}

			Event.SourceUnitId = OwnerCharacterState->RuntimeUnitId;
			Event.UltimateId = OwnerCharacterState->UltimateId;

			if (OwnerCharacterState->bCollapsed)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.Message = FText::FromString(TEXT("Collapsed characters cannot use ultimates."));
				return FinalizeBattleEvent(State, Event);
			}

			if (OwnerCharacterState->UltimateDefinition == nullptr || !HasSupportedEffect(OwnerCharacterState->UltimateDefinition))
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.Message = FText::FromString(TEXT("Ultimate definition is unavailable or has no supported effects."));
				return FinalizeBattleEvent(State, Event);
			}

			if (State.CurrentEP < OwnerCharacterState->UltimateCostEP)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.PrimaryValue = OwnerCharacterState->UltimateCostEP;
				Event.SecondaryValue = State.CurrentEP;
				Event.Message = FText::FromString(TEXT("Not enough EP to use the selected ultimate."));
				return FinalizeBattleEvent(State, Event);
			}

			State.CurrentEP -= OwnerCharacterState->UltimateCostEP;

			FFinalEffectExecutionSummary Summary;
			ExecuteEffectList(State, OwnerCharacterState->UltimateDefinition->Effects, &Command, OwnerCharacterState, nullptr, Summary);

			Event.EventType = EFinalBattleEventType::UltimateResolved;
			Event.TargetUnitId = Command.TargetUnitId != NAME_None ? Command.TargetUnitId : State.CurrentTargetUnitId;
			Event.PrimaryValue = Summary.TotalDamageToEnemies;
			Event.SecondaryValue = Summary.TotalTeamShieldGained;

			if (AreAllEnemiesDefeated(State))
			{
				MarkBattleResolved(State, true);
				Event.Message = FText::Format(
					NSLOCTEXT("FinalBattleResolver", "UltimateVictory", "Ultimate resolved. Damage {0}, Shield {1}. Battle won."),
					FText::AsNumber(Summary.TotalDamageToEnemies),
					FText::AsNumber(Summary.TotalTeamShieldGained));
				return FinalizeBattleEvent(State, Event);
			}

			Event.Message = FText::Format(
				NSLOCTEXT("FinalBattleResolver", "UltimateResolved", "Ultimate resolved. Damage {0}, Shield {1}."),
				FText::AsNumber(Summary.TotalDamageToEnemies),
				FText::AsNumber(Summary.TotalTeamShieldGained));
			break;
		}

	case EFinalBattleCommandType::EndTurn:
		{
			FFinalEffectExecutionSummary Summary;
			for (FFinalBattleEnemyState& EnemyState : State.Enemies)
			{
				if (EnemyState.CurrentHP <= 0)
				{
					continue;
				}

				const int32 TeamDamageBeforeAction = Summary.TotalDamageToTeam;
				const int32 EnemyShieldBeforeAction = Summary.TotalEnemyShieldGained;

				if (EnemyState.CurrentIntentDefinition && HasSupportedEffectList(EnemyState.CurrentIntentDefinition->Effects))
				{
					ExecuteEffectList(State, EnemyState.CurrentIntentDefinition->Effects, nullptr, nullptr, &EnemyState, Summary);
				}
				else
				{
					Summary.TotalDamageToTeam += ApplyTeamIncomingDamage(State, FMath::Max(EnemyState.RuntimeDamagePower, 0));
				}

				EnemyState.bActedThisRound = true;

				FFinalBattleEvent EnemyActionEvent;
				EnemyActionEvent.EventType = EFinalBattleEventType::EnemyActed;
				EnemyActionEvent.SourceUnitId = EnemyState.RuntimeUnitId;
				EnemyActionEvent.TargetUnitId = TeamPlayerUnitId;
				EnemyActionEvent.RelatedTag = EnemyState.CurrentIntentId;
				EnemyActionEvent.PrimaryValue = Summary.TotalDamageToTeam - TeamDamageBeforeAction;
				EnemyActionEvent.SecondaryValue = Summary.TotalEnemyShieldGained - EnemyShieldBeforeAction;
				EnemyActionEvent.Message = FText::Format(
					NSLOCTEXT("FinalBattleResolver", "EnemyActed", "{0} resolved intent {1}."),
					EnemyState.DisplayName.IsEmpty() ? FText::FromName(EnemyState.RuntimeUnitId) : EnemyState.DisplayName,
					EnemyState.CurrentIntentText.IsEmpty() ? FText::FromString(TEXT("Attack")) : EnemyState.CurrentIntentText);
				AppendBattleEvent(State, EnemyActionEvent);

				AdvanceEnemyIntentState(EnemyState, State.CurrentRound);
			}

			State.CurrentEP = RuleConfig ? FMath::Min(State.CurrentEP + RuleConfig->EndTurnEpGain, RuleConfig->MaxEP) : State.CurrentEP;

			if (State.TeamCurrentHP <= 0)
			{
				MarkBattleResolved(State, false);
				Event.EventType = EFinalBattleEventType::BattleResolved;
				Event.TargetUnitId = TeamPlayerUnitId;
				Event.PrimaryValue = Summary.TotalDamageToTeam;
				Event.SecondaryValue = Summary.TotalEnemyShieldGained;
				Event.Message = FText::Format(
					NSLOCTEXT("FinalBattleResolver", "EndTurnDefeat", "Enemies resolved {0} effects. Team damage {1}, enemy shield {2}. Battle lost."),
					FText::AsNumber(Summary.ResolvedEffectCount),
					FText::AsNumber(Summary.TotalDamageToTeam),
					FText::AsNumber(Summary.TotalEnemyShieldGained));
				return FinalizeBattleEvent(State, Event);
			}

			++State.CurrentRound;
			State.CurrentAP = RuleConfig ? RuleConfig->InitialAP : 0;

			for (FFinalBattleEnemyState& EnemyState : State.Enemies)
			{
				EnemyState.bActedThisRound = false;
			}

			const int32 TargetHandSize = RuleConfig ? FMath::Max(RuleConfig->InitialHandSize, 0) : State.DeckState.HandCardInstanceIds.Num();
			const int32 CardsToDraw = FMath::Max(TargetHandSize - State.DeckState.HandCardInstanceIds.Num(), 0);
			DrawCards(State, CardsToDraw);

			Event.EventType = EFinalBattleEventType::TurnTransition;
			Event.TargetUnitId = TeamPlayerUnitId;
			Event.PrimaryValue = Summary.TotalDamageToTeam;
			Event.SecondaryValue = Summary.TotalEnemyShieldGained;
			Event.Message = FText::Format(
				NSLOCTEXT("FinalBattleResolver", "EndTurnAdvanced", "Turn advanced. Enemies resolved {0} effects. Team damage {1}, enemy shield {2}, team shield now {3}."),
				FText::AsNumber(Summary.ResolvedEffectCount),
				FText::AsNumber(Summary.TotalDamageToTeam),
				FText::AsNumber(Summary.TotalEnemyShieldGained),
				FText::AsNumber(State.TeamShield));
			Event.Round = State.CurrentRound;
			break;
		}

	case EFinalBattleCommandType::SelectTarget:
		{
			FFinalBattleEnemyState* SelectedEnemy = FindEnemyState(State, Command.TargetUnitId);
			if (SelectedEnemy == nullptr || SelectedEnemy->CurrentHP <= 0)
			{
				Event.EventType = EFinalBattleEventType::CommandRejected;
				Event.TargetUnitId = Command.TargetUnitId;
				Event.Message = FText::FromString(TEXT("Target is not a valid living enemy."));
				return FinalizeBattleEvent(State, Event);
			}

			State.CurrentTargetUnitId = Command.TargetUnitId;
			Event.EventType = EFinalBattleEventType::TargetChanged;
			Event.TargetUnitId = Command.TargetUnitId;
			Event.Message = FText::FromString(TEXT("Target updated."));
			break;
		}

	default:
		Event.EventType = EFinalBattleEventType::CommandRejected;
		Event.Message = FText::FromString(TEXT("Unsupported command."));
		break;
	}

	return FinalizeBattleEvent(State, Event);
}

FFinalBattleSnapshot FFinalBattleResolver::BuildSnapshot(const FFinalBattleState& State) const
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
		Snapshot.CharacterUltimates.Add(MoveTemp(UltimateView));
	}

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
		EnemyView.IntentText = EnemyState.CurrentIntentText;
		EnemyView.bActedThisRound = EnemyState.bActedThisRound;
		Snapshot.Enemies.Add(MoveTemp(EnemyView));
	}

	for (const FFinalBattleStatusInstance& StatusInstance : State.StatusInstances)
	{
		FFinalBattleStatusViewData StatusView;
		StatusView.StatusInstanceId = StatusInstance.StatusInstanceId;
		StatusView.StatusId = StatusInstance.StatusId;
		StatusView.OwnerUnitId = StatusInstance.OwnerUnitId;
		StatusView.SourceUnitId = StatusInstance.SourceUnitId;
		StatusView.DisplayName = FText::FromName(StatusInstance.StatusId.Value);
		StatusView.CurrentStacks = StatusInstance.CurrentStacks;
		StatusView.RemainingDuration = StatusInstance.RemainingDuration;
		Snapshot.Statuses.Add(MoveTemp(StatusView));
	}

	for (const FGuid& CardInstanceId : State.DeckState.HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* CardInstance = State.CardInstances.FindByPredicate(
			[&CardInstanceId](const FFinalBattleCardInstance& Candidate)
			{
				return Candidate.CardInstanceId == CardInstanceId;
			});

		if (!CardInstance)
		{
			continue;
		}

		FFinalBattleCardViewData CardView;
		CardView.CardInstanceId = CardInstance->CardInstanceId;
		CardView.CardId = CardInstance->CardId;
		CardView.RuntimeOwnerUnitId = CardInstance->RuntimeOwnerUnitId;
		CardView.DisplayName = CardInstance->SourceDefinition != nullptr ? CardInstance->SourceDefinition->DisplayName : FText::FromName(CardInstance->CardId.Value);
		CardView.CardType = CardInstance->SourceDefinition != nullptr ? CardInstance->SourceDefinition->CardType : EFinalCardType::Attack;
		CardView.RuntimeCostAP = CardInstance->RuntimeCostAP;
		CardView.RuntimeKeywords = CardInstance->RuntimeKeywords;
		CardView.bRetained = CardInstance->bRetained;
		if (const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, CardInstance->RuntimeOwnerUnitId))
		{
			CardView.bCollapsedCard = OwnerCharacterState->bCollapsed;
		}
		Snapshot.HandCards.Add(MoveTemp(CardView));
	}

	return Snapshot;
}

FName FFinalBattleResolver::MakeEnemyUnitId(int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_enemy_%d"), Index + 1));
}
