#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"

class UFinalDataRegistry;

struct FFinalResolvedEventOptionResult
{
	const FFinalRunEventOptionDefinition* OptionDefinition = nullptr;
	TArray<FFinalRunRewardEntry> PreviewEntries;
	TArray<FFinalRunRewardEntry> ResolvedEntries;
};

struct FFinalRunEventResolver
{
	static const FFinalRunEventOptionDefinition* FindEventOptionDefinition(
		const FFinalRunNodeDefinition& NodeDefinition,
		FName OptionId);

	static bool TryResolveEventOption(
		const FFinalRunNodeDefinition& NodeDefinition,
		FName OptionId,
		const UFinalDataRegistry* DataRegistry,
		EFinalRunCommandRejectReason& OutRejectReason,
		FText& OutFailureMessage,
		FFinalResolvedEventOptionResult& OutResult);

	static void BuildEventOptionViews(
		const FFinalRunNodeDefinition& NodeDefinition,
		const UFinalDataRegistry* DataRegistry,
		bool bNodeResolved,
		TArray<FFinalRunEventOptionViewData>& OutOptions,
		bool& bOutCanResolve);
};
