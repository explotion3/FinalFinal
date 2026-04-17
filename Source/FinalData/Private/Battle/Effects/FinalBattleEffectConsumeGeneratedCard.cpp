#include "Battle/Effects/FinalBattleEffectConsumeGeneratedCard.h"

UFinalBattleEffectConsumeGeneratedCard::UFinalBattleEffectConsumeGeneratedCard()
{
	EffectType = EFinalBattleEffectType::ConsumeGeneratedCard;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
