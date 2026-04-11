#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRunNodeContentDefinition.h"
#include "Requests/FinalBattleResult.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "FinalRunQueryTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalRunFlowStage : uint8
{
	None,
	PreparingBattle,
	PendingBattleReward,
	AwaitingNodeAdvance,
	PendingRewardNode,
	PendingEventNode,
	PendingShopNode,
	RunEnded
};

UENUM(BlueprintType)
enum class EFinalRunNodeAvailabilityReason : uint8
{
	None,
	DefinitionLocked,
	PendingBattleRewardMustBeClaimed,
	CurrentNodeRequiresResolution,
	MissingBattleConfig,
	RunEnded
};

UENUM(BlueprintType)
enum class EFinalRunRewardPresentationKind : uint8
{
	None,
	Gold,
	Card,
	Relic,
	DeckEdit,
	Growth
};

UENUM(BlueprintType)
enum class EFinalRunRewardVisualTier : uint8
{
	None,
	Currency,
	Common,
	Rare,
	Epic,
	Legendary,
	Utility,
	Progression
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunCharacterViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CollapseCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText StateSummaryText;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunDeckEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCardId CardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunRelicEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRelicId RelicId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunCurrentBuildViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunDeckEntryViewData> DeckEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRelicEntryViewData> RelicEntries;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunRewardEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunRewardType RewardType = EFinalRunRewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText PrimaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText SecondaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunRewardPresentationKind PresentationKind = EFinalRunRewardPresentationKind::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunRewardVisualTier VisualTier = EFinalRunRewardVisualTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DetailText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCharacterId TargetCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCardId CardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCardId SourceCardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCardId ResultCardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRelicId RelicId;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunPendingBattleRewardViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasPendingReward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName SourceNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunNodeType SourceNodeType = EFinalRunNodeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText SourceNodeDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName SourceNodeDisplayLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId SourceEncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome SourceBattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 RewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanClaim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntryViewData> RewardEntryViews;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunPendingRewardNodeViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasPendingContent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanResolve = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bResolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntryViewData> RewardEntryViews;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunEventOptionViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName OptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText OutcomeSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bSelectable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText AvailabilityMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntryViewData> RewardEntryViews;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunPendingEventNodeViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasPendingContent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanResolve = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bResolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunEventOptionViewData> Options;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunShopOfferViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName OfferId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bPurchasable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bPurchased = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText AvailabilityMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntryViewData> RewardEntryViews;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunPendingShopNodeViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasPendingContent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanResolve = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bResolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunShopOfferViewData> Offers;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunNodeOptionViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunNodeType NodeType = EFinalRunNodeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName DisplayLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 ChapterIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 FloorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bVisited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunNodeAvailabilityReason AvailabilityReason = EFinalRunNodeAvailabilityReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText AvailabilityMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasImplementedResolver = false;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunProgressionViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunFlowStage FlowStage = EFinalRunFlowStage::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName CurrentNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunNodeType CurrentNodeType = EFinalRunNodeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText CurrentNodeDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName CurrentNodeDisplayLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentChapter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentFloor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCurrentNodeVisited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCurrentNodeNeedsResolution = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCurrentNodeHasImplementedResolver = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText CurrentNodeStateMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanClaimPendingBattleReward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanAdvanceToNextNode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunNodeOptionViewData> AvailableNextNodes;
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunBattleBridgeViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasPendingBattleStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 PartyCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 DeckCount = 0;
};
