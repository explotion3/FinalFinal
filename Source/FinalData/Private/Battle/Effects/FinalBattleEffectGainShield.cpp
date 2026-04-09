#include "Battle/Effects/FinalBattleEffectGainShield.h"

UFinalBattleEffectGainShield::UFinalBattleEffectGainShield()
{
	EffectType = EFinalBattleEffectType::GainShield;
	UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
}
