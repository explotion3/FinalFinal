#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"

class UFinalCardDefinition;
class UFinalCharacterDefinition;
class UFinalUltimateDefinition;
class UFinalBattleConditionDefinition;
class UFinalBattleEffectDefinition;

struct FINALBATTLE_API FFinalBattleCharacterRuntimeStats
{
	FFinalCharacterId CharacterId;
	int32 VitalShare = 0;
	int32 StressCap = 0;
	int32 RuntimeAttack = 0;
	int32 RuntimeDefense = 0;
	float RuntimeBreakRate = 0.0f;
	float RuntimeCritChance = 0.0f;
	float RuntimeCritDamage = 1.5f;
};

struct FINALBATTLE_API FFinalBattleCharacterInitData
{
	UFinalCharacterDefinition* CharacterDefinition = nullptr;
	UFinalUltimateDefinition* UltimateDefinition = nullptr;
	int32 CurrentStress = 0;
	bool bCollapsed = false;
	int32 CurrentAwakenCount = 0;
	int32 CollapseCount = 0;
	int32 VitalShare = 0;
	int32 StressCap = 0;
	int32 RuntimeAttack = 0;
	int32 RuntimeDefense = 0;
	float RuntimeBreakRate = 0.0f;
	float RuntimeCritChance = 0.0f;
	float RuntimeCritDamage = 1.5f;
};

struct FINALBATTLE_API FFinalBattleCardInitData
{
	UFinalCardDefinition* CardDefinition = nullptr;
	FFinalCardId CardId;
	FFinalCharacterId OwnerCharacterId;
	FName SourceRunCardInstanceId = NAME_None;
};

struct FINALBATTLE_API FFinalBattleCardRefreshRequest
{
	FName SourceRunCardInstanceId = NAME_None;
	FFinalCardId NewCardId;
	UFinalCardDefinition* NewDefinition = nullptr;
};

enum class EFinalBattleCardModifierSourceType : uint8
{
	Card,
	Status,
	Passive,
	Relic,
	System
};

enum class EFinalBattleCardModifierDuration : uint8
{
	UntilPlayed,
	EndOfTurn,
	EndOfRound,
	EndOfBattle,
	ManualClear
};

enum class EFinalBattleCardEffectPatchOperation : uint8
{
	Replace,
	InsertBefore,
	InsertAfter,
	Remove
};

struct FINALBATTLE_API FFinalBattleCardEffectPatch
{
	FName TargetEffectId = NAME_None;
	EFinalBattleCardEffectPatchOperation Operation = EFinalBattleCardEffectPatchOperation::Replace;
	TObjectPtr<UFinalBattleEffectDefinition> EffectDefinition = nullptr;
};

struct FINALBATTLE_API FFinalBattleCardConditionPatch
{
	FName TargetEffectId = NAME_None;
	FName TargetConditionId = NAME_None;
	EFinalBattleCardEffectPatchOperation Operation = EFinalBattleCardEffectPatchOperation::Replace;
	TObjectPtr<UFinalBattleConditionDefinition> ConditionDefinition = nullptr;
};

struct FINALBATTLE_API FFinalBattleCardModifierRecord
{
	FName ModifierId = NAME_None;
	EFinalBattleCardModifierSourceType SourceType = EFinalBattleCardModifierSourceType::System;
	EFinalBattleCardModifierDuration DurationPolicy = EFinalBattleCardModifierDuration::ManualClear;
	int32 ApplyOrder = 0;
	int32 CostDeltaAP = 0;
	FGameplayTagContainer AddedKeywords;
	FGameplayTagContainer RemovedKeywords;
	bool bOverrideRetained = false;
	bool bRetained = false;
	bool bOverrideConsumeOnPlay = false;
	bool bConsumeOnPlay = false;
	bool bOverrideRecycleCount = false;
	int32 RecycleCount = 0;
	int32 OutgoingDamagePercentDelta = 0;
	bool bReplaceEntireEffectList = false;
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> ReplacementEffects;
	TArray<FFinalBattleCardEffectPatch> EffectPatches;
	TArray<FFinalBattleCardConditionPatch> ConditionPatches;
};

struct FINALBATTLE_API FFinalBattleCardProjectionView
{
	FGuid CardInstanceId;
	FFinalCardId CardId;
	int32 EffectiveCostAP = 0;
	FGameplayTagContainer EffectiveKeywords;
	bool bRetained = false;
	bool bConsumeOnPlay = false;
	int32 RecycleCount = 0;
	int32 EffectiveOutgoingDamagePercent = 0;
	int32 EffectCount = 0;
	int32 ModifierCount = 0;
	bool bHasProjectedDefinition = false;
};

struct FINALBATTLE_API FFinalBattleInitContext
{
	int32 TeamCurrentHP = 0;
	TArray<FFinalBattleCharacterInitData> PartyMembers;
	TArray<FFinalBattleCardInitData> DeckCards;
	TArray<UFinalCardDefinition*> DeckDefinitions;
	TArray<FFinalBattleStartRelicInput> BattleStartRelics;
};
