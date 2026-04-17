#include "Battle/Effects/FinalBattleEffectApplyStatus.h"

UFinalBattleEffectApplyStatus::UFinalBattleEffectApplyStatus()
{
	EffectType = EFinalBattleEffectType::ApplyStatus;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
