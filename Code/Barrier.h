#ifndef __BARRIER_H__
#define __BARRIER_H__

#include <string>

struct GameSession;
struct BarrierCallback;

struct Barrier
{
	Barrier(GameSession *owner,
		const std::string &p_name,
		bool p_x, int pos,
		BarrierCallback *cb);

	bool Update();
	void SetPositive();
	void Reset();

	std::string name;
	BarrierCallback *callback;
	GameSession *owner;
	int pos;
	bool x; //false means y
	bool triggered;
	bool positiveOpen;
};


#endif