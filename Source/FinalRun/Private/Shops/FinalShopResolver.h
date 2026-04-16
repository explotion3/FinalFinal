#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Runtime/FinalRunState.h"

class UFinalDataRegistry;

struct FFinalResolvedShopOfferResult
{
	const FFinalRunShopOfferDefinition* OfferDefinition = nullptr;
	int32 SpentGold = 0;
	TArray<FFinalRunRewardEntry> PreviewEntries;
	TArray<FFinalRunRewardEntry> ResolvedEntries;
};

struct FFinalShopResolver
{
	static const FFinalRunShopOfferDefinition* FindShopOfferDefinition(
		const FFinalRunNodeDefinition& NodeDefinition,
		FName OfferId);

	static bool TryResolveShopOffer(
		const FFinalRunNodeDefinition& NodeDefinition,
		FName OfferId,
		const UFinalDataRegistry* DataRegistry,
		const FFinalRunState& RunState,
		EFinalRunCommandRejectReason& OutRejectReason,
		FText& OutFailureMessage,
		FFinalResolvedShopOfferResult& OutResult);

	static void BuildShopOfferViews(
		const FFinalRunNodeDefinition& NodeDefinition,
		const UFinalDataRegistry* DataRegistry,
		const FFinalRunState& RunState,
		bool bNodeResolved,
		TArray<FFinalRunShopOfferViewData>& OutOffers,
		bool& bOutCanResolve);
};
