#include "Controllers/FinalBattleWidgetController.h"

#include "Facade/FinalBattleSession.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleWidgetController::Initialize(UFinalBattleHUDViewModel* InViewModel)
{
	ViewModel = InViewModel;
}

void UFinalBattleWidgetController::RefreshFromSession(UFinalBattleSession* Session)
{
	if (!ViewModel || !Session)
	{
		return;
	}

	ViewModel->ApplySnapshot(Session->GetSnapshot());
}

UFinalBattleHUDViewModel* UFinalBattleWidgetController::GetViewModel() const
{
	return ViewModel;
}
