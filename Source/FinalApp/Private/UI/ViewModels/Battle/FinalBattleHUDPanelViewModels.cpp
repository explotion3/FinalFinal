#include "UI/ViewModels/Battle/FinalBattleHUDPanelViewModels.h"

void UFinalBattleTopBarPanelViewModel::ApplyData(const FFinalBattleTopBarPanelData& InData)
{
	Data = InData;
	BroadcastViewModelChanged();
}

const FFinalBattleTopBarPanelData& UFinalBattleTopBarPanelViewModel::GetData() const
{
	return Data;
}

void UFinalBattleResourcePanelViewModel::ApplyData(const FFinalBattleResourcePanelData& InData)
{
	Data = InData;
	BroadcastViewModelChanged();
}

const FFinalBattleResourcePanelData& UFinalBattleResourcePanelViewModel::GetData() const
{
	return Data;
}

void UFinalBattleFeedbackPanelViewModel::ApplyData(const FFinalBattleFeedbackPanelData& InData)
{
	Data = InData;
	BroadcastViewModelChanged();
}

const FFinalBattleFeedbackPanelData& UFinalBattleFeedbackPanelViewModel::GetData() const
{
	return Data;
}

void UFinalBattleContextPanelViewModel::ApplyData(const FFinalBattleContextPanelData& InData)
{
	Data = InData;
	BroadcastViewModelChanged();
}

const FFinalBattleContextPanelData& UFinalBattleContextPanelViewModel::GetData() const
{
	return Data;
}

void UFinalBattleCharacterPanelViewModel::ApplyEntries(const TArray<FFinalBattleHUDCharacterEntry>& InEntries)
{
	Entries = InEntries;
	BroadcastViewModelChanged();
}

const TArray<FFinalBattleHUDCharacterEntry>& UFinalBattleCharacterPanelViewModel::GetEntries() const
{
	return Entries;
}

void UFinalBattleEnemyPanelViewModel::ApplyEntries(const TArray<FFinalBattleHUDEnemyEntry>& InEntries)
{
	Entries = InEntries;
	BroadcastViewModelChanged();
}

const TArray<FFinalBattleHUDEnemyEntry>& UFinalBattleEnemyPanelViewModel::GetEntries() const
{
	return Entries;
}

void UFinalBattleEnemyDetailPanelViewModel::ApplyData(const FFinalBattleHUDEnemyDetailData& InData)
{
	Data = InData;
	BroadcastViewModelChanged();
}

const FFinalBattleHUDEnemyDetailData& UFinalBattleEnemyDetailPanelViewModel::GetData() const
{
	return Data;
}

void UFinalBattleHandPanelViewModel::ApplyEntries(const TArray<FFinalBattleHUDCardEntry>& InEntries)
{
	Entries = InEntries;
	BroadcastViewModelChanged();
}

const TArray<FFinalBattleHUDCardEntry>& UFinalBattleHandPanelViewModel::GetEntries() const
{
	return Entries;
}

void UFinalBattleUltimatePanelViewModel::ApplyEntries(const TArray<FFinalBattleHUDUltimateEntry>& InEntries)
{
	Entries = InEntries;
	BroadcastViewModelChanged();
}

const TArray<FFinalBattleHUDUltimateEntry>& UFinalBattleUltimatePanelViewModel::GetEntries() const
{
	return Entries;
}

void UFinalBattleRecentEventPanelViewModel::ApplyEntries(const TArray<FFinalBattleHUDLogEntry>& InEntries)
{
	Entries = InEntries;
	BroadcastViewModelChanged();
}

const TArray<FFinalBattleHUDLogEntry>& UFinalBattleRecentEventPanelViewModel::GetEntries() const
{
	return Entries;
}

void UFinalBattleActionPanelViewModel::ApplyData(const FFinalBattleActionPanelData& InData)
{
	Data = InData;
	BroadcastViewModelChanged();
}

const FFinalBattleActionPanelData& UFinalBattleActionPanelViewModel::GetData() const
{
	return Data;
}
