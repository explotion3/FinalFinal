#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UI/ViewModels/FinalViewModelBase.h"
#include "FinalBattleHUDViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFinalPhaseChangedPresentationSignature, const FFinalBattleEvent&, BattleEvent);

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDCharacterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 StressCap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCollapsed = false;

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
struct FINALAPP_API FFinalBattleHUDCardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText OwnerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 RuntimeCostAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText TypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText KeywordText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText RulesText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bRetained = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bCollapsedCard = false;
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
	EFinalBattleEventType EventType = EFinalBattleEventType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText Message;
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalBattleHUDPresentationData
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
	FText CurrentTargetText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText FeedbackTitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FText FeedbackText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalBattleCommandRejectReason FeedbackRejectReason = EFinalBattleCommandRejectReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	FName FeedbackReasonTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> TeamStatusTexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> ActiveRelicTexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDCharacterEntry> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDEnemyEntry> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDCardEntry> HandCards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDUltimateEntry> Ultimates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FFinalBattleHUDLogEntry> LogEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	TArray<FText> MissingFieldNotices;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleHUDViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplySnapshot(const FFinalBattleSnapshot& InSnapshot);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplyBattleEvent(const FFinalBattleEvent& BattleEvent);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleEvent GetLatestPhaseChangedEvent() const;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplyPresentation(const FFinalBattleHUDPresentationData& InPresentation);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleHUDPresentationData GetPresentation() const;

	UPROPERTY(BlueprintAssignable, Category = "Final|UI")
	FFinalPhaseChangedPresentationSignature OnPhaseChangedPresentation;

private:
	UPROPERTY(Transient)
	FFinalBattleSnapshot Snapshot;

	UPROPERTY(Transient)
	FFinalBattleEvent LatestPhaseChangedEvent;

	UPROPERTY(Transient)
	FFinalBattleHUDPresentationData Presentation;
};
