#include "Battle/Effects/FinalBattleEffectConsumeStatusResource.h"

UFinalBattleEffectConsumeStatusResource::UFinalBattleEffectConsumeStatusResource()
{
	EffectType = EFinalBattleEffectType::ConsumeStatusResource;
	UnitTargetRule = EFinalBattleUnitTargetRule::Self;
}
