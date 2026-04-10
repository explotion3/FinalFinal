#include "UI/ViewModels/FinalViewModelBase.h"

void UFinalViewModelBase::BroadcastViewModelChanged()
{
	OnViewModelChanged.Broadcast();
}
