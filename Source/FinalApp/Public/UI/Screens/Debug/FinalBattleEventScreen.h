#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalBattleEventScreen.generated.h"

class UBorder;
class UScrollBox;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalBattleEventScreen : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void RefreshFromSubsystems();

private:
	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	void EnsureWidgetTree();
	void ResetLedgerFromBattleFlow();
	void ConsumeIncrementalEvents();
	void RebuildLedgerEntries();

	class UFinalBattleFlowSubsystem* ResolveBattleFlowSubsystem() const;
	class UFinalDataRegistry* ResolveDataRegistry() const;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PanelBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> EventScrollBox;

	UPROPERTY(Transient)
	TArray<FFinalBattleEvent> CachedLedgerEvents;

	UPROPERTY(Transient)
	FGuid CachedBattleId;

	UPROPERTY(Transient)
	int32 LastSeenEventSequence = 0;
};
