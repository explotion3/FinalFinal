#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalPrototypeRunDebugScreen.generated.h"

class UBorder;
class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalPrototypeRunDebugScreen : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void RefreshFromSubsystems();

private:
	UFUNCTION()
	void HandleRunFlowStateChanged();

	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	UFUNCTION()
	void HandleRestartPrototypeRunClicked();

	UFUNCTION()
	void HandleCompleteResolvedBattleClicked();

	void EnsureWidgetTree();
	class UFinalBattleFlowSubsystem* ResolveBattleFlowSubsystem() const;
	class UFinalGameFlowSubsystem* ResolveGameFlowSubsystem() const;
	class UFinalRunFlowSubsystem* ResolveRunFlowSubsystem() const;
	class UFinalGameInstance* ResolveFinalGameInstance() const;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PanelBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BuildSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CharacterSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CandidateSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BattleRelicSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BattleRelicEventText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RestartRunButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RestartRunLabel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CompleteResolvedBattleButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CompleteResolvedBattleLabel;
};
