#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleHUDViewModel::EnsurePanelViewModels()
{
	if (TopBarViewModel == nullptr)
	{
		TopBarViewModel = NewObject<UFinalBattleTopBarPanelViewModel>(this);
	}

	if (ResourceViewModel == nullptr)
	{
		ResourceViewModel = NewObject<UFinalBattleResourcePanelViewModel>(this);
	}

	if (FeedbackViewModel == nullptr)
	{
		FeedbackViewModel = NewObject<UFinalBattleFeedbackPanelViewModel>(this);
	}

	if (ContextViewModel == nullptr)
	{
		ContextViewModel = NewObject<UFinalBattleContextPanelViewModel>(this);
	}

	if (CharacterViewModel == nullptr)
	{
		CharacterViewModel = NewObject<UFinalBattleCharacterPanelViewModel>(this);
	}

	if (EnemyViewModel == nullptr)
	{
		EnemyViewModel = NewObject<UFinalBattleEnemyPanelViewModel>(this);
	}

	if (EnemyDetailViewModel == nullptr)
	{
		EnemyDetailViewModel = NewObject<UFinalBattleEnemyDetailPanelViewModel>(this);
	}

	if (CharacterDetailViewModel == nullptr)
	{
		CharacterDetailViewModel = NewObject<UFinalBattleCharacterDetailPanelViewModel>(this);
	}

	if (HandViewModel == nullptr)
	{
		HandViewModel = NewObject<UFinalBattleHandPanelViewModel>(this);
	}

	if (CardZoneDetailViewModel == nullptr)
	{
		CardZoneDetailViewModel = NewObject<UFinalBattleCardZoneDetailPanelViewModel>(this);
	}

	if (UltimateViewModel == nullptr)
	{
		UltimateViewModel = NewObject<UFinalBattleUltimatePanelViewModel>(this);
	}

	if (RecentEventViewModel == nullptr)
	{
		RecentEventViewModel = NewObject<UFinalBattleRecentEventPanelViewModel>(this);
	}

	if (ActionViewModel == nullptr)
	{
		ActionViewModel = NewObject<UFinalBattleActionPanelViewModel>(this);
	}
}

void UFinalBattleHUDViewModel::ApplySnapshot(const FFinalBattleSnapshot& InSnapshot)
{
	Snapshot = InSnapshot;
}

void UFinalBattleHUDViewModel::ApplyBattleEvent(const FFinalBattleEvent& BattleEvent)
{
	if (BattleEvent.EventType != EFinalBattleEventType::PhaseChanged)
	{
		return;
	}

	LatestPhaseChangedEvent = BattleEvent;
	OnPhaseChangedPresentation.Broadcast(BattleEvent);
}

FFinalBattleSnapshot UFinalBattleHUDViewModel::GetSnapshot() const
{
	return Snapshot;
}

FFinalBattleEvent UFinalBattleHUDViewModel::GetLatestPhaseChangedEvent() const
{
	return LatestPhaseChangedEvent;
}

UFinalBattleTopBarPanelViewModel* UFinalBattleHUDViewModel::GetTopBarViewModel() const
{
	return TopBarViewModel;
}

UFinalBattleResourcePanelViewModel* UFinalBattleHUDViewModel::GetResourceViewModel() const
{
	return ResourceViewModel;
}

UFinalBattleFeedbackPanelViewModel* UFinalBattleHUDViewModel::GetFeedbackViewModel() const
{
	return FeedbackViewModel;
}

UFinalBattleContextPanelViewModel* UFinalBattleHUDViewModel::GetContextViewModel() const
{
	return ContextViewModel;
}

UFinalBattleCharacterPanelViewModel* UFinalBattleHUDViewModel::GetCharacterViewModel() const
{
	return CharacterViewModel;
}

UFinalBattleEnemyPanelViewModel* UFinalBattleHUDViewModel::GetEnemyViewModel() const
{
	return EnemyViewModel;
}

UFinalBattleEnemyDetailPanelViewModel* UFinalBattleHUDViewModel::GetEnemyDetailViewModel() const
{
	return EnemyDetailViewModel;
}

UFinalBattleCharacterDetailPanelViewModel* UFinalBattleHUDViewModel::GetCharacterDetailViewModel() const
{
	return CharacterDetailViewModel;
}

UFinalBattleHandPanelViewModel* UFinalBattleHUDViewModel::GetHandViewModel() const
{
	return HandViewModel;
}

UFinalBattleCardZoneDetailPanelViewModel* UFinalBattleHUDViewModel::GetCardZoneDetailViewModel() const
{
	return CardZoneDetailViewModel;
}

UFinalBattleUltimatePanelViewModel* UFinalBattleHUDViewModel::GetUltimateViewModel() const
{
	return UltimateViewModel;
}

UFinalBattleRecentEventPanelViewModel* UFinalBattleHUDViewModel::GetRecentEventViewModel() const
{
	return RecentEventViewModel;
}

UFinalBattleActionPanelViewModel* UFinalBattleHUDViewModel::GetActionViewModel() const
{
	return ActionViewModel;
}
