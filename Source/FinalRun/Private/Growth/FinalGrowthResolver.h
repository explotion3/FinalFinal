#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "Runtime/FinalRunState.h"

class UFinalDataRegistry;
class UFinalCardEvolutionDefinition;

struct FFinalGrowthResolver
{
	static bool ValidateAndApplyGrowthChoice(
		const FFinalRunGrowthChoiceInstance& GrowthChoice,
		FFinalRunState& RunState,
		const UFinalDataRegistry* DataRegistry,
		EFinalRunCommandRejectReason& OutRejectReason,
		FText& OutFailureMessage);

	static bool ValidateGrowthReward(
		const FFinalRunRewardEntry& RewardEntry,
		const FFinalRunState& RunState,
		EFinalRunCommandRejectReason& OutRejectReason,
		FText& OutFailureMessage);

	static void ApplyGrowthReward(
		const FFinalRunRewardEntry& RewardEntry,
		FFinalRunState& RunState);

	static bool ContainsGrowthRewards(const TArray<FFinalRunRewardEntry>& RewardEntries);

	static TArray<FFinalRunCharacterViewData> BuildAffectedCharacterResults(
		const TArray<FFinalRunRewardEntry>& AppliedRewardEntries,
		const FFinalRunState& RunState,
		const UFinalDataRegistry* DataRegistry);

	static TArray<FFinalRunCharacterViewData> BuildAffectedCharacterResults(
		const TArray<FFinalCharacterId>& AffectedCharacterIds,
		const FFinalRunState& RunState,
		const UFinalDataRegistry* DataRegistry);
};
