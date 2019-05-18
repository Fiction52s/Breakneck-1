
#include <Windows.h>
#include <public.h>
#include <vjoyinterface.h>
#include <cstdio>
#include <iostream>

#include "USBDriver.h"
#include "VJoyGCController.h"

namespace GCC
{
	VJoyGCController::VJoyGCController(VJoyDevice aDevice)
		:mDevice(aDevice), mStatus(Status::OFF)
	{
		//Set up joystick
		if (!vJoyEnabled())
		{
			mStatus = Status::ERR;
			return;
		}
		//TODO check that the controller selected is a good fit!
		
		VjdStat status = GetVJDStatus(mDevice);

		switch (status)
		{
		case VJD_STAT_OWN:
			std::cout << "vJoy Device " << mDevice << " %d is already owned by this feeder\n";
			break;
		case VJD_STAT_FREE:
			printf("vJoy Device %d is free\n", mDevice);
			break;
		case VJD_STAT_BUSY:
			std::cout << "vJoy Device " << mDevice << " is already owned by another feeder\nCannot continue\n";
			return;
		case VJD_STAT_MISS:
			printf("vJoy Device %d is not installed or disabled\nCannot continue\n", mDevice);
			return;
		default:
			printf("vJoy Device %d general error\nCannot continue\n", mDevice);
			return;
		}

		if ((status == VJD_STAT_OWN) || ((status == VJD_STAT_FREE) && (!AcquireVJD(mDevice))))
		{
			printf("Failed to acquire vJoy device number %d.\n", mDevice);
			return;
		}
		else
		{
			printf("Acquired: vJoy device number %d.\n", mDevice);
		}

		//setup thread
		

		
	}

	void VJoyGCController::update(const GCController& aController)
	{
		if (!aController.enabled)
		{
			return;
		}
		int s = SetBtn(aController.buttons.a, mDevice, 1);
		s = SetBtn(aController.buttons.b, mDevice, 2);
		s = SetBtn(aController.buttons.x, mDevice, 3);
		s = SetBtn(aController.buttons.y, mDevice, 4);

		s = SetBtn(aController.buttons.dpad_left, mDevice, 5);
		s = SetBtn(aController.buttons.dpad_up, mDevice, 6);
		s = SetBtn(aController.buttons.dpad_right, mDevice, 7);
		s = SetBtn(aController.buttons.dpad_down, mDevice, 8);

		s = SetBtn(aController.buttons.l_shoulder, mDevice, 9);
		s = SetBtn(aController.buttons.r_shoulder, mDevice, 10);
		s = SetBtn(aController.buttons.z, mDevice, 11);
		s = SetBtn(aController.buttons.start, mDevice, 12);

		//set axis
		s = SetAxis(aController.axis.left_x * 127, mDevice, HID_USAGE_X);
		s = SetAxis(aController.axis.left_y * 127, mDevice, HID_USAGE_Y);
		s = SetAxis(aController.axis.right_x * 127, mDevice, HID_USAGE_RX);
		s = SetAxis(aController.axis.right_y * 127, mDevice, HID_USAGE_RY);
		s = SetAxis(aController.axis.l_axis * 127, mDevice, HID_USAGE_SL0);
		s = SetAxis(aController.axis.r_axis * 127, mDevice, HID_USAGE_SL1);

		//FIXME check for 's'
	}

	VJoyGCControllers::VJoyGCControllers(const USBDriver& aDriver)
		:mDriver(aDriver), mControllers({ { VJoyGCController(1), VJoyGCController(2), VJoyGCController(3), VJoyGCController(4) } })
	{
		while (mOriginalSetup.empty())
		{
			mOriginalSetup = mDriver.getState();
		}

		mEnabled = true;
		mThread = std::thread(&VJoyGCControllers::mUpdateThread, this);
	}

	VJoyGCControllers::~VJoyGCControllers()
	{
		mEnabled = false;
		mThread.join();
		for (std::uint_fast8_t i = 0; i < mOriginalSetup.size() && i < mControllers.size(); ++i)
		{
			mControllers[i].update(mOriginalSetup[i]);
		}
	}

	void VJoyGCControllers::mUpdateThread()
	{
		
		int s = 0;
		while (mEnabled)
		{
			auto controllers(mDriver.getState());
			for (std::uint_fast8_t i = 0; i < controllers.size() && i < mControllers.size(); ++i)
			{
				mControllers[i].update(controllers[i]);
			}
		}
	}
}
