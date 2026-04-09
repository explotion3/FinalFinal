#include "Battle/Effects/FinalBattleEffectDamage.h"

UFinalBattleEffectDamage::UFinalBattleEffectDamage()
{
	EffectType = EFinalBattleEffectType::Damage;
	UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
}
