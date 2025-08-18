#ifndef __SHIPSTORESEQUENCE_H__
#define __SHIPSTORESEQUENCE_H__

#include "Sequence.h"

struct ShapeEmitter;
struct Session;
struct MovingGeoGroup;
struct InspectObject;
struct KinStore;

struct ShipStoreSequence : Sequence
{
	enum State
	{
		SHOW,
		Count
	};

	struct MyData
	{
		int confirmFrame;
	};

	KinStore *store;

	MyData myData;
	Session *sess;

	Tileset *ts_inspect;
	sf::Vertex inspectQuad[4];

	sf::Vertex overlayRect[4];

	InspectObject *myInspectObject;

	ShipStoreSequence();
	~ShipStoreSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void AddGroups();
	void SetText(const std::string &str);
	void SetBoxPos(sf::Vector2f &pos);
	void LayeredDraw(int layer, sf::RenderTarget *target );
	void Reset();


};

#endif