#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalBattleSnapshot.h"

class FFinalBattleCardService;
class FFinalBattleStatusService;
struct FFinalBattleState;

// Battle 私有快照构建器。
// 职责：把 FFinalBattleState 投影为对外只读的 FFinalBattleSnapshot。
class FFinalBattleSnapshotBuilder
{
public:
	FFinalBattleSnapshot BuildSnapshot(
		const FFinalBattleState& State,
		const FFinalBattleCardService& CardService,
		const FFinalBattleStatusService& StatusService) const;
};
