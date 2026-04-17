#include "Battle/Effects/FinalBattleEffectGainAP.h"

UFinalBattleEffectGainAP::UFinalBattleEffectGainAP()
{
	EffectType = EFinalBattleEffectType::GainAP;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
