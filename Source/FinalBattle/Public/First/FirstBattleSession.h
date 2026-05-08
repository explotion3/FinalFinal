#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleCommand.h"
#include "First/FirstBattleSnapshot.h"

class FFirstBattleKernel;
class UFinalDataRegistry;

class FINALBATTLE_API FFirstBattleSession
{
public:
	FFirstBattleSession();
	~FFirstBattleSession();

	FFirstBattleSession(FFirstBattleSession&& Other) noexcept;
	FFirstBattleSession& operator=(FFirstBattleSession&& Other) noexcept;

	FFirstBattleSession(const FFirstBattleSession&) = delete;
	FFirstBattleSession& operator=(const FFirstBattleSession&) = delete;

	void Initialize(const FFirstBattleStartParams& StartParams);
	FFirstBattleInitializeResult InitializeFromDefinitions(const FFirstBattleStartParams& StartParams, const UFinalDataRegistry& DataRegistry);
	FFirstBattleCommandResult SubmitCommand(const FFirstBattleCommand& Command);
	FFirstBattleSnapshot GetSnapshot() const;

private:
	TUniquePtr<FFirstBattleKernel> Kernel;
};
