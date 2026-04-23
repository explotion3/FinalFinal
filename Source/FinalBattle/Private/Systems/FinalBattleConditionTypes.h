#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRelicRuntimeTriggerTypes.h"
#include "Types/FinalCoreTypes.h"
#include "Battle/Conditions/Requirements/FinalBattleStatusChangeRequirement.h"

class FFinalBattleCardService;
struct FFinalBattleCardInstance;
struct FFinalBattleEnemyState;
struct FFinalBattleResolvedCardTriggerContext;
struct FFinalBattleState;

struct FFinalBattleStatusChangeRecord
{
	FName OwnerUnitId = NAME_None;
	FFinalStatusId StatusId;
	EFinalBattleStatusChangeKind ChangeKind = EFinalBattleStatusChangeKind::Removed;
	int32 ChangedStacks = 0;
};

struct FFinalBattleMovedCardRecord
{
	FName RuntimeOwnerUnitId = NAME_None;
	FFinalCardId CardId;
	int32 MovedCount = 0;
	bool bGeneratedCard = false;
	FGameplayTagContainer RuntimeKeywords;
	EFinalBattleCardZoneRule SourceZone = EFinalBattleCardZoneRule::Hand;
	EFinalBattleCardZoneRule DestinationZone = EFinalBattleCardZoneRule::ConsumePile;
};

// Persistent for one effect list. Later Conditions may read these facts.
struct FFinalBattleEffectChainRecordContext
{
	TArray<FFinalBattleStatusChangeRecord> StatusChangeRecords;
	TArray<FFinalBattleMovedCardRecord> MovedCardRecords;
};

// Scratch flags for effect-list execution that should not be exposed as records.
struct FFinalBattleEffectTransientContext
{
	bool bAppliedSuccessfulEnemyHpDamage = false;
};

struct FFinalBattleEffectExecutionContext
{
	FFinalBattleEffectChainRecordContext ChainRecords;
	FFinalBattleEffectTransientContext Transient;
};

struct FFinalBattleConditionEvaluationContext
{
	const FFinalBattleState* BattleState = nullptr;
	const FFinalBattleEnemyState* TargetEnemyState = nullptr;
	const FFinalBattleEffectChainRecordContext* ChainRecords = nullptr;
	const FFinalBattleResolvedCardTriggerContext* ResolvedCardContext = nullptr;
	const FFinalBattleCardService* CardService = nullptr;
	FName SourceOwnerUnitId = NAME_None;
};
