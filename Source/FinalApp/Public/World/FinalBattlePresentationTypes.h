#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattlePresentationTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalBattlePresentationTeam : uint8
{
	Player,
	Enemy
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleOverheadStatusViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FFinalStatusId StatusId;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 CurrentStacks = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 RemainingDuration = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleEnemyOverheadViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 CurrentHP = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 MaxHP = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	float HealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 CurrentShield = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	float ShieldFramePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 CurrentBreakValue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 MaxBreakValue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	float BreakPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	int32 CurrentInitiative = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FText InitiativeText;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FText IntentText;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FName IntentIconId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	FName EnemyRankTag = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	bool bIsTargeted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	bool bIsAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Overhead")
	TArray<FFinalBattleOverheadStatusViewData> Statuses;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattlePresentationUnitViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	FName UnitDefinitionId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	EFinalBattlePresentationTeam Team = EFinalBattlePresentationTeam::Player;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	bool bIsAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation")
	bool bIsTargeted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	int32 CurrentHP = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	int32 MaxHP = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	int32 CurrentShield = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	int32 CurrentBreakValue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	int32 MaxBreakValue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	int32 CurrentInitiative = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	FText IntentText;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Enemy")
	FFinalBattleEnemyOverheadViewData EnemyOverheadView;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	int32 CurrentStress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	int32 StressCap = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	int32 VitalShare = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	int32 CurrentAwakenThreshold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	int32 CollapseCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Final|Battle|Presentation|Player")
	bool bCollapsed = false;
};
