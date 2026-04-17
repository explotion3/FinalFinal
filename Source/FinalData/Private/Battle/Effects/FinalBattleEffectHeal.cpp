#include "Battle/Effects/FinalBattleEffectHeal.h"

UFinalBattleEffectHeal::UFinalBattleEffectHeal()
{
	EffectType = EFinalBattleEffectType::Heal;
	UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
}
