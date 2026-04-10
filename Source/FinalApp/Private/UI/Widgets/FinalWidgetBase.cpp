#include "UI/Widgets/FinalWidgetBase.h"

#include "UI/Controllers/FinalWidgetControllerBase.h"
#include "UI/ViewModels/FinalViewModelBase.h"

void UFinalWidgetBase::SetPresentationContext(UFinalWidgetControllerBase* InWidgetController, UFinalViewModelBase* InViewModel)
{
	WidgetController = InWidgetController;
	ViewModel = InViewModel;
}

UFinalWidgetControllerBase* UFinalWidgetBase::GetWidgetController() const
{
	return WidgetController;
}

UFinalViewModelBase* UFinalWidgetBase::GetViewModel() const
{
	return ViewModel;
}
