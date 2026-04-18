#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalBattleStatusInstance
{
	FGuid StatusInstanceId;
	FFinalStatusId StatusId;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	FText DisplayName;
	int32 CurrentStacks = 0;
	int32 RemainingDuration = 0;
	int32 OutgoingDamagePercentPerStack = 0;
	bool bExpireAtPlayerTurnEnd = false;
	bool bConsumeOnSuccessfulOwnerDamage = false;
	bool bOnlyAffectAttackCards = false;
	int32 IncomingTeamHealthDamageReductionPercentPerStack = 0;
	bool bConsumeOnPreventedTeamHealthDamage = false;
};
