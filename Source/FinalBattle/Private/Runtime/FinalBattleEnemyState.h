#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"

class UFinalEnemyIntentDefinition;

struct FFinalBattleEnemyIntentRuntimeState
{
	UFinalEnemyIntentDefinition* Definition = nullptr;
	FName IntentId = NAME_None;
	int32 UseCount = 0;
	int32 NextAvailableRound = 1;
};

struct FFinalBattleEnemyScriptedIntentRuntimeStep
{
	FName IntentId = NAME_None;
	FName PhaseTag = NAME_None;
	bool bRepeatLastStep = false;
};

struct FFinalBattleEnemyState
{
	FName RuntimeUnitId = NAME_None;
	FFinalEnemyId EnemyId;
	FText DisplayName;
	FGameplayTagContainer RoleTags;
	int32 PositionIndex = 0;
	int32 SpawnWave = 1;
	int32 MaxHP = 0;
	int32 CurrentHP = 0;
	int32 CurrentShield = 0;
	int32 MaxBreakValue = 0;
	int32 CurrentBreakValue = 0;
	int32 CurrentInitiative = 0;
	int32 RuntimeDamagePower = 0;
	EFinalIntentSelectRule IntentSelectRule = EFinalIntentSelectRule::Cycle;
	TArray<FFinalEnemyPhaseDefinition> PhaseSequence;
	TArray<FFinalBattleEnemyScriptedIntentRuntimeStep> ScriptedIntentSequence;
	int32 CurrentPhaseIndex = INDEX_NONE;
	FName CurrentPhaseTag = NAME_None;
	FName CurrentIntentId = NAME_None;
	FText CurrentIntentText;
	UFinalEnemyIntentDefinition* CurrentIntentDefinition = nullptr;
	TArray<FFinalBattleEnemyIntentRuntimeState> IntentRuntimeStates;
	int32 CurrentIntentIndex = INDEX_NONE;
	FName LastSelectedIntentId = NAME_None;
	FName LastExecutedIntentId = NAME_None;
	int32 IntentExecutionCount = 0;
	int32 ConsecutiveIntentUseCount = 0;
	bool bActedThisRound = false;
};
