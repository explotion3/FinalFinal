#include "Events/FinalRunEventResolver.h"

#include "Queries/FinalDataRegistry.h"
#include "Rewards/FinalRewardResolver.h"

const FFinalRunEventOptionDefinition* FFinalRunEventResolver::FindEventOptionDefinition(
	const FFinalRunNodeDefinition& NodeDefinition,
	const FName OptionId)
{
	return NodeDefinition.EventContent.Options.FindByPredicate([&OptionId](const FFinalRunEventOptionDefinition& Option)
	{
		return Option.OptionId == OptionId;
	});
}

bool FFinalRunEventResolver::TryResolveEventOption(
	const FFinalRunNodeDefinition& NodeDefinition,
	const FName OptionId,
	const UFinalDataRegistry* DataRegistry,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage,
	FFinalResolvedEventOptionResult& OutResult)
{
	if (OptionId.IsNone())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPayloadId;
		OutFailureMessage = FText::FromString(TEXT("ResolveEvent requires an event option id in PayloadId."));
		return false;
	}

	const FFinalRunEventOptionDefinition* SelectedOption = FindEventOptionDefinition(NodeDefinition, OptionId);
	if (SelectedOption == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownEventOption;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalRunEventResolver", "UnknownEventOption", "Event option {0} is not defined on the current node."),
			FText::FromName(OptionId));
		return false;
	}

	if (SelectedOption->bStartsDisabled)
	{
		OutRejectReason = EFinalRunCommandRejectReason::EventOptionDisabled;
		OutFailureMessage = SelectedOption->DisabledReason.IsEmpty()
			? FText::FromString(TEXT("The selected event option is currently disabled."))
			: SelectedOption->DisabledReason;
		return false;
	}

	OutResult.OptionDefinition = SelectedOption;
	OutResult.PreviewEntries = FFinalRewardResolver::MakePreviewRewardEntries(SelectedOption->RewardEntries, DataRegistry);
	OutResult.ResolvedEntries = FFinalRewardResolver::MakeClaimedRewardEntries(SelectedOption->RewardEntries, DataRegistry);
	return true;
}

void FFinalRunEventResolver::BuildEventOptionViews(
	const FFinalRunNodeDefinition& NodeDefinition,
	const UFinalDataRegistry* DataRegistry,
	const bool bNodeResolved,
	TArray<FFinalRunEventOptionViewData>& OutOptions,
	bool& bOutCanResolve)
{
	OutOptions.Reset();
	bOutCanResolve = false;

	for (const FFinalRunEventOptionDefinition& Option : NodeDefinition.EventContent.Options)
	{
		FFinalRunEventOptionViewData OptionView;
		OptionView.OptionId = Option.OptionId;
		OptionView.DisplayText = Option.DisplayText.IsEmpty()
			? FText::FromName(Option.OptionId)
			: Option.DisplayText;
		OptionView.OutcomeSummary = Option.OutcomeSummary;
		OptionView.RewardEntries = FFinalRewardResolver::MakePreviewRewardEntries(Option.RewardEntries, DataRegistry);
		OptionView.RewardEntryViews = FFinalRewardResolver::BuildRewardEntryViews(OptionView.RewardEntries, DataRegistry);
		OptionView.bSelectable = !bNodeResolved && !Option.bStartsDisabled;
		OptionView.AvailabilityMessage = Option.bStartsDisabled
			? (Option.DisabledReason.IsEmpty() ? FText::FromString(TEXT("This option is currently unavailable.")) : Option.DisabledReason)
			: FText::GetEmpty();

		if (bNodeResolved)
		{
			OptionView.bSelectable = false;
			OptionView.AvailabilityMessage = FText::FromString(TEXT("This event node has already been resolved."));
		}

		if (OptionView.bSelectable)
		{
			bOutCanResolve = true;
		}

		OutOptions.Add(MoveTemp(OptionView));
	}
}
