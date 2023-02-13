#include "PS5Controller.h"
#include "Input.h"
#include "MainMenu.h"

using namespace std;

const float PS5Controller::deadZone = .3f;

PS5ControllerManager::PS5ControllerManager()
{
	numControllers = 0;
}

bool PS5ControllerManager::CheckForControllers()
{
	switch (DS5W::enumDevices(infos, 4, &numControllers)) {
	case DS5W_OK:
		// The buffer was not big enough. Ignore for now
	case DS5W_E_INSUFFICIENT_BUFFER:
		break;

		// Any other error will terminate the application
	default:
		// Insert your error handling
		return false;
	}

	// Check number of controllers
	if (!numControllers) {
		return false;
	}

	return true;
}

void PS5ControllerManager::InitControllers(MainMenu *mm)
{
	for (int i = 0; i < numControllers; ++i)
	{
		//mm->GetController(i)->ps5Controller.InitContext(infos[i]);
	}

}

PS5Controller::PS5Controller()
{
	enabled = false;
	ClearState();
}

PS5Controller::~PS5Controller()
{
	CleanupContext();
}

void PS5Controller::ClearState()
{
	ZeroMemory(&inState, sizeof(DS5W::DS5InputState));
}

bool PS5Controller::InitContext( DS5W::DeviceEnumInfo &info)
{
	enabled = false;
	// Init controller and close application is failed
	if (DS5W_FAILED(DS5W::initDeviceContext(&info, &context))) {
		return false;
	}

	enabled = true;

	return true;
}

void PS5Controller::CleanupContext()
{
	if (enabled)
	{
		DS5W::freeDeviceContext(&context);
	}
}

bool PS5Controller::UpdateState()
{
	ClearState();
	if (!DS5W_SUCCESS(DS5W::getDeviceInputState(&context, &inState)))
	{
		return false;
	}
}