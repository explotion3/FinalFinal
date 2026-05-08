#include "Bootstrap/FirstPrototypeContentBundleBuilder.h"

#include "Bootstrap/FinalPrototypeContentBootstrapAssetUtils.h"

#include "First/FirstCardDefinition.h"

namespace FirstPrototypeContentBootstrap
{
	const FString RootPath(TEXT("/Game/Prototype/FirstProject"));
	const FString CardsPath = RootPath / TEXT("Cards");

	const FString LeftHandCardPath = CardsPath / TEXT("DA_FirstCard_Core_LeftHand");
	const FString RightHandCardPath = CardsPath / TEXT("DA_FirstCard_Core_RightHand");
	const FString ChaoGuangMuDieCardPath = CardsPath / TEXT("DA_FirstCard_Insect_ChaoGuangMuDie");
	const FString ChiFuGongYiCardPath = CardsPath / TEXT("DA_FirstCard_Insect_ChiFuGongYi");
	const FString ShuoGuangDieCardPath = CardsPath / TEXT("DA_FirstCard_Insect_ShuoGuangDie");

	const FName LeftHandCardId(TEXT("first.card.core.left_hand"));
	const FName RightHandCardId(TEXT("first.card.core.right_hand"));
	const FName ChaoGuangMuDieCardId(TEXT("first.card.insect.chao_guang_mu_die"));
	const FName ChiFuGongYiCardId(TEXT("first.card.insect.chi_fu_gong_yi"));
	const FName ShuoGuangDieCardId(TEXT("first.card.insect.shuo_guang_die"));

	void ResetFirstCard(UFirstCardDefinition& Card, const FName CardId, const TCHAR* DisplayName, const int32 BaseCost)
	{
		Card.CardId = CardId;
		Card.DisplayName = FText::FromString(DisplayName);
		Card.BaseCost = BaseCost;
		Card.PlayerMaxHPBonusOnEnterBattle = 0;
		Card.PlayDestination = EFirstCardDefinitionPlayDestination::DiscardPile;
		Card.Keywords.Reset();
		Card.HandRole = EFirstCardDefinitionHandRole::None;
		Card.bRequiresHandZoneToPlay = false;
		Card.RequiredHandZone = EFirstCardDefinitionHandZone::None;
		Card.bSkipInitiativeReductionOnPerfectReleaseInZone = false;
		Card.PerfectReleaseInitiativeSkipZone = EFirstCardDefinitionHandZone::None;
		Card.Effects.Reset();
	}

	void AddDamageEffect(UFirstCardDefinition& Card, const FName EffectId, const int32 Value)
	{
		FFirstCardDefinitionEffect& DamageEffect = Card.Effects.AddDefaulted_GetRef();
		DamageEffect.EffectId = EffectId;
		DamageEffect.EffectType = EFirstCardDefinitionEffectType::Damage;
		DamageEffect.Value = Value;
	}

	FFirstCardDefinitionEffect& AddMoveEffect(UFirstCardDefinition& Card, const FName EffectId)
	{
		FFirstCardDefinitionEffect& MoveEffect = Card.Effects.AddDefaulted_GetRef();
		MoveEffect.EffectId = EffectId;
		MoveEffect.EffectType = EFirstCardDefinitionEffectType::MoveHandCard;
		MoveEffect.MoveCardCount = 1;
		MoveEffect.bMoveRequiresSourceZone = false;
		MoveEffect.MoveSourceZone = EFirstCardDefinitionHandZone::None;
		MoveEffect.MoveTargetPolicy = EFirstCardDefinitionHandMoveTargetPolicy::RandomValidZone;
		MoveEffect.MoveTargetZone = EFirstCardDefinitionHandZone::None;
		MoveEffect.MoveTargetCostDelta = 0;
		MoveEffect.bTransferActualCostReductionToSourceCard = false;
		return MoveEffect;
	}
}

void FFirstPrototypeContentBundleBuilder::Build(TSet<UPackage*>& PackagesToSave)
{
	using namespace FirstPrototypeContentBootstrap;
	using namespace FinalPrototypeContentBootstrap;

	bool bCreatedAsset = false;

	UFirstCardDefinition* LeftHandCard = LoadOrCreateAsset<UFirstCardDefinition>(LeftHandCardPath, bCreatedAsset);
	ResetFirstCard(*LeftHandCard, LeftHandCardId, TEXT("左手"), 2);
	LeftHandCard->HandRole = EFirstCardDefinitionHandRole::LeftHandCore;
	TrackPackage(LeftHandCard, PackagesToSave);

	UFirstCardDefinition* RightHandCard = LoadOrCreateAsset<UFirstCardDefinition>(RightHandCardPath, bCreatedAsset);
	ResetFirstCard(*RightHandCard, RightHandCardId, TEXT("右手"), 2);
	RightHandCard->HandRole = EFirstCardDefinitionHandRole::RightHandCore;
	AddDamageEffect(*RightHandCard, TEXT("effect.first.right_hand.damage"), 8);
	TrackPackage(RightHandCard, PackagesToSave);

	UFirstCardDefinition* ChaoGuangMuDieCard = LoadOrCreateAsset<UFirstCardDefinition>(ChaoGuangMuDieCardPath, bCreatedAsset);
	ResetFirstCard(*ChaoGuangMuDieCard, ChaoGuangMuDieCardId, TEXT("朝光暮蝶"), 0);
	ChaoGuangMuDieCard->PlayerMaxHPBonusOnEnterBattle = 1;
	{
		FFirstCardDefinitionEffect& MoveEffect = AddMoveEffect(*ChaoGuangMuDieCard, TEXT("effect.first.chao_guang_mu_die.move_cost_transfer"));
		MoveEffect.MoveTargetPolicy = EFirstCardDefinitionHandMoveTargetPolicy::RandomValidZone;
		MoveEffect.MoveTargetCostDelta = -1;
		MoveEffect.bTransferActualCostReductionToSourceCard = true;
	}
	TrackPackage(ChaoGuangMuDieCard, PackagesToSave);

	UFirstCardDefinition* ChiFuGongYiCard = LoadOrCreateAsset<UFirstCardDefinition>(ChiFuGongYiCardPath, bCreatedAsset);
	ResetFirstCard(*ChiFuGongYiCard, ChiFuGongYiCardId, TEXT("赤腹工蚁"), 0);
	ChiFuGongYiCard->PlayerMaxHPBonusOnEnterBattle = 1;
	ChiFuGongYiCard->bRequiresHandZoneToPlay = true;
	ChiFuGongYiCard->RequiredHandZone = EFirstCardDefinitionHandZone::Both;
	{
		FFirstCardDefinitionEffect& MoveEffect = AddMoveEffect(*ChiFuGongYiCard, TEXT("effect.first.chi_fu_gong_yi.move_both_to_other"));
		MoveEffect.bMoveRequiresSourceZone = true;
		MoveEffect.MoveSourceZone = EFirstCardDefinitionHandZone::Both;
		MoveEffect.MoveTargetPolicy = EFirstCardDefinitionHandMoveTargetPolicy::RandomOtherThanSourceZone;
	}
	TrackPackage(ChiFuGongYiCard, PackagesToSave);

	UFirstCardDefinition* ShuoGuangDieCard = LoadOrCreateAsset<UFirstCardDefinition>(ShuoGuangDieCardPath, bCreatedAsset);
	ResetFirstCard(*ShuoGuangDieCard, ShuoGuangDieCardId, TEXT("烁光蝶"), 1);
	ShuoGuangDieCard->PlayerMaxHPBonusOnEnterBattle = 6;
	ShuoGuangDieCard->PlayDestination = EFirstCardDefinitionPlayDestination::ReturnToHandRandomZone;
	AddDamageEffect(*ShuoGuangDieCard, TEXT("effect.first.shuo_guang_die.damage"), 7);
	TrackPackage(ShuoGuangDieCard, PackagesToSave);
}
