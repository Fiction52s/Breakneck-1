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
	void TransferMyDuplicates(Brush *compare, Brush *dest);
	bool terrainOnly;
	bool IsEmpty();
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

struct ComplexPasteAction : Action
{
	ComplexPasteAction( Brush *mapStartBrush );
	~ComplexPasteAction();
	void SetNewest(Brush &orig, Brush &result );
	void Undo();
	void Perform();

	Brush *mapStartBrush;

	Brush orig;
	Brush applied;

	Brush mapStartOwned;
};

struct ApplyBrushAction : Action
{
	ApplyBrushAction( Brush *brush );
	~ApplyBrushAction();

	void Perform();
	void Undo();


	//Brush *brush; //action doesn't own this
	Brush appliedBrush;
};

struct RemoveBrushAction : Action
{
	RemoveBrushAction( Brush *brush, Brush *mapStartBrush );
	~RemoveBrushAction();
	void Perform();
	void Undo();

	Brush storedBrush;
	Brush mapStartOwned;
};

struct ReplaceBrushAction : Action
{
	ReplaceBrushAction( Brush *original,
		Brush *replacement, Brush *mapStartBrush );
	~ReplaceBrushAction();
	void Perform();
	void Undo();

	Brush original;
	Brush replacement;
	Brush mapStartOwned;
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

struct EditObjectAction : Action
{
	EditObjectAction();
	void Perform();
	void Undo();

	ActorParams *actor;
};

struct CreateGateAction : Action
{
	CreateGateAction( GateInfo &info, 
		const std::string &type);
	~CreateGateAction();
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