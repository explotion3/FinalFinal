#include "Systems/FinalBattleInitializationService.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEventService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattlePassiveService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"
#include "Systems/FinalEnemyIntentService.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalBattleInitializationService, Log, All);

namespace
{
const FName PassiveAppliedInitialGrantReasonTag(TEXT("passive.applied.initial_grant"));

FName MakePlayerUnitId(const int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_player_%d"), Index + 1));
}

FName MakeEnemyUnitId(const int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_enemy_%d"), Index + 1));
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

void RefreshEnemyIntentState(
	const FFinalEnemyIntentService& EnemyIntentService,
	FFinalBattleEnemyState& EnemyState,
	const int32 PreviewRound)
{
	EnemyIntentService.RefreshIntent(EnemyState, PreviewRound);
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
		NSLOCTEXT("FinalBattleInitializationService", "PassiveAppliedMessage", "获得被动：{0}。"),
		ApplyResult.DisplayName);
	return Event;
}
}

void FFinalBattleInitializationService::InitializeBattle(
	FFinalBattleState& State,
	const UFinalBattleEncounterDefinition* EncounterDefinition,
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleInitContext& InitContext,
	const FFinalBattleCardService& CardService,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEventService& EventService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleRelicService& RelicService,
	const FFinalBattleResourceService& ResourceService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleUnitService& UnitService,
	const FFinalEnemyIntentService& EnemyIntentService) const
{
	UObject* RuntimeProjectionOwner = State.RuntimeProjectionOwner;
	State = FFinalBattleState{};
	State.RuntimeProjectionOwner = RuntimeProjectionOwner;
	State.BattleId = FGuid::NewGuid();
	State.CurrentRound = 1;
	ResourceService.InitializeBattleResources(State, RuleConfig);
	CardService.InitializeDeckState(State.DeckState);
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
		CharacterState.StressCap = PartyEntry.StressCap;
		CharacterState.bCollapsed = PartyEntry.bCollapsed;
		CharacterState.CurrentAwakenCount = PartyEntry.CurrentAwakenCount;
		CharacterState.CollapseCount = PartyEntry.CollapseCount;
		CharacterState.CurrentAwakenThreshold = ResolveAwakenThreshold(CharacterState, RuleConfig);
		CharacterState.VitalShare = PartyEntry.VitalShare;
		CharacterState.RuntimeAttack = PartyEntry.RuntimeAttack;
		CharacterState.RuntimeDefense = PartyEntry.RuntimeDefense;
		CharacterState.RuntimeBreakRate = PartyEntry.RuntimeBreakRate;
		CharacterState.RuntimeCritChance = PartyEntry.RuntimeCritChance;
		CharacterState.RuntimeCritDamage = PartyEntry.RuntimeCritDamage;
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
			State.TeamMaxHP += PartyEntry.VitalShare;
		}

		for (const FFinalBattleInitialPassiveGrantData& InitialPassiveGrant : PartyEntry.InitialPassiveGrants)
		{
			if (!InitialPassiveGrant.PassiveId.IsValid() || InitialPassiveGrant.PassiveDefinition == nullptr || InitialPassiveGrant.InitialStacks <= 0)
			{
				continue;
			}

			const FFinalBattlePassiveApplyResult ApplyResult = GetPassiveService().ApplyPassive(
				State,
				State.Characters.Last().RuntimeUnitId,
				State.Characters.Last().RuntimeUnitId,
				InitialPassiveGrant.PassiveId,
				InitialPassiveGrant.PassiveDefinition,
				InitialPassiveGrant.InitialStacks,
				InitialPassiveGrant.DurationOverride);
			if (ApplyResult.bApplied)
			{
				EventService.AppendBattleEvent(State, BuildPassiveAppliedEvent(ApplyResult, PassiveAppliedInitialGrantReasonTag));
			}
		}
	}

	State.TeamCurrentHP = InitContext.TeamCurrentHP > 0
		? FMath::Min(InitContext.TeamCurrentHP, State.TeamMaxHP)
		: State.TeamMaxHP;

	CardService.InitializeDeckCards(State, InitContext.DeckCards, State.RuntimeProjectionOwner, TemplateToRuntimeUnitMap);
	CardService.PrepareInitialDrawPile(State);

	const int32 InitialHandSize = RuleConfig ? FMath::Max(RuleConfig->InitialHandSize, 0) : 0;
	CardService.DrawCards(State, InitialHandSize);

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
			EnemyState.ScriptedIntentSequence.Reset();
			for (const FFinalEnemyScriptedIntentStep& ScriptedStep : LoadedEnemy->ScriptedIntentSequence)
			{
				FFinalBattleEnemyScriptedIntentRuntimeStep& RuntimeStep = EnemyState.ScriptedIntentSequence.AddDefaulted_GetRef();
				RuntimeStep.IntentId = ScriptedStep.IntentId;
				RuntimeStep.PhaseTag = ScriptedStep.PhaseTag;
				RuntimeStep.bRepeatLastStep = ScriptedStep.bRepeatLastStep;
			}
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

			RefreshEnemyIntentState(EnemyIntentService, EnemyState, State.CurrentRound);
		}
		else
		{
			EnemyState.DisplayName = FText::FromString(TEXT("Missing Enemy Definition"));
		}

		State.Enemies.Add(MoveTemp(EnemyState));
	}

	if (FFinalBattleEnemyState* DefaultTargetEnemy = UnitService.FindFirstAliveEnemy(State))
	{
		State.CurrentTargetUnitId = DefaultTargetEnemy->RuntimeUnitId;
	}

	FFinalBattleEvent SessionStartedEvent;
	SessionStartedEvent.EventType = EFinalBattleEventType::SessionStarted;
	SessionStartedEvent.Message = FText::Format(
		NSLOCTEXT("FinalBattleInitializationService", "SessionStarted", "Battle started: {0}."),
		State.EncounterDisplayName.IsEmpty() ? FText::FromName(State.EncounterId.Value) : State.EncounterDisplayName);
	EventService.FinalizeBattleEvent(State, SessionStartedEvent);

	TArray<FFinalBattleEvent> BattleStartRelicEvents;
	RelicService.InitializeRelics(
		State,
		InitContext.BattleStartRelics,
		TriggerService,
		ConditionService,
		EffectExecutionService,
		UnitService,
		BattleStartRelicEvents);
	for (const FFinalBattleEvent& RelicEvent : BattleStartRelicEvents)
	{
		EventService.AppendBattleEvent(State, RelicEvent);
	}

	UE_LOG(LogFinalBattleInitializationService, Log, TEXT("Initialized battle session with %d enemy entries."), State.Enemies.Num());
}
