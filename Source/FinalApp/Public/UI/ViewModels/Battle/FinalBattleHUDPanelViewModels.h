#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/ViewModels/FinalViewModelBase.h"
#include "FinalBattleHUDPanelViewModels.generated.h"

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleTopBarPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleTopBarPanelData& InData);
	const FFinalBattleTopBarPanelData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleTopBarPanelData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleResourcePanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleResourcePanelData& InData);
	const FFinalBattleResourcePanelData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleResourcePanelData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleFeedbackPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleFeedbackPanelData& InData);
	const FFinalBattleFeedbackPanelData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleFeedbackPanelData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleContextPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleContextPanelData& InData);
	const FFinalBattleContextPanelData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleContextPanelData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleCharacterPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyEntries(const TArray<FFinalBattleHUDCharacterEntry>& InEntries);
	const TArray<FFinalBattleHUDCharacterEntry>& GetEntries() const;

private:
	UPROPERTY(Transient)
	TArray<FFinalBattleHUDCharacterEntry> Entries;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleEnemyPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyEntries(const TArray<FFinalBattleHUDEnemyEntry>& InEntries);
	const TArray<FFinalBattleHUDEnemyEntry>& GetEntries() const;

private:
	UPROPERTY(Transient)
	TArray<FFinalBattleHUDEnemyEntry> Entries;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleEnemyDetailPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleHUDEnemyDetailData& InData);
	const FFinalBattleHUDEnemyDetailData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleHUDEnemyDetailData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleCharacterDetailPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleHUDCharacterDetailData& InData);
	const FFinalBattleHUDCharacterDetailData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleHUDCharacterDetailData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleHandPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyEntries(const TArray<FFinalBattleHUDCardEntry>& InEntries);
	const TArray<FFinalBattleHUDCardEntry>& GetEntries() const;

private:
	UPROPERTY(Transient)
	TArray<FFinalBattleHUDCardEntry> Entries;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleCardZoneDetailPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleHUDCardZoneDetailData& InData);
	const FFinalBattleHUDCardZoneDetailData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleHUDCardZoneDetailData Data;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleUltimatePanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyEntries(const TArray<FFinalBattleHUDUltimateEntry>& InEntries);
	const TArray<FFinalBattleHUDUltimateEntry>& GetEntries() const;

private:
	UPROPERTY(Transient)
	TArray<FFinalBattleHUDUltimateEntry> Entries;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleRecentEventPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyEntries(const TArray<FFinalBattleHUDLogEntry>& InEntries);
	const TArray<FFinalBattleHUDLogEntry>& GetEntries() const;

private:
	UPROPERTY(Transient)
	TArray<FFinalBattleHUDLogEntry> Entries;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleActionPanelViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void ApplyData(const FFinalBattleActionPanelData& InData);
	const FFinalBattleActionPanelData& GetData() const;

private:
	UPROPERTY(Transient)
	FFinalBattleActionPanelData Data;
};
