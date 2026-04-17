#include "Battle/Effects/FinalBattleEffectBonusBreak.h"

UFinalBattleEffectBonusBreak::UFinalBattleEffectBonusBreak()
{
	EffectType = EFinalBattleEffectType::BonusBreak;
	UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
}
