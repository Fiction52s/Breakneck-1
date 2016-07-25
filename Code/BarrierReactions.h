#ifndef __BARRIER_REACTIONS__
#define __BARRIER_REACTIONS__


struct GameSession;
struct PoiInfo;
struct BarrierCallback;

struct BarrierCallback
{
	virtual void BarrierTrigger( PoiInfo * poi ) = 0;
};


#endif