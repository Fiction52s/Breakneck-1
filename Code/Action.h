#ifndef __ACTION_H__
#define __ACTION_H__

#include <list>
#include "EditSession.h"

struct CompoundAction;
struct Brush
{
	Brush();
	SelectList objects;
	void AddObject( SelectPtr obj );
	void RemoveObject( SelectPtr obj );
	void Clear();
	void Destroy();
	void Move( sf::Vector2i delta );
	void Draw( sf::RenderTarget *target );
	Brush *Copy();
	sf::Vector2i &GetCenter();
	void CenterOnPoint(sf::Vector2i &point );
	sf::Vector2i center;
	CompoundAction * UnAnchor(); //only works with grounded actors
	void Deactivate();//multiple calls covered?
	void Activate();//multiple calls covered?
	void SetSelected( bool select );
	bool CanApply();
	bool CanAdd();
	bool Has(SelectPtr sp);
	bool terrainOnly;
	//bool activated;
};

struct Action
{
	enum ActionType
	{
		APPLY_BRUSH,
		REMOVE_BRUSH,
		REPLACE_BRUSH,
		DESTROY_ACTOR,
		MOVE_BRUSH,
		MOVE_POINTS,
		DELETE_POINTS,
		EDIT_OBJECT,
		CREATE_GATE,
		DELETE_GATE,
		COMPOUND,
		Count
	};
	//Action( ActionType actionType, Action *next = NULL );
	Action();
	virtual ~Action();
	virtual void Perform() = 0;
	virtual void Undo() = 0;

	//Action *next;
	//Action *prev;
	ActionType actionType;
	bool performed;
};

struct CompoundAction : Action
{
	CompoundAction();
	~CompoundAction();
	std::list<Action*> subActions;
	void Perform();
	void AddSubAction(Action *);
	void Undo();
};

struct ReplaceBrushAction;
struct ComplexPasteAction : Action
{
	ComplexPasteAction();
	~ComplexPasteAction();
	void SetNewest(ReplaceBrushAction *a);
	void Undo();
	void Perform();

	Brush orig;
	Brush applied;
};

struct ApplyBrushAction : Action
{
	ApplyBrushAction( Brush *brush );
	
	void Perform();
	void Undo();


	//Brush *brush; //action doesn't own this
	Brush appliedBrush;
};

struct RemoveBrushAction : Action
{
	RemoveBrushAction( Brush *brush );

	void Perform();
	void Undo();

	Brush storedBrush;
};

struct ReplaceBrushAction : Action
{
	ReplaceBrushAction( Brush *original,
		Brush *replacement );

	void Perform();
	void Undo();

	Brush original;
	Brush replacement;
};

struct AddBrushAction : Action
{
	AddBrushAction(Brush *brush, Brush *intersectingPolys );
	void Perform();
	void Undo();


	Brush *oldPolys;
	Brush *brush;
	Brush *intersectingPolys;
	Brush newPolys;
};

struct AddToPolygonAction : Action
{
	AddToPolygonAction();
	void Perform();
	void Undo();
};

//--NOTE: DOESNT ACTUALLY MOVE BRUSH
//IF PARAMETER IS SET



struct MoveBrushAction : Action
{
	MoveBrushAction( Brush *brush, sf::Vector2i delta,
		bool moveOnFirstPerform,
		PointVectorMap &points, RailPointMap &railPoints);
	void Perform();
	void Undo();
	void CheckValidPointMove();

	bool moveValid;
	Brush movingBrush;
	//std::list<PointMoveInfo> movingPoints;
	PointVectorMap movingPoints;
	RailPointMap movingRailPoints;
	bool moveOnFirstPerform;
	sf::Vector2i delta;
};

struct LeaveGroundAction : Action
{
	LeaveGroundAction( ActorPtr actor );
		//sf::Vector2i newPos );

	ActorPtr actor;
	GroundInfo gi;
	void Perform();
	void Undo();
};

struct GroundAction : Action
{
	GroundAction( ActorPtr actor );
	void Perform();
	void Undo();

	ActorPtr actor;
	GroundInfo gi;
	//sf::Vector2i oldPos;
};

struct DeleteActorAction : Action
{
	DeleteActorAction();
	void Perform();
	void Undo();
	
	ActorParams *actor;
};

struct EditObjectAction : Action
{
	EditObjectAction();
	void Perform();
	void Undo();

	ActorParams *actor;
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

	PolyPtr oldPolygon; //copy
	std::list<TerrainPoint*> points;
};

struct CreateGateAction : Action
{
	CreateGateAction( GateInfo &info, 
		const std::string &type);
	void Perform();
	void Undo();

	GateInfoPtr gate;
};

struct DeleteGateAction : Action
{
	DeleteGateAction( GateInfoPtr ptr );
	void Perform();
	void Undo();

	GateInfoPtr gate;
};

struct ModifyGateAction : Action
{
	ModifyGateAction( GateInfoPtr ptr, const
		std::string &type );
	void Perform();
	void Undo();

	GateInfoPtr gate;
	Gate::GateType oldType;
	std::string newType;
};

struct ModifyTerrainTypeAction : Action
{
	ModifyTerrainTypeAction( Brush *brush,
		int newTerrainWorld,
		int newVariation );
	void Perform();
	void Undo();

	int newTerrainWorld;
	int newVariation;

	std::map<PolyPtr,std::pair<int,int>> 
		terrainTypeMap;
	Brush terrainBrush; 
	//also has other stuff
	//in it possibly
	
};



#endif