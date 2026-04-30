#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleQueryTypes.generated.h"

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleCharacterViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 StressCap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentAwakenThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CollapseCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 VitalShare = 0;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattlePhaseProgressViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentPhaseNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 TotalPhases = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	float ProgressWithinPhase = 0.0f;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleEnemyViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalEnemyId EnemyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 PositionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 MaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 MaxBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentInitiative = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName CurrentPhaseTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName CurrentIntentId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalBattlePhaseProgressViewData PhaseProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText IntentText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bActedThisRound = false;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleCardViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName SourceRunCardInstanceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCardId CardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName RuntimeOwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	EFinalCardType CardType = EFinalCardType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 RuntimeCostAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 RuntimeDamagePowerPercentPointDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 RuntimeFinalDamagePercentDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGameplayTagContainer RuntimeKeywords;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bRetained = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bCollapsedCard = false;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleStatusViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid StatusInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalStatusId StatusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName SourceUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 RemainingDuration = 0;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattlePassiveViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid PassiveInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalPassiveId PassiveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName SourceUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 RemainingDuration = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	EFinalPassiveDurationType DurationType = EFinalPassiveDurationType::Battle;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleCharacterStatusesViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleStatusViewData> StatusEntries;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleUltimateViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalUltimateId UltimateId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CostEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bDefinitionReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bBlockedByCollapse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bCanActivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bUsedThisBattle = false;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleDeckViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 DrawPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 HandCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 DiscardPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 OngoingZoneCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 ConsumePileCount = 0;
};
