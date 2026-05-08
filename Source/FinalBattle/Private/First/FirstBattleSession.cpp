#include "First/FirstBattleSession.h"

#include "First/FirstBattleKernel.h"

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
