#ifndef __BARRIER_H__
#define __BARRIER_H__

#include <string>
#include <SFML/Graphics.hpp>

struct GameSession;
struct BarrierCallback;
struct Edge;
struct Sequence;

struct Barrier
{
	Barrier(GameSession *owner,
		const std::string &p_name,
		bool p_x, int pos, bool hasEdge,
		BarrierCallback *cb);
	~Barrier();
	bool Update();
	void SetPositive();
	void Reset();
	void Trigger();
	void DeactivateEdge();
	void ActivateEdge();
	void DebugDraw(sf::RenderTarget *target);
	bool IsPointWithinBarrier(sf::Vector2<double> &p);
	double GetPlayerDist();
	void SetScene();
	int GetCamPos();
	//void SetSequence( )
	std::string name;
	BarrierCallback *callback;
	GameSession *owner;
	
	bool x; //false means y
	bool triggered;
	bool positiveOpen;
	bool edgeActive;
	Edge *barrierEdge;
	sf::Vertex line[2];
	Sequence *triggerSeq;
	int extraDistance;
	int pos;
private:
	
};


#endif