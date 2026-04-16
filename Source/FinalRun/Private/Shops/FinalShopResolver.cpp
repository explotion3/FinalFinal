#include "Shops/FinalShopResolver.h"

#include "Queries/FinalDataRegistry.h"
#include "Rewards/FinalRewardResolver.h"

const FFinalRunShopOfferDefinition* FFinalShopResolver::FindShopOfferDefinition(
	const FFinalRunNodeDefinition& NodeDefinition,
	const FName OfferId)
{
	return NodeDefinition.ShopContent.Offers.FindByPredicate([&OfferId](const FFinalRunShopOfferDefinition& Offer)
	{
		return Offer.OfferId == OfferId;
	});
}

bool FFinalShopResolver::TryResolveShopOffer(
	const FFinalRunNodeDefinition& NodeDefinition,
	const FName OfferId,
	const UFinalDataRegistry* DataRegistry,
	const FFinalRunState& RunState,
	EFinalRunCommandRejectReason& OutRejectReason,
	FText& OutFailureMessage,
	FFinalResolvedShopOfferResult& OutResult)
{
	if (OfferId.IsNone())
	{
		OutRejectReason = EFinalRunCommandRejectReason::MissingPayloadId;
		OutFailureMessage = FText::FromString(TEXT("ResolveShop requires a shop offer id in PayloadId."));
		return false;
	}

	const FFinalRunShopOfferDefinition* SelectedOffer = FindShopOfferDefinition(NodeDefinition, OfferId);
	if (SelectedOffer == nullptr)
	{
		OutRejectReason = EFinalRunCommandRejectReason::UnknownShopOffer;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalShopResolver", "UnknownShopOffer", "Shop offer {0} is not defined on the current node."),
			FText::FromName(OfferId));
		return false;
	}

	if (SelectedOffer->bStartsUnavailable)
	{
		OutRejectReason = EFinalRunCommandRejectReason::ShopOfferUnavailable;
		OutFailureMessage = SelectedOffer->UnavailableReason.IsEmpty()
			? FText::FromString(TEXT("The selected shop offer is currently unavailable."))
			: SelectedOffer->UnavailableReason;
		return false;
	}

	if (RunState.Gold < SelectedOffer->Price)
	{
		OutRejectReason = EFinalRunCommandRejectReason::InsufficientGold;
		OutFailureMessage = FText::Format(
			NSLOCTEXT("FinalShopResolver", "InsufficientGold", "The selected shop offer costs {0} gold, but the run only has {1}."),
			FText::AsNumber(SelectedOffer->Price),
			FText::AsNumber(RunState.Gold));
		return false;
	}

	OutResult.OfferDefinition = SelectedOffer;
	OutResult.SpentGold = SelectedOffer->Price;
	OutResult.PreviewEntries = FFinalRewardResolver::MakePreviewRewardEntries(SelectedOffer->RewardEntries, DataRegistry);
	OutResult.ResolvedEntries = FFinalRewardResolver::MakeClaimedRewardEntries(SelectedOffer->RewardEntries, DataRegistry);
	return true;
}

void FFinalShopResolver::BuildShopOfferViews(
	const FFinalRunNodeDefinition& NodeDefinition,
	const UFinalDataRegistry* DataRegistry,
	const FFinalRunState& RunState,
	const bool bNodeResolved,
	TArray<FFinalRunShopOfferViewData>& OutOffers,
	bool& bOutCanResolve)
{
	OutOffers.Reset();
	bOutCanResolve = false;

	for (const FFinalRunShopOfferDefinition& Offer : NodeDefinition.ShopContent.Offers)
	{
		FFinalRunShopOfferViewData OfferView;
		OfferView.OfferId = Offer.OfferId;
		OfferView.DisplayId = Offer.DisplayId;
		OfferView.DisplayName = Offer.DisplayName.IsEmpty()
			? FText::FromName(Offer.OfferId)
			: Offer.DisplayName;
		OfferView.Description = Offer.Description;
		OfferView.Price = Offer.Price;
		OfferView.bPurchased = bNodeResolved;
		OfferView.RewardEntries = FFinalRewardResolver::MakePreviewRewardEntries(Offer.RewardEntries, DataRegistry);
		OfferView.RewardEntryViews = FFinalRewardResolver::BuildRewardEntryViews(OfferView.RewardEntries, DataRegistry);

		if (bNodeResolved)
		{
			OfferView.bPurchasable = false;
			OfferView.AvailabilityMessage = FText::FromString(TEXT("This shop node has already been resolved."));
		}
		else if (Offer.bStartsUnavailable)
		{
			OfferView.bPurchasable = false;
			OfferView.AvailabilityMessage = Offer.UnavailableReason.IsEmpty()
				? FText::FromString(TEXT("This offer is currently unavailable."))
				: Offer.UnavailableReason;
		}
		else if (RunState.Gold < Offer.Price)
		{
			OfferView.bPurchasable = false;
			OfferView.AvailabilityMessage = FText::Format(
				NSLOCTEXT("FinalShopResolver", "ShopOfferNeedsMoreGold", "Requires {0} gold."),
				FText::AsNumber(Offer.Price));
		}
		else
		{
			OfferView.bPurchasable = true;
			bOutCanResolve = true;
		}

		OutOffers.Add(MoveTemp(OfferView));
	}
}
