#ifndef _GCC_USBDRIVER_H_
#define _GCC_USBDRIVER_H_

#include "GCController.h"

#include <libusb.h>
#include <thread>
#include <vector>
#include <mutex>

namespace GCC
{
	class USBDriver
	{
	public:
		enum class Status
		{
			READY, ERR
		};
		USBDriver();
		~USBDriver();

		std::vector<GCController> getState() const;
		Status getStatus() const;

	private:
		mutable std::mutex mMutex;
		std::vector<GCController> mControllers;
		Status mStatus;
		libusb_context *mUSBContext;


		std::atomic<bool> mEnabled;
		double mPollRate; //In Hz
		std::thread mThread;

		void mDriverThread();
		libusb_device_handle *mHandle;


	};
}
#endif//_GCC_USBDRIVER_H_
