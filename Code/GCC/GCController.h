#ifndef _GCC_GCController_H_
#define _GCC_GCController_H_

#include <atomic>
#include <cstdint>
#include <array>

namespace GCC
{

	class GCController
	{
	public:

		GCController();
		GCController(const std::array<unsigned char, 9>& aData);

		bool enabled;
		
		struct {
			bool y, b, x, a, start, dpad_up, dpad_down, dpad_left, dpad_right, l_shoulder, r_shoulder, z;
		} buttons;

		struct {
			std::uint_fast8_t left_x, left_y, right_x, right_y, l_axis, r_axis;
		} axis;
		
		bool powered;
		bool rumble;



	private:


	};
	
}

#endif//_GCC_GCController_H_