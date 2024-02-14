
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

using namespace std;

//WUP-028
//VENDORID 0x57E
//PRODUCT ID 0x337

const int NUM_USB_TYPES = 1;
uint_fast16_t vendorID[NUM_USB_TYPES] = { 0x57E };// , 0x79, 0x79, 0x79, 0x79};
uint_fast16_t productID[NUM_USB_TYPES] = { 0x337 };// , 0x1843, 0x1844, 0x1846, 0x1800};

const uint_fast16_t GCC_VENDOR_ID = 0x57E;
const uint_fast16_t GCC_PRODUCT_ID = 0x337;

const uint_fast16_t MAYFLASH_VENDOR_ID = 0x79;
const uint_fast16_t MAYFLASH_PRODUCT_ID = 0x1846;

const uint_fast16_t MAYFLASH_VENDOR_ID_2 = 0x79;
const uint_fast16_t MAYFLASH_PRODUCT_ID_2 = 0x1843;

const uint_fast16_t MAYFLASH_VENDOR_ID_3 = 0x79;
const uint_fast16_t MAYFLASH_PRODUCT_ID_3 = 0x1844;



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

		mEnabled = false;
		mStatus = Status::READY;

		int s = libusb_init(&mUSBContext);

		if (s < 0)
		{
			//cout << "error initializing libusb" << endl;
			mStatus = Status::ERR;
			return;
		}

		//list all usb devices
		//libusb_device **device_list;
		//libusb_device_handle *handle = NULL;
		//int deviceCount = libusb_get_device_list(mUSBContext, &device_list);

		//int i;
		//for (i = 0; i < deviceCount; i++) {
		//	struct libusb_device* device = device_list[i];
		//	struct libusb_device_descriptor desc;
		//	libusb_get_device_descriptor(device, &desc);

		//	cout << std::hex << "DEVICE---   " << "vendor: 0x" << desc.idVendor << ", product: 0x" << desc.idProduct << endl;
		//	/*if (desc.idVendor == VENDOR_ID && desc.idProduct == devid) {
		//		libusb_open(device, &handle);
		//		break;*/
		//}


		//
		//libusb_free_device_list(device_list, 1);



		libusb_set_debug(mUSBContext, 1);


		mHandle = NULL;
		/*for (int i = 0; i < NUM_USB_TYPES; ++i)
		{
			mHandle = libusb_open_device_with_vid_pid(mUSBContext, vendorID[i], productID[i]);
			if (mHandle != NULL)
			{
				break;
			}
		}*/

		
		mHandle = libusb_open_device_with_vid_pid(mUSBContext, vendorID[0], productID[0]);
		if (mHandle == NULL)
		{
			//mStatus = Status::ERR;
			mStatus = Status::ERR;
			//cout << "error opening usb device. Might just not be present" << endl;
			return;
		}


		//not sure if i need this
		//int result = libusb_claim_interface(mHandle, 0);
		//if (result < 0) {
		//	std::cerr << "Error claiming interface: " << libusb_error_name(result) << std::endl;
		//	libusb_close(mHandle);
		//	mStatus = Status::ERR;
		//	//libusb_exit(context);
		//	//return result;
		//	return;
		//}
		

		if (!mHandle)
		{
			mStatus = Status::ERR;
			return;
		}

		/*mHandle = libusb_open_device_with_vid_pid(mUSBContext, GCC_VENDOR_ID, GCC_PRODUCT_ID);

		if (!mHandle)
		{
			mHandle = libusb_open_device_with_vid_pid(mUSBContext, MAYFLASH_VENDOR_ID, MAYFLASH_PRODUCT_ID);

			if (!mHandle)
			{
				mStatus = Status::ERR;
				return;
			}
		}*/

		bool a = false, b = false, c = false;
		a = libusb_set_configuration(mHandle, 1) < 0;//I don't know why it is 1, but that's what I'm seeing elsewhere being used...
		if (!a)
		{
			b = libusb_claim_interface(mHandle, 0) < 0;

			if (!b)
			{
				c = libusb_control_transfer(mHandle, 0x21, 11, 0x0001, 0, nullptr, 0, 0) < 0;
			}
		}

		if (a || b || c )
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

