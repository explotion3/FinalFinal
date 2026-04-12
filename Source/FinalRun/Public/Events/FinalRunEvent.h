#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalRunCommand.h"
#include "Ids/FinalIds.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Requests/FinalBattleResult.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "FinalRunEvent.generated.h"

UENUM(BlueprintType)
enum class EFinalRunEventType : uint8
{
	Info,
	RunInitialized,
	BattleStartConfigured,
	RunCommandAccepted,
	RunCommandRejected,
	BattleResultApplied,
	PendingBattleRewardGenerated,
	PendingBattleRewardClaimed,
	RewardNodeResolved,
	EventNodeResolved,
	ShopOfferPurchased,
	NodeAdvanced
};

UENUM(BlueprintType)
enum class EFinalRunCommandRejectReason : uint8
{
	None,
	UnsupportedCommand,
	MissingPendingBattleReward,
	PendingBattleRewardMustBeClaimed,
	MissingTargetNode,
	UnknownTargetNode,
	TargetNodeNotReachable,
	TargetNodeLocked,
	UnsupportedTargetNodeType,
	TargetNodeMissingBattleConfig,
	CurrentNodeRequiresResolution,
	MissingRewardNodeContent,
	MissingEventNodeContent,
	MissingShopNodeContent,
	MissingPayloadId,
	UnknownEventOption,
	EventOptionDisabled,
	UnknownShopOffer,
	ShopOfferUnavailable,
	InsufficientGold,
	MissingGrantedCardId,
	MissingGrantedRelicId,
	MissingRemovedCardId,
	MissingUpgradeFromCardId,
	MissingUpgradeToCardId,
	MissingGrowthTargetCharacterId,
	RewardCardDefinitionUnavailable,
	RewardRelicDefinitionUnavailable,
	RewardTargetCardNotInRunDeck,
	RewardUpgradeResultInvalid,
	UnknownGrowthTargetCharacter,
	UnsupportedGrowthEffectType,
	InvalidGrowthValue,
	UnsupportedRewardType,
	EventNodeResolutionNotImplemented,
	ShopNodeResolutionNotImplemented,
	RunEnded
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 EventSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunEventType EventType = EFinalRunEventType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunCommandType CommandType = EFinalRunCommandType::AdvanceToNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunCommandRejectReason RejectReason = EFinalRunCommandRejectReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName PayloadId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName SourceNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunNodeType NodeType = EFinalRunNodeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText NodeDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName NodeDisplayLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 ChapterIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 FloorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome BattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 RewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 SpentGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntryViewData> RewardEntryViews;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunCharacterViewData> AffectedCharacterResults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Message;
};
