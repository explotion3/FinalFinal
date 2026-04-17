#include "Battle/Effects/FinalBattleEffectRemoveStatus.h"

UFinalBattleEffectRemoveStatus::UFinalBattleEffectRemoveStatus()
{
	EffectType = EFinalBattleEffectType::RemoveStatus;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
