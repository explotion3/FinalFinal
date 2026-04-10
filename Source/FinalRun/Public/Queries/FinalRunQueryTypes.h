#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Requests/FinalBattleResult.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "FinalRunQueryTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalRunFlowStage : uint8
{
	None,
	PreparingBattle,
	PendingBattleReward,
	AwaitingNodeAdvance,
	RunEnded
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunCharacterViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CollapseCount = 0;
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
	FFinalEncounterId SourceEncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome SourceBattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 RewardGold = 0;
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
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;
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
