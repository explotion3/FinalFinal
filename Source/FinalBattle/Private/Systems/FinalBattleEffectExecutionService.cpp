#include "Systems/FinalBattleEffectExecutionService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectApplyPassive.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainAP.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "Battle/Effects/FinalBattleEffectHeal.h"
#include "Battle/Effects/FinalBattleEffectMoveCards.h"
#include "Battle/Effects/FinalBattleEffectRemoveStatus.h"
#include "Commands/FinalBattleCommand.h"
#include "Events/FinalBattleEvent.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEventService.h"
#include "Systems/FinalBattlePassiveService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"
#include "Systems/FinalEnemyIntentService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName PassiveAppliedEffectReasonTag(TEXT("passive.applied.effect"));

const FFinalBattleCardService& GetCardService();
const FFinalBattleConditionService& GetConditionService();
const FFinalBattleEffectExecutionService& GetEffectExecutionService();
const FFinalBattleEventService& GetEventService();
const FFinalBattleTriggerService& GetTriggerService();
const FFinalBattleResourceService& GetResourceService();
const FFinalBattleStatusService& GetStatusService();
const FFinalBattlePassiveService& GetPassiveService();
const FFinalEnemyIntentService& GetEnemyIntentService();
bool ExecuteEffectListInternal(
	FFinalBattleState& State,
	const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
	const FFinalBattleCommand* Command,
	const UFinalCardDefinition* SourceCardDefinition,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FFinalBattleUnitService& UnitService,
	FFinalBattleEffectExecutionSummary& Summary);

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}

const FFinalBattleCardService& GetCardService()
{
	static const FFinalBattleCardService CardService;
	return CardService;
}

const FFinalBattleConditionService& GetConditionService()
{
	static const FFinalBattleConditionService ConditionService;
	return ConditionService;
}

const FFinalBattleEffectExecutionService& GetEffectExecutionService()
{
	static const FFinalBattleEffectExecutionService EffectExecutionService;
	return EffectExecutionService;
}

const FFinalBattleEventService& GetEventService()
{
	static const FFinalBattleEventService EventService;
	return EventService;
}

const FFinalBattleTriggerService& GetTriggerService()
{
	static const FFinalBattleTriggerService TriggerService;
	return TriggerService;
}

const FFinalBattleResourceService& GetResourceService()
{
	static const FFinalBattleResourceService ResourceService;
	return ResourceService;
}

const FFinalBattleStatusService& GetStatusService()
{
	static const FFinalBattleStatusService StatusService;
	return StatusService;
}

const FFinalBattlePassiveService& GetPassiveService()
{
	static const FFinalBattlePassiveService PassiveService;
	return PassiveService;
}

FFinalBattleEvent BuildPassiveAppliedEvent(
	const FFinalBattlePassiveApplyResult& ApplyResult,
	const FName ReasonTag)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::PassiveApplied;
	Event.PassiveInstanceId = ApplyResult.PassiveInstanceId;
	Event.PassiveId = ApplyResult.PassiveId;
	Event.SourceUnitId = ApplyResult.SourceUnitId;
	Event.TargetUnitId = ApplyResult.OwnerUnitId;
	Event.ReasonTag = ReasonTag;
	Event.PrimaryValue = ApplyResult.CurrentStacks;
	Event.SecondaryValue = ApplyResult.RemainingDuration;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleEffectExecutionService", "PassiveAppliedMessage", "获得被动：{0}。"),
		ApplyResult.DisplayName);
	return Event;
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
	GetEventService().AppendBattleEvent(State, PhaseChangedEvent);
}

bool HasSupportedEffectListInternal(const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects)
{
	for (const UFinalBattleEffectDefinition* EffectDefinition : Effects)
	{
		if (Cast<UFinalBattleEffectDamage>(EffectDefinition)
			|| Cast<UFinalBattleEffectHeal>(EffectDefinition)
			|| Cast<UFinalBattleEffectApplyPassive>(EffectDefinition)
			|| Cast<UFinalBattleEffectApplyStatus>(EffectDefinition)
			|| Cast<UFinalBattleEffectRemoveStatus>(EffectDefinition)
			|| Cast<UFinalBattleEffectGainShield>(EffectDefinition)
			|| Cast<UFinalBattleEffectDrawCards>(EffectDefinition)
			|| Cast<UFinalBattleEffectGainAP>(EffectDefinition)
			|| Cast<UFinalBattleEffectBonusBreak>(EffectDefinition)
			|| Cast<UFinalBattleEffectGenerateCard>(EffectDefinition)
			|| Cast<UFinalBattleEffectMoveCards>(EffectDefinition))
		{
			return true;
		}
	}

	return false;
}

bool HasSupportedEffectInternal(const UFinalCardDefinition* CardDefinition)
{
	return CardDefinition != nullptr && HasSupportedEffectListInternal(CardDefinition->Effects);
}

bool HasSupportedEffectInternal(const UFinalUltimateDefinition* UltimateDefinition)
{
	return UltimateDefinition != nullptr && HasSupportedEffectListInternal(UltimateDefinition->Effects);
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

FFinalBattleConditionEvaluationContext BuildConditionEvaluationContext(
	const FFinalBattleState& State,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	const FName SourceOwnerUnitId,
	const FFinalBattleEnemyState* TargetEnemyState = nullptr)
{
	FFinalBattleConditionEvaluationContext Context;
	Context.BattleState = &State;
	Context.TargetEnemyState = TargetEnemyState;
	Context.ChainRecords = &ExecutionContext.ChainRecords;
	Context.CardService = &GetCardService();
	Context.SourceOwnerUnitId = SourceOwnerUnitId;
	return Context;
}

FFinalBattleEnemyState* ResolvePrimaryEnemyTarget(
	FFinalBattleState& State,
	const FFinalBattleCommand* Command,
	const EFinalBattleUnitTargetRule TargetRule,
	const FFinalBattleUnitService& UnitService)
{
	switch (TargetRule)
	{
	case EFinalBattleUnitTargetRule::SelectedEnemy:
		{
			FFinalBattleEnemyState* SelectedEnemyState = nullptr;
			if (Command && Command->TargetUnitId != NAME_None)
			{
				SelectedEnemyState = UnitService.FindEnemyState(State, Command->TargetUnitId);
			}
			else if (State.CurrentTargetUnitId != NAME_None)
			{
				SelectedEnemyState = UnitService.FindEnemyState(State, State.CurrentTargetUnitId);
			}

			return (SelectedEnemyState != nullptr && SelectedEnemyState->CurrentHP > 0)
				? SelectedEnemyState
				: UnitService.FindFirstAliveEnemy(State);
		}

	case EFinalBattleUnitTargetRule::FirstAliveEnemy:
	case EFinalBattleUnitTargetRule::AllEnemies:
		return UnitService.FindFirstAliveEnemy(State);

	default:
		return nullptr;
	}
}

FFinalStatusId ResolveEffectStatusId(const UFinalBattleEffectApplyStatus* EffectDefinition)
{
	if (EffectDefinition == nullptr)
	{
		return FFinalStatusId();
	}

	if (EffectDefinition->StatusId.IsValid())
	{
		return EffectDefinition->StatusId;
	}

	return EffectDefinition->StatusDefinition ? EffectDefinition->StatusDefinition->StatusId : FFinalStatusId();
}

FFinalStatusId ResolveEffectStatusId(const UFinalBattleEffectRemoveStatus* EffectDefinition)
{
	if (EffectDefinition == nullptr)
	{
		return FFinalStatusId();
	}

	if (EffectDefinition->StatusId.IsValid())
	{
		return EffectDefinition->StatusId;
	}

	return EffectDefinition->StatusDefinition ? EffectDefinition->StatusDefinition->StatusId : FFinalStatusId();
}

bool TryConvertCardZoneRule(const EFinalBattleCardZoneRule ZoneRule, EFinalBattleCardZone& OutZone)
{
	switch (ZoneRule)
	{
	case EFinalBattleCardZoneRule::Hand:
		OutZone = EFinalBattleCardZone::Hand;
		return true;

	case EFinalBattleCardZoneRule::DrawPileTop:
		OutZone = EFinalBattleCardZone::DrawPileTop;
		return true;

	case EFinalBattleCardZoneRule::DrawPileBottom:
		OutZone = EFinalBattleCardZone::DrawPileBottom;
		return true;

	case EFinalBattleCardZoneRule::DiscardPile:
		OutZone = EFinalBattleCardZone::DiscardPile;
		return true;

	case EFinalBattleCardZoneRule::OngoingZone:
		OutZone = EFinalBattleCardZone::OngoingZone;
		return true;

	case EFinalBattleCardZoneRule::ConsumePile:
		OutZone = EFinalBattleCardZone::ConsumePile;
		return true;

	default:
		return false;
	}
}

UFinalCardDefinition* ResolveGeneratedCardDefinition(
	const UFinalBattleEffectGenerateCard* GenerateCardEffect,
	const int32 GenerationIndex)
{
	if (GenerateCardEffect == nullptr)
	{
		return nullptr;
	}

	TArray<UFinalCardDefinition*> CandidateDefinitions;
	if (GenerateCardEffect->GeneratedCardDefinition != nullptr && GenerateCardEffect->GeneratedCardDefinition->CardId.IsValid())
	{
		CandidateDefinitions.Add(GenerateCardEffect->GeneratedCardDefinition);
	}

	for (UFinalCardDefinition* CandidateDefinition : GenerateCardEffect->CandidateCardDefinitions)
	{
		if (CandidateDefinition != nullptr && CandidateDefinition->CardId.IsValid())
		{
			CandidateDefinitions.AddUnique(CandidateDefinition);
		}
	}

	if (CandidateDefinitions.IsEmpty())
	{
		return nullptr;
	}

	if (GenerateCardEffect->bChooseRandomCandidate)
	{
		return CandidateDefinitions[FMath::RandHelper(CandidateDefinitions.Num())];
	}

	const int32 CandidateIndex = FMath::Clamp(GenerationIndex, 0, CandidateDefinitions.Num() - 1);
	return CandidateDefinitions[CandidateIndex];
}

FName ResolveSourceOwnerUnitId(const FFinalBattleCharacterState* SourceCharacterState, const FFinalBattleEnemyState* SourceEnemyState)
{
	if (SourceCharacterState != nullptr)
	{
		return SourceCharacterState->RuntimeUnitId;
	}

	if (SourceEnemyState != nullptr)
	{
		return SourceEnemyState->RuntimeUnitId;
	}

	return NAME_None;
}

TArray<FName> ResolveStatusTargetOwnerUnitIds(
	FFinalBattleState& State,
	const FFinalBattleCommand* Command,
	const EFinalBattleUnitTargetRule TargetRule,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FFinalBattleUnitService& UnitService)
{
	TArray<FName> TargetOwnerUnitIds;

	switch (TargetRule)
	{
	case EFinalBattleUnitTargetRule::TeamPlayer:
		TargetOwnerUnitIds.Add(TeamPlayerUnitId);
		break;

	case EFinalBattleUnitTargetRule::AllPlayerCharacters:
		for (const FFinalBattleCharacterState& CharacterState : State.Characters)
		{
			TargetOwnerUnitIds.Add(CharacterState.RuntimeUnitId);
		}
		break;

	case EFinalBattleUnitTargetRule::Self:
		if (SourceCharacterState != nullptr)
		{
			TargetOwnerUnitIds.Add(SourceCharacterState->RuntimeUnitId);
		}
		else if (SourceEnemyState != nullptr)
		{
			TargetOwnerUnitIds.Add(SourceEnemyState->RuntimeUnitId);
		}
		break;

	case EFinalBattleUnitTargetRule::SelectedEnemy:
	case EFinalBattleUnitTargetRule::FirstAliveEnemy:
		if (FFinalBattleEnemyState* TargetEnemy = ResolvePrimaryEnemyTarget(State, Command, TargetRule, UnitService))
		{
			TargetOwnerUnitIds.Add(TargetEnemy->RuntimeUnitId);
		}
		break;

	case EFinalBattleUnitTargetRule::AllEnemies:
		for (const FFinalBattleEnemyState& EnemyState : State.Enemies)
		{
			if (EnemyState.CurrentHP > 0)
			{
				TargetOwnerUnitIds.Add(EnemyState.RuntimeUnitId);
			}
		}
		break;

	default:
		break;
	}

	return TargetOwnerUnitIds;
}

FFinalPassiveId ResolveEffectPassiveId(const UFinalBattleEffectApplyPassive* EffectDefinition)
{
	if (EffectDefinition == nullptr)
	{
		return FFinalPassiveId();
	}

	if (EffectDefinition->PassiveId.IsValid())
	{
		return EffectDefinition->PassiveId;
	}

	return EffectDefinition->PassiveDefinition != nullptr ? EffectDefinition->PassiveDefinition->PassiveId : FFinalPassiveId();
}

int32 ApplyOutgoingDamageModifier(const int32 BaseDamage, const int32 ModifierPercent)
{
	if (BaseDamage <= 0 || ModifierPercent == 0)
	{
		return BaseDamage;
	}

	const float ModifierScale = 1.0f + (static_cast<float>(ModifierPercent) / 100.0f);
	return FMath::Max(FMath::RoundToInt(static_cast<float>(BaseDamage) * ModifierScale), 0);
}

int32 ResolveCurrentCardOutgoingDamageModifierPercent(
	const FFinalBattleState& State,
	const FFinalBattleEffectExecutionContext& ExecutionContext)
{
	if (!ExecutionContext.Transient.SourceCardInstanceId.IsValid())
	{
		return 0;
	}

	const FFinalBattleCardInstance* SourceCardInstance = GetCardService().FindCardInstance(State, ExecutionContext.Transient.SourceCardInstanceId);
	return SourceCardInstance != nullptr ? SourceCardInstance->RuntimeOutgoingDamagePercent : 0;
}

bool ShouldApplyCriticalHit(const FFinalBattleCharacterState* SourceCharacterState)
{
	if (SourceCharacterState == nullptr)
	{
		return false;
	}

	const float CriticalChance = FMath::Clamp(SourceCharacterState->RuntimeCritChance, 0.0f, 1.0f);
	return CriticalChance >= 1.0f || (CriticalChance > 0.0f && FMath::FRand() < CriticalChance);
}

int32 ApplyCriticalHitMultiplier(const int32 BaseDamage, const FFinalBattleCharacterState* SourceCharacterState)
{
	if (BaseDamage <= 0 || SourceCharacterState == nullptr)
	{
		return BaseDamage;
	}

	return FMath::Max(
		FMath::RoundToInt(static_cast<float>(BaseDamage) * FMath::Max(SourceCharacterState->RuntimeCritDamage, 1.0f)),
		0);
}

int32 ApplyDamageToEnemy(
	FFinalBattleState& State,
	FFinalBattleEnemyState& EnemyState,
	const int32 DamageAmount,
	const FFinalBattleUnitService& UnitService,
	int32& OutDefeatCount)
{
	OutDefeatCount = 0;
	const bool bWasAlive = EnemyState.CurrentHP > 0;
	const int32 ShieldAbsorbed = FMath::Min(EnemyState.CurrentShield, FMath::Max(DamageAmount, 0));
	EnemyState.CurrentShield -= ShieldAbsorbed;
	const int32 HpDamage = FMath::Max(DamageAmount - ShieldAbsorbed, 0);
	EnemyState.CurrentHP = FMath::Max(0, EnemyState.CurrentHP - HpDamage);
	EnemyState.CurrentInitiative = FMath::Max(0, EnemyState.CurrentInitiative - 1);

	if (EnemyState.CurrentHP <= 0)
	{
		if (bWasAlive)
		{
			OutDefeatCount = 1;
		}
		EnemyState.CurrentIntentText = FText::FromString(TEXT("Defeated"));
		if (State.CurrentTargetUnitId == EnemyState.RuntimeUnitId)
		{
			State.CurrentTargetUnitId = NAME_None;
			if (FFinalBattleEnemyState* NextTarget = UnitService.FindFirstAliveEnemy(State))
			{
				State.CurrentTargetUnitId = NextTarget->RuntimeUnitId;
			}
		}
		return HpDamage;
	}

	RefreshEnemyIntentState(State, EnemyState, State.CurrentRound, true);
	return HpDamage;
}

int32 ApplyTeamIncomingDamage(FFinalBattleState& State, const int32 TotalIncomingDamage)
{
	const int32 ShieldAbsorbed = FMath::Min(State.TeamShield, FMath::Max(TotalIncomingDamage, 0));
	State.TeamShield -= ShieldAbsorbed;

	const int32 IncomingHpDamage = FMath::Max(TotalIncomingDamage - ShieldAbsorbed, 0);
	const int32 HpDamage = GetStatusService().ApplyIncomingTeamHealthDamageProtection(State, IncomingHpDamage);
	State.TeamCurrentHP = FMath::Max(0, State.TeamCurrentHP - HpDamage);
	return HpDamage;
}

int32 ApplyTeamIncomingDamageAndTriggersInternal(
	FFinalBattleState& State,
	const int32 TotalIncomingDamage,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	FFinalBattleEffectExecutionSummary& Summary)
{
	const int32 HpDamage = ApplyTeamIncomingDamage(State, TotalIncomingDamage);
	if (HpDamage > 0)
	{
		TArray<FFinalBattleEvent> RelicEvents;
		TriggerService.HandlePlayerTeamTookHealthDamage(State, HpDamage, GetConditionService(), EffectExecutionService, UnitService, RelicEvents);
		for (const FFinalBattleEvent& RelicEvent : RelicEvents)
		{
			GetEventService().AppendBattleEvent(State, RelicEvent);
		}

		TArray<FFinalBattleEvent> PassiveEvents;
		TriggerService.HandleOwnerTookHealthDamage(State, UnitService, GetConditionService(), EffectExecutionService, Summary, PassiveEvents);
		for (const FFinalBattleEvent& PassiveEvent : PassiveEvents)
		{
			GetEventService().AppendBattleEvent(State, PassiveEvent);
		}
	}
	return HpDamage;
}

int32 ApplyTeamHealing(FFinalBattleState& State, const int32 HealingAmount)
{
	const int32 ClampedHealingAmount = FMath::Max(HealingAmount, 0);
	const int32 PreviousHP = State.TeamCurrentHP;
	State.TeamCurrentHP = FMath::Min(State.TeamCurrentHP + ClampedHealingAmount, State.TeamMaxHP);
	return FMath::Max(State.TeamCurrentHP - PreviousHP, 0);
}

int32 ApplyHealingToEnemy(FFinalBattleEnemyState& EnemyState, const int32 HealingAmount)
{
	const int32 ClampedHealingAmount = FMath::Max(HealingAmount, 0);
	const int32 PreviousHP = EnemyState.CurrentHP;
	EnemyState.CurrentHP = FMath::Min(EnemyState.CurrentHP + ClampedHealingAmount, EnemyState.MaxHP);
	return FMath::Max(EnemyState.CurrentHP - PreviousHP, 0);
}

int32 ApplyBonusBreakToEnemy(FFinalBattleEnemyState& EnemyState, const int32 BreakAmount)
{
	const int32 ClampedBreakAmount = FMath::Max(BreakAmount, 0);
	const int32 PreviousBreakValue = EnemyState.CurrentBreakValue;
	EnemyState.CurrentBreakValue = FMath::Max(EnemyState.CurrentBreakValue - ClampedBreakAmount, 0);
	return FMath::Max(PreviousBreakValue - EnemyState.CurrentBreakValue, 0);
}

bool ExecuteGenerateCardEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectGenerateCard* GenerateCardEffect,
	const FName SourceOwnerUnitId,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		GenerateCardEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 GenerateCount = FMath::Max(GenerateCardEffect->GenerateCount, 0);
	if (GenerateCount <= 0 || SourceOwnerUnitId.IsNone())
	{
		return false;
	}

	int32 GeneratedCardCount = 0;
	for (int32 GenerationIndex = 0; GenerationIndex < GenerateCount; ++GenerationIndex)
	{
		UFinalCardDefinition* SelectedCardDefinition = ResolveGeneratedCardDefinition(GenerateCardEffect, GenerationIndex);
		if (SelectedCardDefinition == nullptr)
		{
			continue;
		}

		const FGuid GeneratedCardInstanceId = GetCardService().CreateCardInstance(
			State,
			SelectedCardDefinition,
			SourceOwnerUnitId,
			State.RuntimeProjectionOwner,
			NAME_None,
			GenerateCardEffect->bGeneratedCard,
			GenerateCardEffect->bTemporaryCard);
		if (GeneratedCardInstanceId.IsValid())
		{
			GetCardService().MoveCardInstanceToZone(State, GeneratedCardInstanceId, EFinalBattleCardZone::Hand);
		}
		if (GeneratedCardInstanceId.IsValid())
		{
			++GeneratedCardCount;
		}
	}

	if (GeneratedCardCount <= 0)
	{
		return false;
	}

	GetStatusService().ResyncProjectedHandCardModifiers(State, GetCardService(), SourceOwnerUnitId);
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteMoveCardsEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectMoveCards* MoveCardsEffect,
	const FName SourceOwnerUnitId,
	FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		MoveCardsEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 TargetMoveCount = FMath::Max(MoveCardsEffect->MoveCount, 0);
	if (TargetMoveCount <= 0 || SourceOwnerUnitId.IsNone() || MoveCardsEffect->SourceZone == MoveCardsEffect->DestinationZone)
	{
		return false;
	}

	EFinalBattleCardZone SourceZone = EFinalBattleCardZone::Hand;
	EFinalBattleCardZone DestinationZone = EFinalBattleCardZone::ConsumePile;
	if (!TryConvertCardZoneRule(MoveCardsEffect->SourceZone, SourceZone)
		|| !TryConvertCardZoneRule(MoveCardsEffect->DestinationZone, DestinationZone))
	{
		return false;
	}

	TArray<FGuid> MovedCardInstanceIds;
	FFinalBattleCardMatchCriteria MatchCriteria;
	MatchCriteria.RuntimeOwnerUnitId = SourceOwnerUnitId;
	MatchCriteria.RequiredCardId = MoveCardsEffect->RequiredCardId;
	MatchCriteria.RequiredKeyword = MoveCardsEffect->RequiredKeyword;
	MatchCriteria.bGeneratedOnly = MoveCardsEffect->bGeneratedOnly;
	const int32 MovedCount = GetCardService().MoveMatchingCardsBetweenZones(
		State,
		SourceZone,
		DestinationZone,
		MatchCriteria,
		TargetMoveCount,
		&MovedCardInstanceIds);
	if (MovedCount <= 0)
	{
		return false;
	}

	if (SourceZone == EFinalBattleCardZone::Hand || DestinationZone == EFinalBattleCardZone::Hand)
	{
		GetStatusService().ResyncProjectedHandCardModifiers(State, GetCardService(), SourceOwnerUnitId);
	}

	if (MoveCardsEffect->bRecordMovedCards)
	{
		for (const FGuid& MovedCardInstanceId : MovedCardInstanceIds)
		{
			if (const FFinalBattleCardInstance* MovedCardInstance = GetCardService().FindCardInstance(State, MovedCardInstanceId))
			{
				GetConditionService().RecordMovedCard(
					ExecutionContext.ChainRecords,
					SourceOwnerUnitId,
					*MovedCardInstance,
					MoveCardsEffect->SourceZone,
					MoveCardsEffect->DestinationZone,
					1);
			}
		}
	}

	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteHealEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectHeal* HealEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FName SourceOwnerUnitId,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		HealEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 HealAmount = ResolveScalarValue(HealEffect->Scalar, SourceCharacterState, SourceEnemyState);
	if (HealAmount <= 0)
	{
		return false;
	}

	int32 AppliedHealing = 0;
	switch (HealEffect->UnitTargetRule)
	{
	case EFinalBattleUnitTargetRule::TeamPlayer:
		AppliedHealing = ApplyTeamHealing(State, HealAmount);
		Summary.TotalHealingToTeam += AppliedHealing;
		break;

	case EFinalBattleUnitTargetRule::Self:
		if (SourceEnemyState != nullptr)
		{
			AppliedHealing = ApplyHealingToEnemy(*SourceEnemyState, HealAmount);
		}
		else
		{
			AppliedHealing = ApplyTeamHealing(State, HealAmount);
			Summary.TotalHealingToTeam += AppliedHealing;
		}
		break;

	case EFinalBattleUnitTargetRule::AllEnemies:
		for (FFinalBattleEnemyState& EnemyState : State.Enemies)
		{
			if (EnemyState.CurrentHP > 0)
			{
				ApplyHealingToEnemy(EnemyState, HealAmount);
			}
		}
		break;

	default:
		if (FFinalBattleEnemyState* TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, HealEffect->UnitTargetRule, UnitService))
		{
			AppliedHealing = ApplyHealingToEnemy(*TargetEnemyState, HealAmount);
		}
		break;
	}

	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteApplyStatusEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectApplyStatus* ApplyStatusEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FName SourceOwnerUnitId,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		ApplyStatusEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const FFinalStatusId StatusId = ResolveEffectStatusId(ApplyStatusEffect);
	if (!StatusId.IsValid() || ApplyStatusEffect->Stacks <= 0)
	{
		return false;
	}

	const TArray<FName> TargetOwnerUnitIds = ResolveStatusTargetOwnerUnitIds(
		State,
		Command,
		ApplyStatusEffect->UnitTargetRule,
		SourceCharacterState,
		SourceEnemyState,
		UnitService);
	int32 AppliedStacks = 0;
	for (const FName TargetOwnerUnitId : TargetOwnerUnitIds)
	{
		const int32 AppliedStacksForTarget = GetStatusService().AddStatusStacks(
			State,
			TargetOwnerUnitId,
			SourceOwnerUnitId,
			StatusId,
			ApplyStatusEffect->StatusDefinition,
			ApplyStatusEffect->Stacks,
			ApplyStatusEffect->DurationOverride);
		AppliedStacks += AppliedStacksForTarget;
		if (AppliedStacksForTarget > 0)
		{
			GetStatusService().ResyncProjectedHandCardModifiers(State, GetCardService(), TargetOwnerUnitId);
		}
	}

	if (AppliedStacks <= 0)
	{
		return false;
	}

	Summary.TotalStatusStacksApplied += AppliedStacks;
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteApplyPassiveEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectApplyPassive* ApplyPassiveEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FName SourceOwnerUnitId,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		ApplyPassiveEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const FFinalPassiveId PassiveId = ResolveEffectPassiveId(ApplyPassiveEffect);
	if (!PassiveId.IsValid() || ApplyPassiveEffect->PassiveDefinition == nullptr || ApplyPassiveEffect->Stacks <= 0)
	{
		return false;
	}

	const TArray<FName> TargetOwnerUnitIds = ResolveStatusTargetOwnerUnitIds(
		State,
		Command,
		ApplyPassiveEffect->UnitTargetRule,
		SourceCharacterState,
		SourceEnemyState,
		UnitService);
	int32 AppliedCount = 0;
	for (const FName TargetOwnerUnitId : TargetOwnerUnitIds)
	{
		const FFinalBattlePassiveApplyResult ApplyResult = GetPassiveService().ApplyPassive(
			State,
			TargetOwnerUnitId,
			SourceOwnerUnitId,
			PassiveId,
			ApplyPassiveEffect->PassiveDefinition,
			ApplyPassiveEffect->Stacks,
			ApplyPassiveEffect->DurationOverride);
		if (!ApplyResult.bApplied)
		{
			continue;
		}

		++AppliedCount;
		GetEventService().AppendBattleEvent(State, BuildPassiveAppliedEvent(ApplyResult, PassiveAppliedEffectReasonTag));
	}

	if (AppliedCount <= 0)
	{
		return false;
	}

	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteRemoveStatusEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectRemoveStatus* RemoveStatusEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FFinalBattleUnitService& UnitService,
	FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	const FName SourceOwnerUnitId = ResolveSourceOwnerUnitId(SourceCharacterState, SourceEnemyState);
	if (!GetConditionService().SatisfiesAllEffectConditions(
		RemoveStatusEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const FFinalStatusId StatusId = ResolveEffectStatusId(RemoveStatusEffect);
	if (!StatusId.IsValid() || RemoveStatusEffect->Stacks <= 0)
	{
		return false;
	}

	const TArray<FName> TargetOwnerUnitIds = ResolveStatusTargetOwnerUnitIds(
		State,
		Command,
		RemoveStatusEffect->UnitTargetRule,
		SourceCharacterState,
		SourceEnemyState,
		UnitService);
	int32 RemovedStacks = 0;
	for (const FName TargetOwnerUnitId : TargetOwnerUnitIds)
	{
		const int32 RemovedStacksForTarget = GetStatusService().RemoveStatusStacks(
			State,
			TargetOwnerUnitId,
			StatusId,
			RemoveStatusEffect->Stacks);
		RemovedStacks += RemovedStacksForTarget;
		GetConditionService().RecordStatusChange(
			ExecutionContext.ChainRecords,
			TargetOwnerUnitId,
			StatusId,
			EFinalBattleStatusChangeKind::Removed,
			RemovedStacksForTarget);
		if (RemovedStacksForTarget > 0)
		{
			GetStatusService().ResyncProjectedHandCardModifiers(State, GetCardService(), TargetOwnerUnitId);
		}
	}

	if (RemovedStacks <= 0)
	{
		return false;
	}

	Summary.TotalStatusStacksRemoved += RemovedStacks;
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteGainShieldEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectGainShield* ShieldEffect,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FName SourceOwnerUnitId,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		ShieldEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 ShieldAmount = ResolveScalarValue(ShieldEffect->Scalar, SourceCharacterState, SourceEnemyState);
	if (ShieldAmount <= 0)
	{
		return false;
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
	return true;
}

bool ExecuteDrawCardsEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectDrawCards* DrawCardsEffect,
	const FName SourceOwnerUnitId,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		DrawCardsEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 HandCountBeforeDraw = State.DeckState.HandCardInstanceIds.Num();
	TArray<FGuid> DrawnCardInstanceIds;
	GetCardService().DrawCards(State, FMath::Max(DrawCardsEffect->DrawCount, 0), &DrawnCardInstanceIds);
	Summary.TotalCardsDrawn += FMath::Max(State.DeckState.HandCardInstanceIds.Num() - HandCountBeforeDraw, 0);
	Summary.DrawnCardInstanceIds.Append(DrawnCardInstanceIds);
	if (State.DeckState.HandCardInstanceIds.Num() > HandCountBeforeDraw)
	{
		GetStatusService().ResyncProjectedHandCardModifiers(State, GetCardService(), SourceOwnerUnitId);
	}
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteGainAPEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectGainAP* GainAPEffect,
	const FName SourceOwnerUnitId,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesAllEffectConditions(
		GainAPEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 APGain = FMath::Max(GainAPEffect->GainValue, 0);
	if (APGain <= 0)
	{
		return false;
	}

	GetResourceService().GainAP(State, APGain);
	Summary.TotalAPGained += APGain;
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteDamageEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectDamage* DamageEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FName SourceOwnerUnitId,
	const bool bIsAttackCardDamage,
	const FFinalBattleUnitService& UnitService,
	FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesSourceAndChainConditions(
		DamageEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 HitCount = FMath::Max(DamageEffect->HitCount, 1);
	const int32 BaseDamagePerHit = ResolveScalarValue(DamageEffect->Scalar, SourceCharacterState, SourceEnemyState);
	const int32 DamageModifierPercent = SourceCharacterState != nullptr
		? GetStatusService().GetOutgoingDamageModifierPercent(State, SourceOwnerUnitId, bIsAttackCardDamage)
		: 0;
	const int32 CardOutgoingDamageModifierPercent = ResolveCurrentCardOutgoingDamageModifierPercent(State, ExecutionContext);
	const int32 DamagePerHit = ApplyOutgoingDamageModifier(BaseDamagePerHit, DamageModifierPercent + CardOutgoingDamageModifierPercent);
	if (DamagePerHit <= 0)
	{
		return false;
	}

	if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::TeamPlayer)
	{
		if (!GetConditionService().SatisfiesTargetConditions(
			DamageEffect,
			BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
		{
			return false;
		}

		for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
		{
			const bool bIsCriticalHit = ShouldApplyCriticalHit(SourceCharacterState);
			const int32 ResolvedDamagePerHit = bIsCriticalHit
				? ApplyCriticalHitMultiplier(DamagePerHit, SourceCharacterState)
				: DamagePerHit;
			const int32 HpDamage = ApplyTeamIncomingDamageAndTriggersInternal(
				State,
				ResolvedDamagePerHit,
				UnitService,
				GetTriggerService(),
				GetEffectExecutionService(),
				Summary);
			Summary.TotalDamageToTeam += HpDamage;
			Summary.TotalCriticalHits += bIsCriticalHit ? 1 : 0;
			Summary.TotalCriticalBonusDamage += bIsCriticalHit ? FMath::Max(ResolvedDamagePerHit - DamagePerHit, 0) : 0;
		}

		++Summary.ResolvedEffectCount;
		return true;
	}

	if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::AllEnemies)
	{
		bool bAppliedDamageToAnyEnemy = false;
		for (FFinalBattleEnemyState& EnemyState : State.Enemies)
		{
			if (EnemyState.CurrentHP <= 0)
			{
				continue;
			}

			if (!GetConditionService().SatisfiesTargetConditions(
				DamageEffect,
				BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId, &EnemyState)))
			{
				continue;
			}

			for (int32 HitIndex = 0; HitIndex < HitCount && EnemyState.CurrentHP > 0; ++HitIndex)
			{
				const bool bIsCriticalHit = ShouldApplyCriticalHit(SourceCharacterState);
				const int32 ResolvedDamagePerHit = bIsCriticalHit
					? ApplyCriticalHitMultiplier(DamagePerHit, SourceCharacterState)
					: DamagePerHit;
				int32 DefeatCount = 0;
				const int32 HpDamage = ApplyDamageToEnemy(State, EnemyState, ResolvedDamagePerHit, UnitService, DefeatCount);
				ExecutionContext.Transient.bAppliedSuccessfulEnemyHpDamage |= HpDamage > 0;
				Summary.TotalDamageToEnemies += ResolvedDamagePerHit;
				Summary.TotalEnemiesDefeated += DefeatCount;
				Summary.TotalCriticalHits += bIsCriticalHit ? 1 : 0;
				Summary.TotalCriticalBonusDamage += bIsCriticalHit ? FMath::Max(ResolvedDamagePerHit - DamagePerHit, 0) : 0;
				bAppliedDamageToAnyEnemy = true;
			}
		}

		if (!bAppliedDamageToAnyEnemy)
		{
			return false;
		}

		++Summary.ResolvedEffectCount;
		return true;
	}

	FFinalBattleEnemyState* TargetEnemyState = nullptr;
	if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::Self && SourceEnemyState)
	{
		TargetEnemyState = SourceEnemyState;
	}
	else
	{
		TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, DamageEffect->UnitTargetRule, UnitService);
	}

	if (TargetEnemyState == nullptr)
	{
		return false;
	}

	if (!GetConditionService().SatisfiesTargetConditions(
		DamageEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId, TargetEnemyState)))
	{
		return false;
	}

	for (int32 HitIndex = 0; HitIndex < HitCount && TargetEnemyState->CurrentHP > 0; ++HitIndex)
	{
		const bool bIsCriticalHit = ShouldApplyCriticalHit(SourceCharacterState);
		const int32 ResolvedDamagePerHit = bIsCriticalHit
			? ApplyCriticalHitMultiplier(DamagePerHit, SourceCharacterState)
			: DamagePerHit;
		int32 DefeatCount = 0;
		const int32 HpDamage = ApplyDamageToEnemy(State, *TargetEnemyState, ResolvedDamagePerHit, UnitService, DefeatCount);
		ExecutionContext.Transient.bAppliedSuccessfulEnemyHpDamage |= HpDamage > 0;
		Summary.TotalDamageToEnemies += ResolvedDamagePerHit;
		Summary.TotalEnemiesDefeated += DefeatCount;
		Summary.TotalCriticalHits += bIsCriticalHit ? 1 : 0;
		Summary.TotalCriticalBonusDamage += bIsCriticalHit ? FMath::Max(ResolvedDamagePerHit - DamagePerHit, 0) : 0;
	}

	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteBonusBreakEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectBonusBreak* BonusBreakEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FName SourceOwnerUnitId,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleEffectExecutionContext& ExecutionContext,
	FFinalBattleEffectExecutionSummary& Summary)
{
	if (!GetConditionService().SatisfiesSourceAndChainConditions(
		BonusBreakEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId)))
	{
		return false;
	}

	const int32 BreakAmount = ResolveScalarValue(BonusBreakEffect->Scalar, SourceCharacterState, SourceEnemyState);
	if (BreakAmount <= 0)
	{
		return false;
	}

	if (BonusBreakEffect->UnitTargetRule == EFinalBattleUnitTargetRule::AllEnemies)
	{
		bool bAppliedBreakToAnyEnemy = false;
		for (FFinalBattleEnemyState& EnemyState : State.Enemies)
		{
			if (EnemyState.CurrentHP <= 0)
			{
				continue;
			}

			if (!GetConditionService().SatisfiesTargetConditions(
				BonusBreakEffect,
				BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId, &EnemyState)))
			{
				continue;
			}

			Summary.TotalBreakDamageToEnemies += ApplyBonusBreakToEnemy(EnemyState, BreakAmount);
			bAppliedBreakToAnyEnemy = true;
		}

		if (!bAppliedBreakToAnyEnemy)
		{
			return false;
		}

		++Summary.ResolvedEffectCount;
		return true;
	}

	FFinalBattleEnemyState* TargetEnemyState = nullptr;
	if (BonusBreakEffect->UnitTargetRule == EFinalBattleUnitTargetRule::Self && SourceEnemyState)
	{
		TargetEnemyState = SourceEnemyState;
	}
	else
	{
		TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, BonusBreakEffect->UnitTargetRule, UnitService);
	}

	if (TargetEnemyState == nullptr)
	{
		return false;
	}

	if (!GetConditionService().SatisfiesTargetConditions(
		BonusBreakEffect,
		BuildConditionEvaluationContext(State, ExecutionContext, SourceOwnerUnitId, TargetEnemyState)))
	{
		return false;
	}

	Summary.TotalBreakDamageToEnemies += ApplyBonusBreakToEnemy(*TargetEnemyState, BreakAmount);
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteEffectListInternal(
	FFinalBattleState& State,
	const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
	const FFinalBattleCommand* Command,
	const UFinalCardDefinition* SourceCardDefinition,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FFinalBattleUnitService& UnitService,
	FFinalBattleEffectExecutionSummary& Summary)
{
	FFinalBattleEffectExecutionContext ExecutionContext;
	if (Command != nullptr && Command->CommandType == EFinalBattleCommandType::PlayCard)
	{
		ExecutionContext.Transient.SourceCardInstanceId = Command->CardInstanceId;
	}
	const FName SourceOwnerUnitId = ResolveSourceOwnerUnitId(SourceCharacterState, SourceEnemyState);
	const bool bIsAttackCardDamage = SourceCardDefinition != nullptr && SourceCardDefinition->CardType == EFinalCardType::Attack;

	for (UFinalBattleEffectDefinition* EffectDefinition : Effects)
	{
		if (EffectDefinition == nullptr)
		{
			continue;
		}

		if (const UFinalBattleEffectDamage* DamageEffect = Cast<UFinalBattleEffectDamage>(EffectDefinition))
		{
			ExecuteDamageEffect(State, DamageEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, bIsAttackCardDamage, UnitService, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectGenerateCard* GenerateCardEffect = Cast<UFinalBattleEffectGenerateCard>(EffectDefinition))
		{
			ExecuteGenerateCardEffect(State, GenerateCardEffect, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectMoveCards* MoveCardsEffect = Cast<UFinalBattleEffectMoveCards>(EffectDefinition))
		{
			ExecuteMoveCardsEffect(State, MoveCardsEffect, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectHeal* HealEffect = Cast<UFinalBattleEffectHeal>(EffectDefinition))
		{
			ExecuteHealEffect(State, HealEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, UnitService, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectApplyStatus* ApplyStatusEffect = Cast<UFinalBattleEffectApplyStatus>(EffectDefinition))
		{
			ExecuteApplyStatusEffect(State, ApplyStatusEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, UnitService, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectApplyPassive* ApplyPassiveEffect = Cast<UFinalBattleEffectApplyPassive>(EffectDefinition))
		{
			ExecuteApplyPassiveEffect(State, ApplyPassiveEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, UnitService, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectRemoveStatus* RemoveStatusEffect = Cast<UFinalBattleEffectRemoveStatus>(EffectDefinition))
		{
			ExecuteRemoveStatusEffect(State, RemoveStatusEffect, Command, SourceCharacterState, SourceEnemyState, UnitService, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectGainShield* ShieldEffect = Cast<UFinalBattleEffectGainShield>(EffectDefinition))
		{
			ExecuteGainShieldEffect(State, ShieldEffect, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectDrawCards* DrawCardsEffect = Cast<UFinalBattleEffectDrawCards>(EffectDefinition))
		{
			ExecuteDrawCardsEffect(State, DrawCardsEffect, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectGainAP* GainAPEffect = Cast<UFinalBattleEffectGainAP>(EffectDefinition))
		{
			ExecuteGainAPEffect(State, GainAPEffect, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectBonusBreak* BonusBreakEffect = Cast<UFinalBattleEffectBonusBreak>(EffectDefinition))
		{
			ExecuteBonusBreakEffect(State, BonusBreakEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, UnitService, ExecutionContext, Summary);
			continue;
		}
	}

	if (SourceCharacterState != nullptr && ExecutionContext.Transient.bAppliedSuccessfulEnemyHpDamage)
	{
		GetStatusService().ConsumeOutgoingDamageModifierStacks(State, SourceOwnerUnitId, bIsAttackCardDamage);
		GetStatusService().ResyncProjectedHandCardModifiers(State, GetCardService(), SourceOwnerUnitId);
	}

	return Summary.ResolvedEffectCount > 0;
}
}

bool FFinalBattleEffectExecutionService::HasSupportedEffectList(const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects) const
{
	return HasSupportedEffectListInternal(Effects);
}

bool FFinalBattleEffectExecutionService::HasSupportedEffect(const UFinalCardDefinition* CardDefinition) const
{
	return HasSupportedEffectInternal(CardDefinition);
}

bool FFinalBattleEffectExecutionService::HasSupportedEffect(const UFinalUltimateDefinition* UltimateDefinition) const
{
	return HasSupportedEffectInternal(UltimateDefinition);
}

bool FFinalBattleEffectExecutionService::ExecuteEffectList(
	FFinalBattleState& State,
	const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
	const FFinalBattleCommand* Command,
	const UFinalCardDefinition* SourceCardDefinition,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	const FFinalBattleUnitService& UnitService,
	FFinalBattleEffectExecutionSummary& Summary) const
{
	return ExecuteEffectListInternal(State, Effects, Command, SourceCardDefinition, SourceCharacterState, SourceEnemyState, UnitService, Summary);
}

int32 FFinalBattleEffectExecutionService::ApplyTeamIncomingDamageAndTriggers(
	FFinalBattleState& State,
	const int32 TotalIncomingDamage,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	FFinalBattleEffectExecutionSummary& Summary) const
{
	return ApplyTeamIncomingDamageAndTriggersInternal(State, TotalIncomingDamage, UnitService, TriggerService, *this, Summary);
}
