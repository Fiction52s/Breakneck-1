#ifndef __ACTION_H__
#define __ACTION_H__

#include "EditSession.h"
#include <list>

struct ISelectable
{
	enum ISelectableTypes
	{
		TERRAIN,
		ACTOR,
		GATE,
		Count
	};
	//copyable
	//is a point inside me
	//is a rectangle intersecting me
	//is a move valid
	//execute move
};

struct Action
{
	enum ActionType
	{
		CREATE_POLYGON,
		ADD_TO_POLYGON,
		CREATE_ACTOR,
		DESTROY_ACTOR,
		EDIT_ACTOR,
		MOVE_OBJECT,
		MOVE_POINTS,
		DELETE_POINTS,
		CREATE_GATE,
		DELETE_GATE,
		Count
	};
	Action();
	virtual void Perform() = 0;
	virtual void Undo() = 0;

	Action *next;
	ActionType actionType;
	static EditSession *session;
};

struct CreatePolygonAction : Action
{
	CreatePolygonAction();
	void Perform();
	void Undo();

	TerrainPolygon *createdPolygon;
};

struct AddToPolygonAction : Action
{
	AddToPolygonAction();
	void Perform();
	void Undo();

	TerrainPolygon *newPoly;
	TerrainPolygon *destroyedPolys;
};

struct CreateActorAction : Action
{
	CreateActorAction();
	ActorParams *actor;
	void Perform();
	void Undo();
};

struct DeleteActorAction : Action
{
	DeleteActorAction();
	void Perform();
	void Undo();
	
	ActorParams *actor;
};

struct EditActorAction : Action
{
	EditActorAction();
	void Perform();
	void Undo();

	ActorParams *actor;
};

struct MoveObjectAction : Action
{
	MoveObjectAction();
	void Perform();
	void Undo();

	ISelectable * obj;
	sf::Vector2i delta;
};

//should this be split up or should the others
//be in lists
struct MovePointsAction : Action
{
	MovePointsAction();
	void Perform();
	void Undo();

	std::list<TerrainPoint*> points;
	sf::Vector2i delta;
};

struct DeletePointsAction : Action
{
	DeletePointsAction();
	void Perform();
	void Undo();

	TerrainPolygon *oldPolygon; //copy
	std::list<TerrainPoint*> points;
};

struct CreateGateAction : Action
{
	CreateGateAction();
	void Perform();
	void Undo();

	GateInfo *gi;
};

struct DeleteGateAction : Action
{
	DeleteGateAction();
	void Perform();
	void Undo();

	GateInfo *gi;
};





#endif