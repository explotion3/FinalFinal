#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "Runtime/FinalRunState.h"

class UFinalDataRegistry;

struct FFinalRewardResolver
{
	static int32 GetRewardGoldTotal(const TArray<FFinalRunRewardEntry>& RewardEntries);

	static TArray<FFinalRunRewardEntry> MakeClaimedRewardEntries(
		const TArray<FFinalRunRewardEntry>& RewardEntries,
		const UFinalDataRegistry* DataRegistry);

	static TArray<FFinalRunRewardEntry> MakePreviewRewardEntries(
		const TArray<FFinalRunRewardEntry>& RewardEntries,
		const UFinalDataRegistry* DataRegistry);

	static TArray<FFinalRunRewardEntryViewData> BuildRewardEntryViews(
		const TArray<FFinalRunRewardEntry>& RewardEntries,
		const UFinalDataRegistry* DataRegistry);

	static void PopulateRewardEventViewData(
		FFinalRunEvent& Event,
		const UFinalDataRegistry* DataRegistry);

	static void PopulateAffectedCharacterResults(
		FFinalRunEvent& Event,
		const TArray<FFinalRunRewardEntry>& AppliedRewardEntries,
		const FFinalRunState& RunState,
		const UFinalDataRegistry* DataRegistry);

	static bool ValidateRewardEntriesForApplication(
		const TArray<FFinalRunRewardEntry>& RewardEntries,
		const UFinalDataRegistry* DataRegistry,
		const FFinalRunState& RunState,
		EFinalRunCommandRejectReason& OutRejectReason,
		FText& OutFailureMessage);

	static void ApplyValidatedRewardEntriesToRunState(
		const TArray<FFinalRunRewardEntry>& RewardEntries,
		FFinalRunState& RunState);
};
