#include "Battle/Effects/FinalBattleEffectMoveCards.h"

UFinalBattleEffectMoveCards::UFinalBattleEffectMoveCards()
{
	EffectType = EFinalBattleEffectType::MoveCards;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
