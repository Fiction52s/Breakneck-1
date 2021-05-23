#include "PS5Controller.h"

using namespace std;

bool PS5Controller::CheckForControllers()
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

PS5Controller::PS5Controller()
{
	enabled = false;
	ClearState();
}

void PS5Controller::ClearState()
{
	ZeroMemory(&inState, sizeof(DS5W::DS5InputState));
}

bool PS5Controller::InitContext()
{
	enabled = false;
	// Init controller and close application is failed
	if (DS5W_FAILED(DS5W::initDeviceContext(&infos[0], &context))) {
		return false;
	}

	enabled = true;

	return true;
}

void PS5Controller::CleanupContext()
{
	DS5W::freeDeviceContext(&context);
}

bool PS5Controller::Update()
{
	if (!DS5W_SUCCESS(DS5W::getDeviceInputState(&context, &inState)))
	{
		return false;
	}
}