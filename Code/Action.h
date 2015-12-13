#ifndef __ACTION_H__
#define __ACTION_H__

#include "EditSession.h"
#include <list>

struct ISelectable
{
	enum ISelectableType
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
	ISelectable( ISelectableType type );
	virtual bool ContainsPoint( sf::Vector2i point ) = 0;
	virtual bool Intersects( sf::IntRect rect ) = 0;
	virtual bool IsMoveOkay( sf::Vector2i delta ) = 0;
	virtual void Move( sf::Vector2i delta ) = 0;
	virtual void BrushDraw( sf::RenderTarget *target, 
		bool valid ) = 0;
	virtual void Draw( sf::RenderTarget *target ) = 0;
	virtual void Deactivate();


	ISelectableType selectableType;
	bool active;
};

typedef std::list<ISelectable*> SelectList;
typedef SelectList::iterator SelectIter;

struct Brush
{
	Brush();
	SelectList objects;
	void AddObject( ISelectable *obj );
	void Clear();
	void Destroy();

	bool terrainOnly;
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
	Action( Action *next = NULL );
	virtual void Perform() = 0;
	virtual void Undo() = 0;

	Action *next;
	ActionType actionType;
	bool performed;
	static EditSession *session;
};

struct ApplyBrushAction : Action
{
	ApplyBrushAction( Brush *brush );
	//TerrainPoint *pointStart;
	//TerrainPoint *pointEnd;
	//int numPoints;
	
	void Perform();
	void Undo();

	Brush *brush; //action doesn't own this
	Brush appliedBrush;
};

struct AddToPolygonAction : Action
{
	AddToPolygonAction(TerrainPolygon *brush, TerrainPolygon *poly );
	void Perform();
	void Undo();


	TerrainPolygon *oldPoly;
	TerrainPolygon *brush;
	TerrainPolygon *newPoly;
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