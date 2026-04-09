#include "Battle/Effects/FinalBattleEffectDrawCards.h"

UFinalBattleEffectDrawCards::UFinalBattleEffectDrawCards()
{
	EffectType = EFinalBattleEffectType::DrawCards;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
