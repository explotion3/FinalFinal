#include "Save/FinalRunSaveData.h"

namespace
{
void SetRunSaveValidationFailure(FText* OutFailureReason, const FText& FailureReason)
{
	if (OutFailureReason != nullptr)
	{
		*OutFailureReason = FailureReason;
	}
}

bool DoesConfiguredNodeExist(const TArray<FFinalRunNodeDefinition>& ConfiguredRunNodes, const FName NodeId)
{
	if (NodeId.IsNone())
	{
		return true;
	}

	return ConfiguredRunNodes.ContainsByPredicate(
		[NodeId](const FFinalRunNodeDefinition& NodeDefinition)
		{
			return NodeDefinition.NodeId == NodeId;
		});
}

bool ValidateNodeIdList(const TArray<FFinalRunNodeDefinition>& ConfiguredRunNodes, const TArray<FName>& NodeIds, const FText& FailureFormat, FText* OutFailureReason)
{
	for (const FName NodeId : NodeIds)
	{
		if (NodeId.IsNone())
		{
			continue;
		}

		if (!DoesConfiguredNodeExist(ConfiguredRunNodes, NodeId))
		{
			SetRunSaveValidationFailure(
				OutFailureReason,
				FText::Format(FailureFormat, FText::FromName(NodeId)));
			return false;
		}
	}

	return true;
}
}

bool FFinalRunSaveData::IsSupportedVersion() const
{
	return SaveVersion == CurrentSaveVersion;
}

bool FFinalRunSaveData::IsStructurallyValid(FText* OutFailureReason) const
{
	if (!IsSupportedVersion())
	{
		SetRunSaveValidationFailure(
			OutFailureReason,
			FText::Format(
				NSLOCTEXT("FinalRunSaveData", "UnsupportedSaveVersion", "Unsupported run save version {0}; expected {1}."),
				FText::AsNumber(SaveVersion),
				FText::AsNumber(CurrentSaveVersion)));
		return false;
	}

	if (RunState.Characters.IsEmpty() && RunState.RunDeck.IsEmpty())
	{
		SetRunSaveValidationFailure(
			OutFailureReason,
			NSLOCTEXT("FinalRunSaveData", "EmptyRunState", "Run save data does not contain any characters or deck cards."));
		return false;
	}

	if (!CurrentNodeId.IsNone() && !DoesConfiguredNodeExist(ConfiguredRunNodes, CurrentNodeId))
	{
		SetRunSaveValidationFailure(
			OutFailureReason,
			FText::Format(
				NSLOCTEXT("FinalRunSaveData", "CurrentNodeMissing", "Current run node {0} is not present in ConfiguredRunNodes."),
				FText::FromName(CurrentNodeId)));
		return false;
	}

	if (!ValidateNodeIdList(
		ConfiguredRunNodes,
		VisitedNodeIds,
		NSLOCTEXT("FinalRunSaveData", "VisitedNodeMissing", "Visited run node {0} is not present in ConfiguredRunNodes."),
		OutFailureReason))
	{
		return false;
	}

	if (!ValidateNodeIdList(
		ConfiguredRunNodes,
		ResolvedNodeIds,
		NSLOCTEXT("FinalRunSaveData", "ResolvedNodeMissing", "Resolved run node {0} is not present in ConfiguredRunNodes."),
		OutFailureReason))
	{
		return false;
	}

	const bool bHasPendingRewardSource = !PendingRewardSourceNodeId.IsNone();
	const bool bHasPendingRewardEntries = !PendingRewardEntries.IsEmpty();
	if (bHasPendingRewardSource)
	{
		if (!DoesConfiguredNodeExist(ConfiguredRunNodes, PendingRewardSourceNodeId))
		{
			SetRunSaveValidationFailure(
				OutFailureReason,
				FText::Format(
					NSLOCTEXT("FinalRunSaveData", "PendingRewardSourceNodeMissing", "Pending reward source node {0} is not present in ConfiguredRunNodes."),
					FText::FromName(PendingRewardSourceNodeId)));
			return false;
		}

		if (!bHasPendingRewardEntries)
		{
			SetRunSaveValidationFailure(
				OutFailureReason,
				NSLOCTEXT("FinalRunSaveData", "PendingRewardSourceWithoutEntries", "PendingRewardSourceNodeId is set but PendingRewardEntries is empty."));
			return false;
		}

		if (!PendingRewardSourceEncounterId.IsValid())
		{
			SetRunSaveValidationFailure(
				OutFailureReason,
				NSLOCTEXT("FinalRunSaveData", "PendingRewardMissingEncounter", "Pending reward context is missing a valid source encounter id."));
			return false;
		}

		if (PendingRewardBattleOutcome == EFinalBattleOutcome::None)
		{
			SetRunSaveValidationFailure(
				OutFailureReason,
				NSLOCTEXT("FinalRunSaveData", "PendingRewardMissingOutcome", "Pending reward context is missing a battle outcome."));
			return false;
		}
	}
	else if (bHasPendingRewardEntries)
	{
		SetRunSaveValidationFailure(
			OutFailureReason,
			NSLOCTEXT("FinalRunSaveData", "PendingRewardEntriesWithoutSource", "PendingRewardEntries exist but PendingRewardSourceNodeId is empty."));
		return false;
	}
	else if (PendingRewardSourceEncounterId.IsValid() || PendingRewardBattleOutcome != EFinalBattleOutcome::None)
	{
		SetRunSaveValidationFailure(
			OutFailureReason,
			NSLOCTEXT("FinalRunSaveData", "PendingRewardContextWithoutSource", "Pending reward encounter or battle outcome is set while PendingRewardSourceNodeId is empty."));
		return false;
	}

	const int32 MaxRunLogEventSequence = GetMaxRunLogEventSequence();
	if (LastEventSequence < MaxRunLogEventSequence)
	{
		SetRunSaveValidationFailure(
			OutFailureReason,
			FText::Format(
				NSLOCTEXT("FinalRunSaveData", "LastEventSequenceBehindLog", "LastEventSequence {0} is smaller than the max RunLogEntries sequence {1}."),
				FText::AsNumber(LastEventSequence),
				FText::AsNumber(MaxRunLogEventSequence)));
		return false;
	}

	if (OutFailureReason != nullptr)
	{
		*OutFailureReason = FText::GetEmpty();
	}
	return true;
}

int32 FFinalRunSaveData::GetMaxRunLogEventSequence() const
{
	int32 MaxEventSequence = 0;
	for (const FFinalRunEvent& Event : RunLogEntries)
	{
		MaxEventSequence = FMath::Max(MaxEventSequence, Event.EventSequence);
	}

	return MaxEventSequence;
}
