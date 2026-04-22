#include "Resolver/FinalBattleResolver.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Battle/Effects/FinalBattleEffectConsumeGeneratedCard.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainAP.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Battle/Effects/FinalBattleHandCardRequirement.h"
#include "Battle/Effects/FinalBattleEffectHeal.h"
#include "Battle/Effects/FinalBattleGeneratedCardConsumeRequirement.h"
#include "Battle/Effects/FinalBattleEffectRemoveStatus.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalBattleTurnService.h"
#include "Systems/FinalEnemyIntentService.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalBattleResolver, Log, All);

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName RejectBattleResolvedTag(TEXT("battle.already_resolved"));
const FName RejectCardInstanceMissingTag(TEXT("battle.card_instance_missing"));
const FName RejectCardDefinitionMissingTag(TEXT("battle.card_definition_missing"));
const FName RejectCardNotInHandTag(TEXT("battle.card_not_in_hand"));
const FName RejectNotEnoughAPTag(TEXT("battle.not_enough_ap"));
const FName RejectUnsupportedCardEffectsTag(TEXT("battle.unsupported_card_effects"));
const FName RejectUltimateOwnerMissingTag(TEXT("battle.ultimate_owner_missing"));
const FName RejectUltimateBlockedByCollapseTag(TEXT("battle.ultimate_blocked_by_collapse"));
const FName RejectUltimateAlreadyUsedTag(TEXT("battle.ultimate_already_used"));
const FName RejectUltimateDefinitionMissingTag(TEXT("battle.ultimate_definition_unavailable"));
const FName RejectNotEnoughEPTag(TEXT("battle.not_enough_ep"));
const FName RejectInvalidTargetTag(TEXT("battle.invalid_target"));
const FName RejectUnsupportedCommandTag(TEXT("battle.unsupported_command"));

struct FFinalEffectExecutionSummary
{
	int32 TotalDamageToEnemies = 0;
	int32 TotalDamageToTeam = 0;
	int32 TotalBreakDamageToEnemies = 0;
	int32 TotalHealingToTeam = 0;
	int32 TotalTeamShieldGained = 0;
	int32 TotalEnemyShieldGained = 0;
	int32 TotalStatusStacksApplied = 0;
	int32 TotalStatusStacksRemoved = 0;
	int32 TotalCardsDrawn = 0;
	int32 TotalAPGained = 0;
	int32 ResolvedEffectCount = 0;
};

struct FFinalConsumedStatusRecord
{
	FName OwnerUnitId = NAME_None;
	FFinalStatusId StatusId;
	int32 RemovedStacks = 0;
};

struct FFinalConsumedGeneratedCardRecord
{
	FName RuntimeOwnerUnitId = NAME_None;
	FFinalCardId CardId;
	int32 RemovedCount = 0;
	FGameplayTagContainer RuntimeKeywords;
};

struct FFinalEffectExecutionContext
{
	TArray<FFinalConsumedStatusRecord> ConsumedStatuses;
	TArray<FFinalConsumedGeneratedCardRecord> ConsumedGeneratedCards;
	bool bAppliedSuccessfulEnemyHpDamage = false;
};

void AppendBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event);
const FFinalBattleCardService& GetCardService();
const FFinalBattleRelicService& GetRelicService();
const FFinalBattleResourceService& GetResourceService();
const FFinalBattleTurnService& GetTurnService();
const FFinalBattleStatusService& GetStatusService();
bool ExecuteEffectList(
	FFinalBattleState& State,
	const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
	const FFinalBattleCommand* Command,
	const UFinalCardDefinition* SourceCardDefinition,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	FFinalEffectExecutionSummary& Summary);

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
		&& !CharacterState.bUltimateUsedThisBattle
		&& CharacterState.UltimateId.IsValid()
		&& State.CurrentEP >= CharacterState.UltimateCostEP;
}

void SetRejectReason(FFinalBattleEvent& Event, const EFinalBattleCommandRejectReason RejectReason, const FName ReasonTag)
{
	Event.RejectReason = RejectReason;
	Event.ReasonTag = ReasonTag;
}

FFinalBattleEvent BuildRejectedCommandEvent(
	const EFinalBattleCommandRejectReason RejectReason,
	const FName ReasonTag,
	const FText& Message)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::CommandRejected;
	SetRejectReason(Event, RejectReason, ReasonTag);
	Event.Message = Message;
	return Event;
}

FName ResolveCommandTargetUnitId(const FFinalBattleState& State, const FFinalBattleCommand& Command)
{
	return Command.TargetUnitId != NAME_None ? Command.TargetUnitId : State.CurrentTargetUnitId;
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

void DrawCards(FFinalBattleState& State, const int32 DrawCount)
{
	GetCardService().DrawCards(State, DrawCount);
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

const FFinalBattleCardService& GetCardService()
{
	static const FFinalBattleCardService CardService;
	return CardService;
}

const FFinalBattleRelicService& GetRelicService()
{
	static const FFinalBattleRelicService RelicService;
	return RelicService;
}

const FFinalBattleResourceService& GetResourceService()
{
	static const FFinalBattleResourceService ResourceService;
	return ResourceService;
}

const FFinalBattleTurnService& GetTurnService()
{
	static const FFinalBattleTurnService TurnService;
	return TurnService;
}

const FFinalBattleStatusService& GetStatusService()
{
	static const FFinalBattleStatusService StatusService;
	return StatusService;
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
			|| Cast<UFinalBattleEffectHeal>(EffectDefinition)
			|| Cast<UFinalBattleEffectApplyStatus>(EffectDefinition)
			|| Cast<UFinalBattleEffectRemoveStatus>(EffectDefinition)
			|| Cast<UFinalBattleEffectGainShield>(EffectDefinition)
			|| Cast<UFinalBattleEffectDrawCards>(EffectDefinition)
			|| Cast<UFinalBattleEffectGainAP>(EffectDefinition)
			|| Cast<UFinalBattleEffectBonusBreak>(EffectDefinition)
			|| Cast<UFinalBattleEffectGenerateCard>(EffectDefinition)
			|| Cast<UFinalBattleEffectConsumeGeneratedCard>(EffectDefinition))
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

bool HasTargetStateRequirement(const FFinalBattleTargetStateRequirement& Requirement)
{
	return Requirement.bRequireEnemyTarget
		|| Requirement.bRequireTargetBroken
		|| Requirement.bRequireTargetAlive;
}

bool IsEnemyBroken(const FFinalBattleEnemyState& EnemyState)
{
	return EnemyState.CurrentBreakValue <= 0;
}

bool SatisfiesTargetStateRequirement(
	const FFinalBattleTargetStateRequirement& Requirement,
	const FFinalBattleEnemyState* TargetEnemyState)
{
	if (!HasTargetStateRequirement(Requirement))
	{
		return true;
	}

	if (Requirement.bRequireEnemyTarget && TargetEnemyState == nullptr)
	{
		return false;
	}

	if (TargetEnemyState == nullptr)
	{
		return false;
	}

	if (Requirement.bRequireTargetAlive && TargetEnemyState->CurrentHP <= 0)
	{
		return false;
	}

	if (Requirement.bRequireTargetBroken && !IsEnemyBroken(*TargetEnemyState))
	{
		return false;
	}

	return true;
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

void RecordConsumedStatus(
	FFinalEffectExecutionContext& Context,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId,
	const int32 RemovedStacks)
{
	if (OwnerUnitId.IsNone() || !StatusId.IsValid() || RemovedStacks <= 0)
	{
		return;
	}

	if (FFinalConsumedStatusRecord* ExistingRecord = Context.ConsumedStatuses.FindByPredicate(
		[&OwnerUnitId, &StatusId](const FFinalConsumedStatusRecord& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.StatusId == StatusId;
		}))
	{
		ExistingRecord->RemovedStacks += RemovedStacks;
		return;
	}

	FFinalConsumedStatusRecord& NewRecord = Context.ConsumedStatuses.AddDefaulted_GetRef();
	NewRecord.OwnerUnitId = OwnerUnitId;
	NewRecord.StatusId = StatusId;
	NewRecord.RemovedStacks = RemovedStacks;
}

int32 ResolveConsumedStatusStacks(
	const FFinalEffectExecutionContext& Context,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId)
{
	if (const FFinalConsumedStatusRecord* ExistingRecord = Context.ConsumedStatuses.FindByPredicate(
		[&OwnerUnitId, &StatusId](const FFinalConsumedStatusRecord& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.StatusId == StatusId;
		}))
	{
		return ExistingRecord->RemovedStacks;
	}

	return 0;
}

void RecordConsumedGeneratedCard(
	FFinalEffectExecutionContext& Context,
	const FName RuntimeOwnerUnitId,
	const FFinalBattleCardInstance& CardInstance,
	const int32 RemovedCount)
{
	if (RuntimeOwnerUnitId.IsNone() || !CardInstance.CardId.IsValid() || RemovedCount <= 0)
	{
		return;
	}

	if (FFinalConsumedGeneratedCardRecord* ExistingRecord = Context.ConsumedGeneratedCards.FindByPredicate(
		[&RuntimeOwnerUnitId, &CardInstance](const FFinalConsumedGeneratedCardRecord& Candidate)
		{
			return Candidate.RuntimeOwnerUnitId == RuntimeOwnerUnitId && Candidate.CardId == CardInstance.CardId;
		}))
	{
		ExistingRecord->RemovedCount += RemovedCount;
		ExistingRecord->RuntimeKeywords.AppendTags(CardInstance.RuntimeKeywords);
		return;
	}

	FFinalConsumedGeneratedCardRecord& NewRecord = Context.ConsumedGeneratedCards.AddDefaulted_GetRef();
	NewRecord.RuntimeOwnerUnitId = RuntimeOwnerUnitId;
	NewRecord.CardId = CardInstance.CardId;
	NewRecord.RemovedCount = RemovedCount;
	NewRecord.RuntimeKeywords = CardInstance.RuntimeKeywords;
}

int32 ResolveConsumedGeneratedCardCount(
	const FFinalEffectExecutionContext& Context,
	const FName RuntimeOwnerUnitId,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword)
{
	int32 TotalConsumedCount = 0;

	for (const FFinalConsumedGeneratedCardRecord& Record : Context.ConsumedGeneratedCards)
	{
		if (Record.RuntimeOwnerUnitId != RuntimeOwnerUnitId)
		{
			continue;
		}

		if (RequiredCardId.IsValid() && Record.CardId != RequiredCardId)
		{
			continue;
		}

		if (RequiredKeyword.IsValid() && !Record.RuntimeKeywords.HasTagExact(RequiredKeyword))
		{
			continue;
		}

		TotalConsumedCount += Record.RemovedCount;
	}

	return TotalConsumedCount;
}

bool SatisfiesGeneratedCardConsumeRequirement(
	const FFinalBattleGeneratedCardConsumeRequirement& Requirement,
	const FFinalEffectExecutionContext& Context,
	const FName SourceOwnerUnitId)
{
	if (!Requirement.bRequireConsumedGeneratedCard)
	{
		return true;
	}

	return !SourceOwnerUnitId.IsNone()
		&& ResolveConsumedGeneratedCardCount(
			Context,
			SourceOwnerUnitId,
			Requirement.RequiredCardId,
			Requirement.RequiredKeyword) >= FMath::Max(Requirement.MinimumCount, 1);
}

bool SatisfiesHandCardRequirement(
	const FFinalBattleHandCardRequirement& Requirement,
	const FFinalBattleState& State,
	const FName SourceOwnerUnitId)
{
	if (!Requirement.bRequireInHand)
	{
		return true;
	}

	return !SourceOwnerUnitId.IsNone()
		&& GetCardService().SatisfiesHandCardRequirement(State, SourceOwnerUnitId, Requirement);
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

bool SatisfiesConsumeRequirement(
	const FFinalBattleStatusConsumeRequirement& Requirement,
	const FFinalEffectExecutionContext& Context,
	const FName SourceOwnerUnitId)
{
	if (!Requirement.bRequireConsumedStatus)
	{
		return true;
	}

	return Requirement.RequiredStatusId.IsValid()
		&& !SourceOwnerUnitId.IsNone()
		&& ResolveConsumedStatusStacks(Context, SourceOwnerUnitId, Requirement.RequiredStatusId) >= FMath::Max(Requirement.MinimumStacks, 1);
}

TArray<FName> ResolveStatusTargetOwnerUnitIds(
	FFinalBattleState& State,
	const FFinalBattleCommand* Command,
	const EFinalBattleUnitTargetRule TargetRule,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState)
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
		if (FFinalBattleEnemyState* TargetEnemy = ResolvePrimaryEnemyTarget(State, Command, TargetRule))
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

int32 ApplyOutgoingDamageModifier(const int32 BaseDamage, const int32 ModifierPercent)
{
	if (BaseDamage <= 0 || ModifierPercent == 0)
	{
		return BaseDamage;
	}

	const float ModifierScale = 1.0f + (static_cast<float>(ModifierPercent) / 100.0f);
	return FMath::Max(FMath::RoundToInt(static_cast<float>(BaseDamage) * ModifierScale), 0);
}

int32 ApplyDamageToEnemy(FFinalBattleState& State, FFinalBattleEnemyState& EnemyState, const int32 DamageAmount)
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

void ExecuteOwnerTookHealthDamageTriggers(FFinalBattleState& State, FFinalEffectExecutionSummary& Summary)
{
	for (const FFinalBattleCharacterState& CharacterState : State.Characters)
	{
		if (CharacterState.bCollapsed)
		{
			continue;
		}

		for (const FFinalBattleTriggerDefinition& TriggerDefinition : CharacterState.BattleTriggers)
		{
			if (TriggerDefinition.TriggerWindow != EFinalBattleTriggerWindow::OwnerTookHealthDamage
				|| TriggerDefinition.Effects.IsEmpty())
			{
				continue;
			}

			ExecuteEffectList(
				State,
				TriggerDefinition.Effects,
				nullptr,
				nullptr,
				&CharacterState,
				nullptr,
				Summary);
		}
	}
}

int32 ApplyTeamIncomingDamageAndTriggers(
	FFinalBattleState& State,
	const int32 TotalIncomingDamage,
	FFinalEffectExecutionSummary& Summary)
{
	const int32 HpDamage = ApplyTeamIncomingDamage(State, TotalIncomingDamage);
	if (HpDamage > 0)
	{
		TArray<FFinalBattleEvent> RelicEvents;
		GetRelicService().HandlePlayerTeamTookHealthDamage(State, HpDamage, RelicEvents);
		for (const FFinalBattleEvent& RelicEvent : RelicEvents)
		{
			AppendBattleEvent(State, RelicEvent);
		}

		ExecuteOwnerTookHealthDamageTriggers(State, Summary);
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
	FFinalEffectExecutionSummary& Summary)
{
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

		const FGuid GeneratedCardInstanceId = GetCardService().AddGeneratedCardToHand(
			State,
			SelectedCardDefinition,
			SourceOwnerUnitId,
			GenerateCardEffect->bGeneratedCard,
			GenerateCardEffect->bTemporaryCard,
			GenerateCardEffect->bRetainInHand,
			GenerateCardEffect->bConsumeOnPlay);
		if (GeneratedCardInstanceId.IsValid())
		{
			++GeneratedCardCount;
		}
	}

	if (GeneratedCardCount <= 0)
	{
		return false;
	}

	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteConsumeGeneratedCardEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectConsumeGeneratedCard* ConsumeGeneratedCardEffect,
	const FName SourceOwnerUnitId,
	FFinalEffectExecutionContext& ExecutionContext,
	FFinalEffectExecutionSummary& Summary)
{
	const int32 TargetConsumeCount = FMath::Max(ConsumeGeneratedCardEffect->ConsumeCount, 0);
	if (TargetConsumeCount <= 0 || SourceOwnerUnitId.IsNone())
	{
		return false;
	}

	TArray<FGuid> ConsumedCardInstanceIds;
	const int32 ConsumedCount = GetCardService().ConsumeMatchingCardsFromHand(
		State,
		SourceOwnerUnitId,
		ConsumeGeneratedCardEffect->RequiredCardId,
		ConsumeGeneratedCardEffect->RequiredKeyword,
		TargetConsumeCount,
		ConsumeGeneratedCardEffect->bGeneratedOnly,
		&ConsumedCardInstanceIds);
	if (ConsumedCount <= 0)
	{
		return false;
	}

	for (const FGuid& ConsumedCardInstanceId : ConsumedCardInstanceIds)
	{
		if (const FFinalBattleCardInstance* ConsumedCardInstance = GetCardService().FindCardInstance(State, ConsumedCardInstanceId))
		{
			RecordConsumedGeneratedCard(ExecutionContext, SourceOwnerUnitId, *ConsumedCardInstance, 1);
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
	FFinalEffectExecutionSummary& Summary)
{
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
		if (FFinalBattleEnemyState* TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, HealEffect->UnitTargetRule))
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
	FFinalEffectExecutionSummary& Summary)
{
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
		SourceEnemyState);
	int32 AppliedStacks = 0;
	for (const FName TargetOwnerUnitId : TargetOwnerUnitIds)
	{
		AppliedStacks += GetStatusService().AddStatusStacks(
			State,
			TargetOwnerUnitId,
			SourceOwnerUnitId,
			StatusId,
			ApplyStatusEffect->StatusDefinition,
			ApplyStatusEffect->Stacks,
			ApplyStatusEffect->DurationOverride);
	}

	if (AppliedStacks <= 0)
	{
		return false;
	}

	Summary.TotalStatusStacksApplied += AppliedStacks;
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteRemoveStatusEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectRemoveStatus* RemoveStatusEffect,
	const FFinalBattleCommand* Command,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	FFinalEffectExecutionContext& ExecutionContext,
	FFinalEffectExecutionSummary& Summary)
{
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
		SourceEnemyState);
	int32 RemovedStacks = 0;
	for (const FName TargetOwnerUnitId : TargetOwnerUnitIds)
	{
		const int32 RemovedStacksForTarget = GetStatusService().RemoveStatusStacks(
			State,
			TargetOwnerUnitId,
			StatusId,
			RemoveStatusEffect->Stacks);
		RemovedStacks += RemovedStacksForTarget;
		RecordConsumedStatus(ExecutionContext, TargetOwnerUnitId, StatusId, RemovedStacksForTarget);
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
	FFinalEffectExecutionSummary& Summary)
{
	if (!SatisfiesHandCardRequirement(ShieldEffect->HandCardRequirement, State, SourceOwnerUnitId))
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
	const FFinalEffectExecutionContext& ExecutionContext,
	FFinalEffectExecutionSummary& Summary)
{
	if (!SatisfiesConsumeRequirement(DrawCardsEffect->ConsumeRequirement, ExecutionContext, SourceOwnerUnitId))
	{
		return false;
	}

	if (!SatisfiesHandCardRequirement(DrawCardsEffect->HandCardRequirement, State, SourceOwnerUnitId))
	{
		return false;
	}

	if (!SatisfiesGeneratedCardConsumeRequirement(DrawCardsEffect->GeneratedCardConsumeRequirement, ExecutionContext, SourceOwnerUnitId))
	{
		return false;
	}

	const int32 HandCountBeforeDraw = State.DeckState.HandCardInstanceIds.Num();
	DrawCards(State, FMath::Max(DrawCardsEffect->DrawCount, 0));
	Summary.TotalCardsDrawn += FMath::Max(State.DeckState.HandCardInstanceIds.Num() - HandCountBeforeDraw, 0);
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteGainAPEffect(
	FFinalBattleState& State,
	const UFinalBattleEffectGainAP* GainAPEffect,
	const FName SourceOwnerUnitId,
	const FFinalEffectExecutionContext& ExecutionContext,
	FFinalEffectExecutionSummary& Summary)
{
	if (!SatisfiesConsumeRequirement(GainAPEffect->ConsumeRequirement, ExecutionContext, SourceOwnerUnitId))
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
	FFinalEffectExecutionContext& ExecutionContext,
	FFinalEffectExecutionSummary& Summary)
{
	if (!SatisfiesGeneratedCardConsumeRequirement(DamageEffect->GeneratedCardConsumeRequirement, ExecutionContext, SourceOwnerUnitId))
	{
		return false;
	}

	const int32 HitCount = FMath::Max(DamageEffect->HitCount, 1);
	const int32 BaseDamagePerHit = ResolveScalarValue(DamageEffect->Scalar, SourceCharacterState, SourceEnemyState);
	const int32 DamageModifierPercent = SourceCharacterState != nullptr
		? GetStatusService().GetOutgoingDamageModifierPercent(State, SourceOwnerUnitId, bIsAttackCardDamage)
		: 0;
	const int32 DamagePerHit = ApplyOutgoingDamageModifier(BaseDamagePerHit, DamageModifierPercent);
	if (DamagePerHit <= 0)
	{
		return false;
	}

	if (DamageEffect->UnitTargetRule == EFinalBattleUnitTargetRule::TeamPlayer)
	{
		if (!SatisfiesTargetStateRequirement(DamageEffect->TargetStateRequirement, nullptr))
		{
			return false;
		}

		for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
		{
			const int32 HpDamage = ApplyTeamIncomingDamageAndTriggers(State, DamagePerHit, Summary);
			Summary.TotalDamageToTeam += HpDamage;
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

			if (!SatisfiesTargetStateRequirement(DamageEffect->TargetStateRequirement, &EnemyState))
			{
				continue;
			}

			for (int32 HitIndex = 0; HitIndex < HitCount && EnemyState.CurrentHP > 0; ++HitIndex)
			{
				const int32 HpDamage = ApplyDamageToEnemy(State, EnemyState, DamagePerHit);
				ExecutionContext.bAppliedSuccessfulEnemyHpDamage |= HpDamage > 0;
				Summary.TotalDamageToEnemies += DamagePerHit;
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
		TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, DamageEffect->UnitTargetRule);
	}

	if (TargetEnemyState == nullptr)
	{
		return false;
	}

	if (!SatisfiesTargetStateRequirement(DamageEffect->TargetStateRequirement, TargetEnemyState))
	{
		return false;
	}

	for (int32 HitIndex = 0; HitIndex < HitCount && TargetEnemyState->CurrentHP > 0; ++HitIndex)
	{
		const int32 HpDamage = ApplyDamageToEnemy(State, *TargetEnemyState, DamagePerHit);
		ExecutionContext.bAppliedSuccessfulEnemyHpDamage |= HpDamage > 0;
		Summary.TotalDamageToEnemies += DamagePerHit;
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
	const FFinalEffectExecutionContext& ExecutionContext,
	FFinalEffectExecutionSummary& Summary)
{
	if (!SatisfiesConsumeRequirement(BonusBreakEffect->ConsumeRequirement, ExecutionContext, SourceOwnerUnitId))
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
		for (FFinalBattleEnemyState& EnemyState : State.Enemies)
		{
			if (EnemyState.CurrentHP <= 0)
			{
				continue;
			}

			Summary.TotalBreakDamageToEnemies += ApplyBonusBreakToEnemy(EnemyState, BreakAmount);
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
		TargetEnemyState = ResolvePrimaryEnemyTarget(State, Command, BonusBreakEffect->UnitTargetRule);
	}

	if (TargetEnemyState == nullptr)
	{
		return false;
	}

	Summary.TotalBreakDamageToEnemies += ApplyBonusBreakToEnemy(*TargetEnemyState, BreakAmount);
	++Summary.ResolvedEffectCount;
	return true;
}

bool ExecuteEffectList(
	FFinalBattleState& State,
	const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
	const FFinalBattleCommand* Command,
	const UFinalCardDefinition* SourceCardDefinition,
	const FFinalBattleCharacterState* SourceCharacterState,
	FFinalBattleEnemyState* SourceEnemyState,
	FFinalEffectExecutionSummary& Summary)
{
	FFinalEffectExecutionContext ExecutionContext;
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
			ExecuteDamageEffect(State, DamageEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, bIsAttackCardDamage, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectGenerateCard* GenerateCardEffect = Cast<UFinalBattleEffectGenerateCard>(EffectDefinition))
		{
			ExecuteGenerateCardEffect(State, GenerateCardEffect, SourceOwnerUnitId, Summary);
			continue;
		}

		if (const UFinalBattleEffectConsumeGeneratedCard* ConsumeGeneratedCardEffect = Cast<UFinalBattleEffectConsumeGeneratedCard>(EffectDefinition))
		{
			ExecuteConsumeGeneratedCardEffect(State, ConsumeGeneratedCardEffect, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectHeal* HealEffect = Cast<UFinalBattleEffectHeal>(EffectDefinition))
		{
			ExecuteHealEffect(State, HealEffect, Command, SourceCharacterState, SourceEnemyState, Summary);
			continue;
		}

		if (const UFinalBattleEffectApplyStatus* ApplyStatusEffect = Cast<UFinalBattleEffectApplyStatus>(EffectDefinition))
		{
			ExecuteApplyStatusEffect(State, ApplyStatusEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, Summary);
			continue;
		}

		if (const UFinalBattleEffectRemoveStatus* RemoveStatusEffect = Cast<UFinalBattleEffectRemoveStatus>(EffectDefinition))
		{
			ExecuteRemoveStatusEffect(State, RemoveStatusEffect, Command, SourceCharacterState, SourceEnemyState, ExecutionContext, Summary);
			continue;
		}

		if (const UFinalBattleEffectGainShield* ShieldEffect = Cast<UFinalBattleEffectGainShield>(EffectDefinition))
		{
			ExecuteGainShieldEffect(State, ShieldEffect, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, Summary);
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
			ExecuteBonusBreakEffect(State, BonusBreakEffect, Command, SourceCharacterState, SourceEnemyState, SourceOwnerUnitId, ExecutionContext, Summary);
			continue;
		}
	}

	if (SourceCharacterState != nullptr && ExecutionContext.bAppliedSuccessfulEnemyHpDamage)
	{
		GetStatusService().ConsumeOutgoingDamageModifierStacks(State, SourceOwnerUnitId, bIsAttackCardDamage);
	}

	return Summary.ResolvedEffectCount > 0;
}
}

void FFinalBattleResolver::Initialize(FFinalBattleState& State, const UFinalBattleEncounterDefinition* EncounterDefinition, const UFinalBattleRuleConfig* RuleConfig, const FFinalBattleInitContext& InitContext) const
{
	State = FFinalBattleState{};
	State.BattleId = FGuid::NewGuid();
	State.CurrentRound = 1;
	GetResourceService().InitializeBattleResources(State, RuleConfig);
	GetCardService().InitializeDeckState(State.DeckState);
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
		CharacterState.BattleTriggers = PartyEntry.CharacterDefinition->BattleTriggers;
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

	GetCardService().InitializeDeckCards(State, InitContext.DeckDefinitions, TemplateToRuntimeUnitMap);

	const int32 InitialHandSize = RuleConfig ? FMath::Max(RuleConfig->InitialHandSize, 0) : 0;
	GetCardService().DrawCards(State, FMath::Min(InitialHandSize, State.DeckState.DrawPileCardInstanceIds.Num()));

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

	TArray<FFinalBattleEvent> BattleStartRelicEvents;
	GetRelicService().InitializeRelics(State, InitContext.BattleStartRelics, BattleStartRelicEvents);
	for (const FFinalBattleEvent& RelicEvent : BattleStartRelicEvents)
	{
		AppendBattleEvent(State, RelicEvent);
	}

	UE_LOG(LogFinalBattleResolver, Log, TEXT("Initialized battle session with %d enemy entries."), State.Enemies.Num());
}

FName FFinalBattleResolver::MakePlayerUnitId(int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_player_%d"), Index + 1));
}

FFinalBattleEvent FFinalBattleResolver::ExecutePlayCardCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	FFinalBattleCardInstance* CardInstance = GetCardService().FindCardInstance(State, Command.CardInstanceId);
	if (CardInstance == nullptr)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::CardInstanceNotFound,
			RejectCardInstanceMissingTag,
			FText::FromString(TEXT("Card instance was not found.")));
		Event.CardInstanceId = Command.CardInstanceId;
		return FinalizeBattleEvent(State, Event);
	}

	if (CardInstance->SourceDefinition == nullptr)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::CardDefinitionMissing,
			RejectCardDefinitionMissingTag,
			FText::FromString(TEXT("Card definition is missing for the selected card.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		return FinalizeBattleEvent(State, Event);
	}

	if (!GetCardService().IsCardInHand(State, Command.CardInstanceId))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::CardNotInHand,
			RejectCardNotInHandTag,
			FText::FromString(TEXT("Card instance is not in hand.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		return FinalizeBattleEvent(State, Event);
	}

	if (!GetResourceService().HasEnoughAP(State, CardInstance->RuntimeCostAP))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::NotEnoughAP,
			RejectNotEnoughAPTag,
			FText::FromString(TEXT("Not enough AP to play the selected card.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		Event.PrimaryValue = CardInstance->RuntimeCostAP;
		Event.SecondaryValue = State.CurrentAP;
		return FinalizeBattleEvent(State, Event);
	}

	if (!HasSupportedEffect(CardInstance->SourceDefinition))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UnsupportedCardEffects,
			RejectUnsupportedCardEffectsTag,
			FText::FromString(TEXT("Selected card has no supported effects.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		return FinalizeBattleEvent(State, Event);
	}

	const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, CardInstance->RuntimeOwnerUnitId);
	const UFinalCardDefinition* SourceCardDefinition = CardInstance->SourceDefinition;
	const FName ResolvedRuntimeOwnerUnitId = CardInstance->RuntimeOwnerUnitId;
	const FGuid ResolvedCardInstanceId = CardInstance->CardInstanceId;
	const FFinalCardId ResolvedCardId = CardInstance->CardId;

	FFinalBattleResolvedCardTriggerContext RelicCardContext;
	RelicCardContext.RuntimeOwnerUnitId = ResolvedRuntimeOwnerUnitId;
	RelicCardContext.CardId = ResolvedCardId;
	RelicCardContext.CardType = SourceCardDefinition->CardType;
	RelicCardContext.RuntimeCostAP = CardInstance->RuntimeCostAP;
	RelicCardContext.RuntimeKeywords = CardInstance->RuntimeKeywords;
	RelicCardContext.bGeneratedCard = CardInstance->bGeneratedCard;

	GetResourceService().SpendAP(State, CardInstance->RuntimeCostAP);
	GetResourceService().GainCardPlayEP(State, RuleConfig);
	GetCardService().MoveHandCardAfterPlay(State, Command.CardInstanceId);

	FFinalEffectExecutionSummary Summary;
	ExecuteEffectList(State, SourceCardDefinition->Effects, &Command, SourceCardDefinition, OwnerCharacterState, nullptr, Summary);

	TArray<FFinalBattleEvent> RelicEvents;
	GetRelicService().HandlePlayerCardResolved(State, RelicCardContext, GetCardService(), RelicEvents);
	for (const FFinalBattleEvent& RelicEvent : RelicEvents)
	{
		AppendBattleEvent(State, RelicEvent);
	}

	Event.EventType = EFinalBattleEventType::CardResolved;
	Event.SourceUnitId = ResolvedRuntimeOwnerUnitId;
	Event.TargetUnitId = ResolveCommandTargetUnitId(State, Command);
	Event.CardInstanceId = ResolvedCardInstanceId;
	Event.CardId = ResolvedCardId;
	Event.PrimaryValue = Summary.TotalDamageToEnemies;
	Event.SecondaryValue = Summary.TotalCardsDrawn;

	if (AreAllEnemiesDefeated(State))
	{
		MarkBattleResolved(State, true);
		Event.Message = FText::Format(
			NSLOCTEXT("FinalBattleResolver", "CardPlayVictory", "Resolved {0} effects. Damage {1}, Break {2}, Heal {3}, Shield {4}, Draw {5}, AP {6}. Battle won."),
			FText::AsNumber(Summary.ResolvedEffectCount),
			FText::AsNumber(Summary.TotalDamageToEnemies),
			FText::AsNumber(Summary.TotalBreakDamageToEnemies),
			FText::AsNumber(Summary.TotalHealingToTeam),
			FText::AsNumber(Summary.TotalTeamShieldGained),
			FText::AsNumber(Summary.TotalCardsDrawn),
			FText::AsNumber(Summary.TotalAPGained));
		return FinalizeBattleEvent(State, Event);
	}

	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "CardPlayAccepted", "Resolved {0} effects. Damage {1}, Break {2}, Heal {3}, Shield {4}, Draw {5}, AP {6}."),
		FText::AsNumber(Summary.ResolvedEffectCount),
		FText::AsNumber(Summary.TotalDamageToEnemies),
		FText::AsNumber(Summary.TotalBreakDamageToEnemies),
		FText::AsNumber(Summary.TotalHealingToTeam),
		FText::AsNumber(Summary.TotalTeamShieldGained),
		FText::AsNumber(Summary.TotalCardsDrawn),
		FText::AsNumber(Summary.TotalAPGained));
	return FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecutePlayUltimateCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, Command.UltimateOwnerUnitId);
	if (OwnerCharacterState == nullptr)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateOwnerNotFound,
			RejectUltimateOwnerMissingTag,
			FText::FromString(TEXT("Ultimate owner was not found.")));
		Event.SourceUnitId = Command.UltimateOwnerUnitId;
		return FinalizeBattleEvent(State, Event);
	}

	Event.SourceUnitId = OwnerCharacterState->RuntimeUnitId;
	Event.UltimateId = OwnerCharacterState->UltimateId;

	if (OwnerCharacterState->bCollapsed)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateBlockedByCollapse,
			RejectUltimateBlockedByCollapseTag,
			FText::FromString(TEXT("Collapsed characters cannot use ultimates.")));
		return FinalizeBattleEvent(State, Event);
	}

	if (OwnerCharacterState->bUltimateUsedThisBattle)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateAlreadyUsed,
			RejectUltimateAlreadyUsedTag,
			FText::FromString(TEXT("Ultimate was already used this battle.")));
		return FinalizeBattleEvent(State, Event);
	}

	if (OwnerCharacterState->UltimateDefinition == nullptr || !HasSupportedEffect(OwnerCharacterState->UltimateDefinition))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateDefinitionUnavailable,
			RejectUltimateDefinitionMissingTag,
			FText::FromString(TEXT("Ultimate definition is unavailable or has no supported effects.")));
		return FinalizeBattleEvent(State, Event);
	}

	if (!GetResourceService().HasEnoughEP(State, OwnerCharacterState->UltimateCostEP))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::NotEnoughEP,
			RejectNotEnoughEPTag,
			FText::FromString(TEXT("Not enough EP to use the selected ultimate.")));
		Event.PrimaryValue = OwnerCharacterState->UltimateCostEP;
		Event.SecondaryValue = State.CurrentEP;
		return FinalizeBattleEvent(State, Event);
	}

	GetResourceService().SpendEP(State, OwnerCharacterState->UltimateCostEP);

	FFinalEffectExecutionSummary Summary;
	ExecuteEffectList(State, OwnerCharacterState->UltimateDefinition->Effects, &Command, nullptr, OwnerCharacterState, nullptr, Summary);

	Event.EventType = EFinalBattleEventType::UltimateResolved;
	Event.TargetUnitId = ResolveCommandTargetUnitId(State, Command);
	Event.PrimaryValue = Summary.TotalDamageToEnemies;
	Event.SecondaryValue = Summary.TotalTeamShieldGained;
	if (FFinalBattleCharacterState* MutableOwnerCharacterState = State.Characters.FindByPredicate(
		[&Command](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == Command.UltimateOwnerUnitId;
		}))
	{
		MutableOwnerCharacterState->bUltimateUsedThisBattle = true;
	}

	if (AreAllEnemiesDefeated(State))
	{
		MarkBattleResolved(State, true);
		Event.Message = FText::Format(
			NSLOCTEXT("FinalBattleResolver", "UltimateVictory", "Ultimate resolved. Damage {0}, Break {1}, Heal {2}, Shield {3}, AP {4}. Battle won."),
			FText::AsNumber(Summary.TotalDamageToEnemies),
			FText::AsNumber(Summary.TotalBreakDamageToEnemies),
			FText::AsNumber(Summary.TotalHealingToTeam),
			FText::AsNumber(Summary.TotalTeamShieldGained),
			FText::AsNumber(Summary.TotalAPGained));
		return FinalizeBattleEvent(State, Event);
	}

	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "UltimateResolved", "Ultimate resolved. Damage {0}, Break {1}, Heal {2}, Shield {3}, AP {4}."),
		FText::AsNumber(Summary.TotalDamageToEnemies),
		FText::AsNumber(Summary.TotalBreakDamageToEnemies),
		FText::AsNumber(Summary.TotalHealingToTeam),
		FText::AsNumber(Summary.TotalTeamShieldGained),
		FText::AsNumber(Summary.TotalAPGained));
	return FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteEndTurnCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	const FFinalBattleEndTurnResult EndTurnResult = GetTurnService().ResolveEndTurn(
		State,
		RuleConfig,
		GetCardService(),
		GetRelicService(),
		GetResourceService(),
		GetStatusService(),
		[](FFinalBattleState& MutableState, FFinalBattleEnemyState& EnemyState) -> FFinalBattleEnemyActionResult
		{
			FFinalBattleEnemyActionResult ActionResult;
			FFinalEffectExecutionSummary Summary;

			if (EnemyState.CurrentIntentDefinition && HasSupportedEffectList(EnemyState.CurrentIntentDefinition->Effects))
			{
				ExecuteEffectList(MutableState, EnemyState.CurrentIntentDefinition->Effects, nullptr, nullptr, nullptr, &EnemyState, Summary);
			}
			else
			{
				const int32 HpDamage = ApplyTeamIncomingDamageAndTriggers(
					MutableState,
					FMath::Max(EnemyState.RuntimeDamagePower, 0),
					Summary);
				Summary.TotalDamageToTeam += HpDamage;
			}

			ActionResult.DamageToTeam = Summary.TotalDamageToTeam;
			ActionResult.EnemyShieldGained = Summary.TotalEnemyShieldGained;
			ActionResult.ResolvedEffectCount = Summary.ResolvedEffectCount;
			return ActionResult;
		});

	for (const FFinalBattleEvent& GeneratedEvent : EndTurnResult.GeneratedEvents)
	{
		AppendBattleEvent(State, GeneratedEvent);
	}

	if (EndTurnResult.bBattleLost)
	{
		MarkBattleResolved(State, false);
		Event.EventType = EFinalBattleEventType::BattleResolved;
		Event.TargetUnitId = TeamPlayerUnitId;
		Event.PrimaryValue = EndTurnResult.TotalDamageToTeam;
		Event.SecondaryValue = EndTurnResult.TotalEnemyShieldGained;
		Event.Message = FText::Format(
			NSLOCTEXT("FinalBattleResolver", "EndTurnDefeat", "Enemies resolved {0} effects. Team damage {1}, enemy shield {2}. Battle lost."),
			FText::AsNumber(EndTurnResult.ResolvedEffectCount),
			FText::AsNumber(EndTurnResult.TotalDamageToTeam),
			FText::AsNumber(EndTurnResult.TotalEnemyShieldGained));
		return FinalizeBattleEvent(State, Event);
	}

	Event.EventType = EFinalBattleEventType::TurnTransition;
	Event.TargetUnitId = TeamPlayerUnitId;
	Event.PrimaryValue = EndTurnResult.TotalDamageToTeam;
	Event.SecondaryValue = EndTurnResult.TotalEnemyShieldGained;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "EndTurnAdvanced", "Turn advanced. Enemies resolved {0} effects. Team damage {1}, enemy shield {2}, team shield now {3}."),
		FText::AsNumber(EndTurnResult.ResolvedEffectCount),
		FText::AsNumber(EndTurnResult.TotalDamageToTeam),
		FText::AsNumber(EndTurnResult.TotalEnemyShieldGained),
		FText::AsNumber(State.TeamShield));
	Event.Round = State.CurrentRound;
	return FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteSelectTargetCommand(FFinalBattleState& State, const FFinalBattleCommand& Command) const
{
	FFinalBattleEvent Event;

	FFinalBattleEnemyState* SelectedEnemy = FindEnemyState(State, Command.TargetUnitId);
	if (SelectedEnemy == nullptr || SelectedEnemy->CurrentHP <= 0)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::InvalidTarget,
			RejectInvalidTargetTag,
			FText::FromString(TEXT("Target is not a valid living enemy.")));
		Event.TargetUnitId = Command.TargetUnitId;
		return FinalizeBattleEvent(State, Event);
	}

	State.CurrentTargetUnitId = Command.TargetUnitId;
	Event.EventType = EFinalBattleEventType::TargetChanged;
	Event.TargetUnitId = Command.TargetUnitId;
	Event.Message = FText::FromString(TEXT("Target updated."));
	return FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteUnsupportedCommand(FFinalBattleState& State) const
{
	FFinalBattleEvent Event = BuildRejectedCommandEvent(
		EFinalBattleCommandRejectReason::UnsupportedCommand,
		RejectUnsupportedCommandTag,
		FText::FromString(TEXT("Unsupported command.")));
	return FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	if (State.bBattleEnded)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::BattleAlreadyResolved,
			RejectBattleResolvedTag,
			FText::FromString(TEXT("Battle is already resolved.")));
		return FinalizeBattleEvent(State, Event);
	}

	switch (Command.CommandType)
	{
	case EFinalBattleCommandType::PlayCard:
		return ExecutePlayCardCommand(State, Command, RuleConfig);

	case EFinalBattleCommandType::PlayUltimate:
		return ExecutePlayUltimateCommand(State, Command, RuleConfig);

	case EFinalBattleCommandType::EndTurn:
		return ExecuteEndTurnCommand(State, Command, RuleConfig);

	case EFinalBattleCommandType::SelectTarget:
		return ExecuteSelectTargetCommand(State, Command);

	default:
		return ExecuteUnsupportedCommand(State);
	}
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

	GetStatusService().BuildStatusSnapshotData(State, Snapshot.CharacterStatuses, Snapshot.TeamStatuses, Snapshot.Statuses);
	GetCardService().BuildHandCardViews(State, Snapshot.HandCards);

	return Snapshot;
}

FName FFinalBattleResolver::MakeEnemyUnitId(int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_enemy_%d"), Index + 1));
}
