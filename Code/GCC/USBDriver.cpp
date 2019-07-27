
#pragma warning(disable:4996) //Shut up warnings about std::copy being insecure
#include "GCController.h"
#include <vector>

#include "USBDriver.h"

#include <iostream>
#include <Windows.h>
#include "public.h"
#include "vjoyinterface.h"
#include "libusb.h"

#include <algorithm>
#include <cstdint>
#include <mutex>
#pragma warning(default:4996)

//WUP-028
//VENDORID 0x57E
//PRODUCT ID 0x337

const uint_fast16_t GCC_VENDOR_ID = 0x57E;
const uint_fast16_t GCC_PRODUCT_ID = 0x337;

const uint_fast16_t MAYFLASH_VENDOR_ID = 0x79;
const uint_fast16_t MAYFLASH_PRODUCT_ID = 0x1843;



/*
	Rumble. It seems it behaves by recieving a 5 byte message with 0x11 as its header, so:
	0x11, 1, 0, 0, 0
	should trigger the first controller rumble.
*/

namespace GCC
{

	std::vector<GCController> readRawInput(const std::array<unsigned char, 37>& data)
	{
		std::vector<GCController> results;
		auto port = std::begin(data) + 1;
		for (int i = 0; i < 4; port += 9, ++i)
		{
			std::array<unsigned char, 9> param;
			std::copy(port, port + param.size(), std::begin(param));
			results.emplace_back(GCController(param));
		}
		return results;
	}

	USBDriver::USBDriver()
		:mPollRate(1000)
	{ 
		int s = libusb_init(&mUSBContext);

		if (s < 0)
		{
			mStatus = Status::ERR;
			return;
		}

		libusb_set_debug(mUSBContext, 3);

		mHandle = libusb_open_device_with_vid_pid(mUSBContext, GCC_VENDOR_ID, GCC_PRODUCT_ID);

		if (!mHandle)
		{
			//mHandle = libusb_open_device_with_vid_pid(mUSBContext, MAYFLASH_VENDOR_ID, MAYFLASH_PRODUCT_ID);

			//if (!mHandle)
			//{
				mStatus = Status::ERR;
				return;
			//}
		}
		
		if (
			libusb_set_configuration(mHandle, 1) < 0 || //I don't know why it is 1, but that's what I'm seeing elsewhere being used...
			libusb_claim_interface(mHandle, 0) < 0 ||
			libusb_control_transfer(mHandle, 0x21, 11, 0x0001, 0, nullptr, 0, 0) < 0
			) 
		{
			mStatus = Status::ERR;
			return;
		}
		
		std::array<unsigned char, 37> data = { 0x13 };
		
		int transferred = 0;
		if (libusb_interrupt_transfer(mHandle, 2, data.data(), 1, &transferred, 0) < 0) //Send 0x13 to begin controller polling
		{
			mStatus = Status::ERR;
			return;
		}

		mEnabled = true;
		mThread.swap(std::thread(&USBDriver::mDriverThread, this));
		
	}

	USBDriver::~USBDriver()
	{
		if (mEnabled)
		{
			mEnabled = false;
			mThread.join();
			
		}

		if (mHandle != NULL)
		{
			libusb_release_interface(mHandle, 0);
			libusb_close(mHandle);
		}
		libusb_exit(mUSBContext);
	}

	std::vector<GCController> USBDriver::getState() const
	{
		std::vector<GCController> result;
		std::unique_lock<std::mutex> lock(mMutex);
		result = mControllers;
		lock.unlock();
		return result;
	}

	USBDriver::Status USBDriver::getStatus() const
	{
		return mStatus;
	}

	void USBDriver::mDriverThread()
	{
		int s = 0, transferred = 0;
		std::array<unsigned char, 37> data{};
		std::array<unsigned char, 5> rumble{ { 0x11, 0, 0, 0, 0 } };
		s = libusb_interrupt_transfer(mHandle, 0x02, rumble.data(), rumble.size(), &transferred, 0); //if for any reason rumble is on, turn it off.
		while (mEnabled)
		{
			std::chrono::milliseconds pollInterval(static_cast<long long>(1000. / mPollRate));
			auto start = std::chrono::steady_clock::now();

			//Get current status
			s = libusb_interrupt_transfer(mHandle, 0x81, data.data(), data.size(), &transferred, 0); //FIXME have this time out?
			auto end = std::chrono::steady_clock::now();

			double count = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

			if (s == 0)
			{
				auto controllers = readRawInput(data);
				std::unique_lock<std::mutex> lock(mMutex);
				mControllers.swap(controllers);
			}

			//assert(count < 1./mPollRate );
			std::this_thread::sleep_for(pollInterval - std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
		}
	}
}

