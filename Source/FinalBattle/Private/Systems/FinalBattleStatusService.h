#pragma once

#include "CoreMinimal.h"

struct FFinalBattleCharacterStatusesViewData;
struct FFinalBattleState;
struct FFinalBattleStatusViewData;

class FFinalBattleStatusService
{
public:
	void TickStatusWindows(FFinalBattleState& BattleState) const;
	void BuildStatusSnapshotData(
		const FFinalBattleState& BattleState,
		TArray<FFinalBattleCharacterStatusesViewData>& OutCharacterStatuses,
		TArray<FFinalBattleStatusViewData>& OutTeamStatuses,
		TArray<FFinalBattleStatusViewData>& OutStatuses) const;
};
