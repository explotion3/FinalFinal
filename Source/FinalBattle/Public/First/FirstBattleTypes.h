#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

enum class EFirstCardEffectType : uint8
{
	None,
	Damage,
	MoveHandCard
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
	PlayerTurnStarted,
	CardPlayed,
	CardReturnedToHand,
	CardDrawn,
	DrawPileShuffled,
	InitiativeChanged,
	PerfectReleaseTriggered,
	EnemyPartActed,
	PlayerDamaged,
	EnemyPartDestroyed,
	BattleWon,
	BattleLost,
	HandCardMoved,
	CardRuntimeCostChanged,
	PlayerMaxHPChanged,
	CommandRejected
};

enum class EFirstHandRole : uint8
{
	None,
	LeftHandCore,
	RightHandCore
};

enum class EFirstHandZone : uint8
{
	None,
	Left,
	Both,
	Right
};

enum class EFirstHandMoveTargetPolicy : uint8
{
	RandomValidZone,
	RandomOtherThanSourceZone,
	FixedZone
};

enum class EFirstCardDrawSource : uint8
{
	None,
	ForcedCoreFromDrawPile,
	ForcedCoreFromDiscard,
	DrawPile,
	ShuffledDiscard
};

enum class EFirstCardPlayDestination : uint8
{
	DiscardPile,
	ReturnToHandRandomZone
};

struct FINALBATTLE_API FFirstCardEffectInstance
{
	EFirstCardEffectType EffectType = EFirstCardEffectType::None;
	FName EffectId = NAME_None;
	int32 Value = 0;
	int32 MoveCardCount = 1;
	bool bMoveRequiresSourceZone = false;
	EFirstHandZone MoveSourceZone = EFirstHandZone::None;
	EFirstHandMoveTargetPolicy MoveTargetPolicy = EFirstHandMoveTargetPolicy::RandomValidZone;
	EFirstHandZone MoveTargetZone = EFirstHandZone::None;
	int32 MoveTargetCostDelta = 0;
	bool bTransferActualCostReductionToSourceCard = false;
};

struct FINALBATTLE_API FFirstCardInstance
{
	FGuid CardInstanceId;
	FName CardId = NAME_None;
	FText DisplayName;
	int32 BaseCost = 0;
	int32 RuntimeCost = 0;
	int32 PlayerMaxHPBonusOnEnterBattle = 0;
	EFirstCardPlayDestination PlayDestination = EFirstCardPlayDestination::DiscardPile;
	EFirstHandRole HandRole = EFirstHandRole::None;
	bool bRequiresHandZoneToPlay = false;
	EFirstHandZone RequiredHandZone = EFirstHandZone::None;
	bool bSkipInitiativeReductionOnPerfectReleaseInZone = false;
	EFirstHandZone PerfectReleaseInitiativeSkipZone = EFirstHandZone::None;
	FGameplayTagContainer Keywords;
	TArray<FFirstCardEffectInstance> Effects;
};

struct FINALBATTLE_API FFirstCardDefinitionStartEntry
{
	FName CardId = NAME_None;
	int32 Count = 1;
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
	int32 RandomSeed = 1337;
	int32 PlayerMaxHP = 30;
	int32 PlayerCurrentHP = 30;
	TArray<FFirstCardInstance> InitialHand;
	TArray<FFirstCardInstance> InitialDrawPile;
	TArray<FFirstCardDefinitionStartEntry> InitialHandCardDefinitions;
	TArray<FFirstCardDefinitionStartEntry> InitialDrawPileCardDefinitions;
	TArray<FFirstEnemyPartStartData> EnemyParts;
};

struct FINALBATTLE_API FFirstBattleInitializeResult
{
	bool bSuccess = false;
	TArray<FName> MissingCardIds;
	TArray<FName> InvalidCardIds;
	FText Message;
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
