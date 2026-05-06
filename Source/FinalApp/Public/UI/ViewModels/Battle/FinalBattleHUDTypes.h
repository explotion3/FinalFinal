#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleHUDTypes.generated.h"

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCharacterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName ArtId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BreakthroughValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BreakthroughRequiredValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float BreakthroughFillNormalized = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bBreakthroughReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 StressCap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float StressPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCanActHint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentAwakenThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CollapseCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 VitalShare = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText StateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> StatusTexts;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName ArtId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 PositionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentInitiative = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentPhaseNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TotalPhases = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float PhaseProgressWithinPhase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText PhaseProgressText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText IntentText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bActedThisRound = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> StatusTexts;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDTeamCharacterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName ArtId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 StressCap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float StressPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BreakthroughValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BreakthroughRequiredValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float BreakthroughPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCanActHint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bBreakthroughReady = false;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDTeamStatusEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FGuid StatusInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalStatusId StatusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText OwnerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText SummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RemainingDuration = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDTeamPanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasActiveBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TeamMaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float TeamHealthPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TeamShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float TeamShieldFramePercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDTeamCharacterEntry> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDTeamStatusEntry> StatusPreviewEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 HiddenStatusCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bStatusDetailOpen = false;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDTeamStatusDetailData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDTeamStatusEntry> Statuses;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDEnemyDetailStatusEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalStatusId StatusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText SummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RemainingDuration = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDEnemyDetailData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasEnemy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalEnemyId EnemyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName EnemyRankTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float HealthPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float ShieldFramePercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float BreakPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentInitiative = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText InitiativeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText IntentText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText IntentNameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalIntentType IntentType = EFinalIntentType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName IntentIconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText PhaseProgressText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bIsCurrentBattleTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bIsInspected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bIsAlive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDEnemyDetailStatusEntry> Statuses;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCharacterDetailStatusEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalStatusId StatusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText SummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RemainingDuration = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCharacterDetailPassiveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalPassiveId PassiveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText SummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RemainingDuration = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCharacterDetailData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasCharacter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText RoleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName ArtId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 StressCap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float StressPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 VitalShare = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BreakthroughValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BreakthroughRequiredValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float BreakthroughFillNormalized = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bBreakthroughReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RootBone = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Insight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 KillingIntent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentAwakenThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CollapseCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bIsInspected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RuntimeAttack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RuntimeDefense = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float RuntimeBreakRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float RuntimeCritChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	float RuntimeCritDamage = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText UltimateNameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText UltimateRulesText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 UltimateCostEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bUltimateDefinitionReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bUltimateCanActivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bUltimateBlockedByCollapse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bUltimateUsedThisBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDCharacterDetailStatusEntry> Statuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDCharacterDetailPassiveEntry> Passives;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName ArtId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText OwnerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BaseCostAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RuntimeCostAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalCardType CardType = EFinalCardType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalBattleCardTargetRequirement TargetRequirement = EFinalBattleCardTargetRequirement::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText TypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText KeywordText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText RulesText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText ResolvedRulesText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCanPlayHint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText UnplayableHintText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bRetained = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCollapsedCard = false;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCardZoneEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FFinalCardId CardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText OwnerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalCardType CardType = EFinalCardType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText TypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 BaseCostAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RuntimeCostAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText KeywordText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText RulesText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bRetained = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bConsumeOnPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bOngoingCard = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bGeneratedCard = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bTemporaryCard = false;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCardZoneDetailData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasActiveBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalBattleCardZone SelectedZone = EFinalBattleCardZone::DrawPile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDCardZoneEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DrawPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 HandCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DiscardPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 OngoingZoneCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 ConsumePileCount = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDUltimateEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CostEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText StatusText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bBlockedByCollapse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bDefinitionReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bUsedThisBattle = false;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDLogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 EventSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalBattleEventType EventType = EFinalBattleEventType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText SummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DetailText;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleTopBarPanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasActiveBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText EncounterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentRound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TeamMaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 TeamShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DrawPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 HandCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DiscardPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 ConsumePileCount = 0;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleResourcePanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasActiveBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bEPFull = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 ActiveQiPipCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 MaxQiPipCount = 7;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleFeedbackPanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText FeedbackTitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText FeedbackText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalBattleCommandRejectReason FeedbackRejectReason = EFinalBattleCommandRejectReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName FeedbackReasonTag = NAME_None;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleContextPanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasActiveBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText CurrentTargetText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RelicCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RunDeckCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DrawPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 HandCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DiscardPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 OngoingZoneCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 ConsumePileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> TeamStatusTexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> ActiveRelicTexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> MissingFieldNotices;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleActionPanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHasActiveBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 DiscardPileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 ConsumePileCount = 0;
};
