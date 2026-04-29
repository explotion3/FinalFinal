#include "Growth/FinalGrowthResolver.h"

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalCardEvolutionDefinition.h"

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
	View.Level = CharacterState.Level;
	View.BreakthroughValue = CharacterState.BreakthroughValue;
	View.BreakthroughRequiredValue = CharacterState.BreakthroughRequiredValue;
	View.RootBone = CharacterState.RootBone;
	View.Insight = CharacterState.Insight;
	View.KillingIntent = CharacterState.KillingIntent;
	View.bHasPendingGrowthChoice = CharacterState.bHasPendingGrowthChoice;
	View.CurrentStress = CharacterState.CurrentStress;
	View.bCollapsed = CharacterState.bCollapsed;
	View.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
	View.CollapseCount = CharacterState.CollapseCount;
	View.StateSummaryText = BuildCharacterStateSummaryText(CharacterState);
	return View;
}

int32 FindRunDeckIndexByInstanceId(const TArray<FFinalRunCardInstance>& RunDeck, const FName InstanceId)
{
	return RunDeck.IndexOfByPredicate([&InstanceId](const FFinalRunCardInstance& CardInstance)
	{
		return CardInstance.InstanceId == InstanceId;
	});
}

bool ApplyAttributeGrowthChoice(
	const FFinalRunGrowthChoiceInstance& GrowthChoice,
	FFinalRunPersistentCharacterState& CharacterState,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage)
{
	if (GrowthChoice.AttributeDelta <= 0)
	{
		OutRejectReason = EFinalRunCommandRejectReason::InvalidGrowthValue;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "InvalidAttributeGrowthDelta", "Growth choice {0} is invalid because AttributeDelta must be greater than zero."),
			FText::FromName(GrowthChoice.ChoiceInstanceId));
		return false;
	}

	switch (GrowthChoice.AttributeType)
	{
	case EFinalGrowthAttributeType::RootBone:
		CharacterState.RootBone += GrowthChoice.AttributeDelta;
		return true;

	case EFinalGrowthAttributeType::Insight:
		CharacterState.Insight += GrowthChoice.AttributeDelta;
		return true;

	case EFinalGrowthAttributeType::KillingIntent:
		CharacterState.KillingIntent += GrowthChoice.AttributeDelta;
		return true;

	default:
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedGrowthAttributeType;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "UnsupportedGrowthAttributeType", "Growth choice {0} uses unsupported AttributeType {1}."),
			FText::FromName(GrowthChoice.ChoiceInstanceId),
			FText::AsNumber(static_cast<int32>(GrowthChoice.AttributeType)));
		return false;
	}
}

bool ApplyCardEvolutionGrowthChoice(
	const FFinalRunGrowthChoiceInstance& GrowthChoice,
	FFinalRunState& RunState,
	const UFinalDataRegistry* DataRegistry,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage)
{
	if (DataRegistry == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingGrowthCardEvolutionDefinition;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "MissingDataRegistryForGrowthEvolution", "Growth choice {0} cannot be applied because the data registry is unavailable."),
			FText::FromName(GrowthChoice.ChoiceInstanceId));
		return false;
	}

	if (!GrowthChoice.TargetRunCardInstanceId.IsNone())
	{
		const int32 RunDeckIndex = FindRunDeckIndexByInstanceId(RunState.RunDeck, GrowthChoice.TargetRunCardInstanceId);
		if (RunDeckIndex == INDEX_NONE)
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingGrowthTargetRunCardInstance;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalGrowthResolver", "MissingGrowthTargetRunCardInstance", "Growth choice {0} targets run card instance {1}, but that card is no longer present in the current RunDeck."),
				FText::FromName(GrowthChoice.ChoiceInstanceId),
				FText::FromName(GrowthChoice.TargetRunCardInstanceId));
			return false;
		}

		UFinalCardEvolutionDefinition* EvolutionDefinition = DataRegistry->FindCardEvolutionDefinition(GrowthChoice.CardEvolutionId);
		if (EvolutionDefinition == nullptr)
		{
			OutRejectReason = EFinalRunCommandRejectReason::MissingGrowthCardEvolutionDefinition;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalGrowthResolver", "MissingGrowthCardEvolutionDefinition", "Growth choice {0} references evolution definition {1}, but it could not be resolved."),
				FText::FromName(GrowthChoice.ChoiceInstanceId),
				FText::FromName(GrowthChoice.CardEvolutionId.Value));
			return false;
		}

		FFinalRunCardInstance& CardInstance = RunState.RunDeck[RunDeckIndex];
		if (CardInstance.OwnerCharacterId != GrowthChoice.CharacterId
			|| CardInstance.CurrentCardId != EvolutionDefinition->FromCardId
			|| CardInstance.EvolutionStage != EvolutionDefinition->FromStage)
		{
			OutRejectReason = EFinalRunCommandRejectReason::GrowthEvolutionTargetMismatch;
			OutFailureMessage = FText::Format(
				NSLOCTEXT("FinalGrowthResolver", "GrowthEvolutionTargetMismatch", "Growth choice {0} can no longer evolve run card instance {1} because its current card or evolution stage no longer matches the expected source."),
				FText::FromName(GrowthChoice.ChoiceInstanceId),
				FText::FromName(GrowthChoice.TargetRunCardInstanceId));
			return false;
		}

		CardInstance.CurrentCardId = EvolutionDefinition->ToCardId;
		CardInstance.EvolutionStage = EvolutionDefinition->ToStage;
		return true;
	}

	OutRejectReason = EFinalRunCommandRejectReason::MissingGrowthTargetRunCardInstance;
	OutFailureMessage = FText::Format(
		NSLOCTEXT("FinalGrowthResolver", "MissingTargetRunCardInstanceOnChoice", "Growth choice {0} is missing TargetRunCardInstanceId."),
		FText::FromName(GrowthChoice.ChoiceInstanceId));
	return false;
}
}

bool FFinalGrowthResolver::ValidateAndApplyGrowthChoice(
	const FFinalRunGrowthChoiceInstance& GrowthChoice,
	FFinalRunState& RunState,
	const UFinalDataRegistry* DataRegistry,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage)
{
	using namespace FinalGrowthResolverLocal;

	const int32 CharacterIndex = FindRunCharacterIndex(RunState.Characters, GrowthChoice.CharacterId);
	if (CharacterIndex == INDEX_NONE)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownGrowthTargetCharacter;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "UnknownGrowthChoiceTargetCharacter", "Growth choice {0} targets character {1}, but that character is not present in the current Run state."),
			FText::FromName(GrowthChoice.ChoiceInstanceId),
			FText::FromName(GrowthChoice.CharacterId.Value));
		return false;
	}

	FFinalRunPersistentCharacterState& CharacterState = RunState.Characters[CharacterIndex];
	switch (GrowthChoice.ChoiceType)
	{
	case EFinalGrowthChoiceType::AttributeGrowth:
		return ApplyAttributeGrowthChoice(GrowthChoice, CharacterState, OutRejectReason, OutFailureMessage);

	case EFinalGrowthChoiceType::CardEvolution:
		return ApplyCardEvolutionGrowthChoice(GrowthChoice, RunState, DataRegistry, OutRejectReason, OutFailureMessage);

	case EFinalGrowthChoiceType::Special:
	default:
		OutRejectReason = EFinalRunCommandRejectReason::UnsupportedGrowthChoiceType;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalGrowthResolver", "UnsupportedGrowthChoiceType", "Growth choice {0} uses unsupported ChoiceType {1}."),
			FText::FromName(GrowthChoice.ChoiceInstanceId),
			FText::AsNumber(static_cast<int32>(GrowthChoice.ChoiceType)));
		return false;
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

TArray<FFinalRunCharacterViewData> FFinalGrowthResolver::BuildAffectedCharacterResults(
	const TArray<FFinalCharacterId>& AffectedCharacterIds,
	const FFinalRunState& RunState,
	const UFinalDataRegistry* DataRegistry)
{
	using namespace FinalGrowthResolverLocal;

	TArray<FFinalRunCharacterViewData> Results;
	if (AffectedCharacterIds.IsEmpty())
	{
		return Results;
	}

	TSet<FName> UniqueCharacterIds;
	for (const FFinalCharacterId& CharacterId : AffectedCharacterIds)
	{
		if (CharacterId.IsValid())
		{
			UniqueCharacterIds.Add(CharacterId.Value);
		}
	}

	for (const FFinalRunPersistentCharacterState& CharacterState : RunState.Characters)
	{
		if (!CharacterState.CharacterId.IsValid() || !UniqueCharacterIds.Contains(CharacterState.CharacterId.Value))
		{
			continue;
		}

		Results.Add(MakeCharacterView(CharacterState, DataRegistry));
	}

	return Results;
}
