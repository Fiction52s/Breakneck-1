#ifndef __CONTROL_PROFILE__
#define __CONTROL_PROFILE__
#include "Input.h"
#include <string>

struct ControlProfile
{
	//you must make a named profile to store
	//controls
	ControlProfile();
	std::string name;
	XBoxButton filter[ControllerSettings::Count];
};

struct ControlProfileManager
{
	bool LoadControlProfiles();
};

#endif