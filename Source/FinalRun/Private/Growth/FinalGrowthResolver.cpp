#include "Growth/FinalGrowthResolver.h"

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Queries/FinalDataRegistry.h"

namespace FinalGrowthResolverLocal
{
int32 FindRunCharacterIndex(const TArray<FFinalRunPersistentCharacterState>& Characters, const FFinalCharacterId& CharacterId)
{
	return Characters.IndexOfByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& CharacterState)
	{
		return CharacterState.CharacterId == CharacterId;
	});
}

FText ResolveCharacterDisplayName(const FFinalCharacterId& CharacterId, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry != nullptr && CharacterId.IsValid())
	{
		if (const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(CharacterId))
		{
			if (!CharacterDefinition->DisplayName.IsEmpty())
			{
				return CharacterDefinition->DisplayName;
			}
		}
	}

	return CharacterId.IsValid()
		? FText::FromName(CharacterId.Value)
		: NSLOCTEXT("FinalGrowthResolver", "UnknownGrowthCharacter", "Unknown Character");
}

FName ResolveCharacterIconId(const FFinalCharacterId& CharacterId)
{
	return CharacterId.IsValid() ? CharacterId.Value : NAME_None;
}

FText BuildCharacterStateSummaryText(const FFinalRunPersistentCharacterState& CharacterState)
{
	return FText::Format(
		NSLOCTEXT("FinalGrowthResolver", "RunCharacterStateSummary", "Stress {0} | Awaken {1} | Collapse {2}"),
		FText::AsNumber(CharacterState.CurrentStress),
		FText::AsNumber(CharacterState.CurrentAwakenCount),
		FText::AsNumber(CharacterState.CollapseCount));
}

FFinalRunCharacterViewData MakeCharacterView(const FFinalRunPersistentCharacterState& CharacterState, const UFinalDataRegistry* DataRegistry)
{
	FFinalRunCharacterViewData View;
	View.CharacterId = CharacterState.CharacterId;
	View.DisplayName = ResolveCharacterDisplayName(CharacterState.CharacterId, DataRegistry);
	View.IconId = ResolveCharacterIconId(CharacterState.CharacterId);
	View.CurrentStress = CharacterState.CurrentStress;
	View.bCollapsed = CharacterState.bCollapsed;
	View.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
	View.CollapseCount = CharacterState.CollapseCount;
	View.StateSummaryText = BuildCharacterStateSummaryText(CharacterState);
	return View;
}
}

bool FFinalGrowthResolver::ValidateGrowthReward(
	const FFinalRunRewardEntry& RewardEntry,
	const FFinalRunState& RunState,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage)
{
	using namespace FinalGrowthResolverLocal;

	if (!RewardEntry.GrowthTargetCharacterId.IsValid())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingGrowthTargetCharacterId;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "MissingGrowthTargetCharacterId", "Growth reward {0} is missing GrowthTargetCharacterId."),
			FText::FromName(RewardEntry.RewardId));
		return false;
	}

	if (FindRunCharacterIndex(RunState.Characters, RewardEntry.GrowthTargetCharacterId) == INDEX_NONE)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownGrowthTargetCharacter;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "UnknownGrowthTargetCharacter", "Growth reward {0} targets character {1}, but that character is not present in the current Run state."),
			FText::FromName(RewardEntry.RewardId),
			FText::FromName(RewardEntry.GrowthTargetCharacterId.Value));
		return false;
	}

	if (RewardEntry.Value <= 0)
	{
		OutRejectReason = EFinalRunCommandRejectReason::InvalidGrowthValue;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "InvalidGrowthValue", "Growth reward {0} is invalid because Value must be greater than zero."),
			FText::FromName(RewardEntry.RewardId));
		return false;
	}

	switch (RewardEntry.GrowthEffectType)
	{
	case EFinalRunGrowthEffectType::ReduceStress:
	case EFinalRunGrowthEffectType::GainAwakenProgress:
	case EFinalRunGrowthEffectType::ReduceCollapseCount:
		return true;

	case EFinalRunGrowthEffectType::None:
	default:
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedGrowthEffectType;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "UnsupportedGrowthEffectType", "Growth reward {0} uses unsupported GrowthEffectType {1}."),
			FText::FromName(RewardEntry.RewardId),
			FText::AsNumber(static_cast<int32>(RewardEntry.GrowthEffectType)));
		return false;
	}
}

void FFinalGrowthResolver::ApplyGrowthReward(
	const FFinalRunRewardEntry& RewardEntry,
	FFinalRunState& RunState)
{
	using namespace FinalGrowthResolverLocal;

	const int32 CharacterIndex = FindRunCharacterIndex(RunState.Characters, RewardEntry.GrowthTargetCharacterId);
	if (CharacterIndex == INDEX_NONE)
	{
		return;
	}

	FFinalRunPersistentCharacterState& CharacterState = RunState.Characters[CharacterIndex];
	switch (RewardEntry.GrowthEffectType)
	{
	case EFinalRunGrowthEffectType::ReduceStress:
		CharacterState.CurrentStress = FMath::Max(0, CharacterState.CurrentStress - RewardEntry.Value);
		break;

	case EFinalRunGrowthEffectType::GainAwakenProgress:
		CharacterState.CurrentAwakenCount += RewardEntry.Value;
		break;

	case EFinalRunGrowthEffectType::ReduceCollapseCount:
		CharacterState.CollapseCount = FMath::Max(0, CharacterState.CollapseCount - RewardEntry.Value);
		break;

	default:
		break;
	}
}

bool FFinalGrowthResolver::ContainsGrowthRewards(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	for (const FFinalRunRewardEntry& Entry : RewardEntries)
	{
		if (Entry.RewardType == EFinalRunRewardType::Growth && Entry.GrowthTargetCharacterId.IsValid())
		{
			return true;
		}
	}

	return false;
}

TArray<FFinalRunCharacterViewData> FFinalGrowthResolver::BuildAffectedCharacterResults(
	const TArray<FFinalRunRewardEntry>& AppliedRewardEntries,
	const FFinalRunState& RunState,
	const UFinalDataRegistry* DataRegistry)
{
	using namespace FinalGrowthResolverLocal;

	TArray<FFinalRunCharacterViewData> Results;
	if (!ContainsGrowthRewards(AppliedRewardEntries))
	{
		return Results;
	}

	TSet<FName> AffectedCharacterIds;
	for (const FFinalRunRewardEntry& Entry : AppliedRewardEntries)
	{
		if (Entry.RewardType == EFinalRunRewardType::Growth && Entry.GrowthTargetCharacterId.IsValid())
		{
			AffectedCharacterIds.Add(Entry.GrowthTargetCharacterId.Value);
		}
	}

	for (const FFinalRunPersistentCharacterState& CharacterState : RunState.Characters)
	{
		if (!CharacterState.CharacterId.IsValid() || !AffectedCharacterIds.Contains(CharacterState.CharacterId.Value))
		{
			continue;
		}

		Results.Add(MakeCharacterView(CharacterState, DataRegistry));
	}

	return Results;
}
