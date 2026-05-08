#pragma once

#include "CoreMinimal.h"
#include "First/Cards/FirstBattleCardResolver.h"
#include "First/Core/FirstBattleState.h"
#include "First/Deck/FirstBattleDeckService.h"
#include "First/Enemy/FirstBattleEnemyPartService.h"
#include "First/Events/FirstBattleEventLog.h"
#include "First/FirstBattleCommand.h"
#include "First/FirstBattleSnapshot.h"
#include "First/Hand/FirstBattleHandService.h"
#include "First/Snapshots/FirstBattleSnapshotBuilder.h"

class FFirstBattleKernel
{
public:
	void Initialize(const FFirstBattleStartParams& StartParams);
	FFirstBattleCommandResult SubmitCommand(const FFirstBattleCommand& Command);
	FFirstBattleSnapshot BuildSnapshot() const;

private:
	FFirstBattleState State;
	FFirstBattleEventLog EventLog;
	FFirstBattleHandService HandService;
	FFirstBattleDeckService DeckService;
	FFirstBattleEnemyPartService EnemyPartService;
	FFirstBattleCardResolver CardResolver;
	FFirstBattleSnapshotBuilder SnapshotBuilder;

	FFirstBattleCommandResult ResolvePlayCard(const FFirstBattleCommand& Command);
	FFirstBattleCommandResult ResolveEndTurn();
};
