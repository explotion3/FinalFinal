#include "First/FirstBattleSession.h"

#include "First/FirstCardDefinitionCompiler.h"
#include "First/Core/FirstBattleKernel.h"
#include "First/FirstCardDefinition.h"
#include "Queries/FinalDataRegistry.h"

namespace
{
	bool ValidateAndCompileCardDefinitions(
		const TArray<FFirstCardDefinitionStartEntry>& Entries,
		const UFinalDataRegistry& DataRegistry,
		TArray<FFirstCardInstance>& OutCards,
		FFirstBattleInitializeResult& InOutResult)
	{
		bool bValid = true;

		for (const FFirstCardDefinitionStartEntry& Entry : Entries)
		{
			if (Entry.CardId.IsNone() || Entry.Count <= 0)
			{
				InOutResult.InvalidCardIds.AddUnique(Entry.CardId);
				bValid = false;
				continue;
			}

			const UFirstCardDefinition* Definition = DataRegistry.FindFirstCardDefinition(Entry.CardId);
			if (!IsValid(Definition))
			{
				InOutResult.MissingCardIds.AddUnique(Entry.CardId);
				bValid = false;
				continue;
			}

			for (int32 Index = 0; Index < Entry.Count; ++Index)
			{
				OutCards.Add(FFirstCardDefinitionCompiler::CompileCardDefinition(Definition));
			}
		}

		return bValid;
	}
}

FFirstBattleSession::FFirstBattleSession()
	: Kernel(MakeUnique<FFirstBattleKernel>())
{
}

FFirstBattleSession::~FFirstBattleSession() = default;

FFirstBattleSession::FFirstBattleSession(FFirstBattleSession&& Other) noexcept = default;

FFirstBattleSession& FFirstBattleSession::operator=(FFirstBattleSession&& Other) noexcept = default;

void FFirstBattleSession::Initialize(const FFirstBattleStartParams& StartParams)
{
	if (!Kernel.IsValid())
	{
		Kernel = MakeUnique<FFirstBattleKernel>();
	}

	Kernel->Initialize(StartParams);
}

FFirstBattleInitializeResult FFirstBattleSession::InitializeFromDefinitions(const FFirstBattleStartParams& StartParams, const UFinalDataRegistry& DataRegistry)
{
	FFirstBattleInitializeResult Result;
	FFirstBattleStartParams RuntimeStartParams = StartParams;

	const bool bHandDefinitionsValid = ValidateAndCompileCardDefinitions(
		StartParams.InitialHandCardDefinitions,
		DataRegistry,
		RuntimeStartParams.InitialHand,
		Result);
	const bool bDrawPileDefinitionsValid = ValidateAndCompileCardDefinitions(
		StartParams.InitialDrawPileCardDefinitions,
		DataRegistry,
		RuntimeStartParams.InitialDrawPile,
		Result);

	if (!bHandDefinitionsValid || !bDrawPileDefinitionsValid)
	{
		Result.bSuccess = false;
		Result.Message = NSLOCTEXT("FirstBattle", "FirstInitializeFromDefinitionsFailed", "First battle failed to initialize from card definitions.");
		return Result;
	}

	Initialize(RuntimeStartParams);

	Result.bSuccess = true;
	Result.Message = NSLOCTEXT("FirstBattle", "FirstInitializeFromDefinitionsSucceeded", "First battle initialized from card definitions.");
	return Result;
}

FFirstBattleCommandResult FFirstBattleSession::SubmitCommand(const FFirstBattleCommand& Command)
{
	if (!Kernel.IsValid())
	{
		Kernel = MakeUnique<FFirstBattleKernel>();
	}

	return Kernel->SubmitCommand(Command);
}

FFirstBattleSnapshot FFirstBattleSession::GetSnapshot() const
{
	if (!Kernel.IsValid())
	{
		return FFirstBattleSnapshot();
	}

	return Kernel->BuildSnapshot();
}
