#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

enum class EFirstCardEffectType : uint8
{
	None,
	Damage
};

enum class EFirstBattleCommandType : uint8
{
	PlayCard,
	EndTurn
};

enum class EFirstBattleCommandResultCode : uint8
{
	Accepted,
	Rejected
};

enum class EFirstBattleEventType : uint8
{
	None,
	CardPlayed,
	InitiativeChanged,
	PerfectReleaseTriggered,
	EnemyPartActed,
	PlayerDamaged,
	EnemyPartDestroyed,
	BattleWon,
	BattleLost,
	CommandRejected
};

struct FINALBATTLE_API FFirstCardEffectInstance
{
	EFirstCardEffectType EffectType = EFirstCardEffectType::None;
	FName EffectId = NAME_None;
	int32 Value = 0;
};

struct FINALBATTLE_API FFirstCardInstance
{
	FGuid CardInstanceId;
	FName CardId = NAME_None;
	FText DisplayName;
	int32 BaseCost = 0;
	int32 RuntimeCost = 0;
	FGameplayTagContainer Keywords;
	TArray<FFirstCardEffectInstance> Effects;
};

struct FINALBATTLE_API FFirstEnemyPartIntentInstance
{
	FName IntentId = NAME_None;
	FText DisplayName;
	int32 InitialInitiative = 0;
	TArray<FFirstCardEffectInstance> Effects;
};

struct FINALBATTLE_API FFirstEnemyPartStartData
{
	FName PartId = NAME_None;
	FText DisplayName;
	int32 PositionIndex = 0;
	int32 MaxHP = 0;
	int32 CurrentHP = 0;
	FName CurrentIntentId = NAME_None;
	FText CurrentIntentDisplayName;
	int32 CurrentInitiative = 0;
	TArray<FFirstEnemyPartIntentInstance> IntentSequence;
	int32 CurrentIntentIndex = 0;
};

struct FINALBATTLE_API FFirstBattleStartParams
{
	FGuid BattleId;
	int32 StartingRound = 1;
	int32 PlayerMaxHP = 30;
	int32 PlayerCurrentHP = 30;
	TArray<FFirstCardInstance> InitialHand;
	TArray<FFirstEnemyPartStartData> EnemyParts;
};

struct FINALBATTLE_API FFirstBattleEvent
{
	EFirstBattleEventType EventType = EFirstBattleEventType::None;
	FName RelatedId = NAME_None;
	FGuid CardInstanceId;
	FName PartId = NAME_None;
	int32 PrimaryValue = 0;
	int32 SecondaryValue = 0;
	FText Message;
};
