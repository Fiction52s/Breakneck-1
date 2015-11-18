#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"

struct Gate: public Edge
{
	Gate();
	enum GateType
	{
		NORMAL
	};
	GateType type;
	bool locked;
};

#endif