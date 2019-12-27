#ifndef __BARRIER_H__
#define __BARRIER_H__

#include <string>
#include <SFML/Graphics.hpp>

struct GameSession;
struct BarrierCallback;
struct Edge;

struct Barrier
{
	Barrier(GameSession *owner,
		const std::string &p_name,
		bool p_x, int pos,
		BarrierCallback *cb);
	~Barrier();
	bool Update();
	void SetPositive();
	void Reset();
	void DebugDraw(sf::RenderTarget *target);

	std::string name;
	BarrierCallback *callback;
	GameSession *owner;
	int pos;
	bool x; //false means y
	bool triggered;
	bool positiveOpen;
	bool edgeActive;
	Edge *barrierEdge;
	sf::Vertex line[2];
};


#endif