#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleSnapshot.h"
#include "First/Core/FirstBattleState.h"

class FFirstBattleSnapshotBuilder
{
public:
	FFirstBattleSnapshot BuildSnapshot(const FFirstBattleState& State) const;
};
