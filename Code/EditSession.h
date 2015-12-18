#ifndef __EDIT_SESSION__
#define __EDIT_SESSION__

#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <list>
#include "VectorMath.h"
#include "GUI.h"
#include "Tileset.h"
#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>


struct EditSession;
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
	virtual bool ContainsPoint( sf::Vector2f test ) = 0;
	virtual bool Intersects( sf::IntRect rect ) = 0;
	virtual bool IsPlacementOkay() = 0;
	virtual void Move( sf::Vector2i delta ) = 0;
	virtual void BrushDraw( sf::RenderTarget *target, 
		bool valid ) = 0;
	virtual void Draw( sf::RenderTarget *target ) = 0;
	virtual void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> & select ) = 0;
	virtual void Activate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select) = 0;
	virtual bool CanApply() = 0;
	virtual bool CanAdd() = 0;
	virtual void SetSelected( bool select ) = 0;
	//virtual bool CanSubtract() = 0;

	ISelectableType selectableType;
	bool active;
	bool selected;
};

typedef boost::shared_ptr<ISelectable> SelectPtr;
typedef std::list<SelectPtr> SelectList;
typedef SelectList::iterator SelectIter;



struct ActorParams;

struct GrassSeg
{
	GrassSeg( int edgeI, int grassIndex, int rep )
		:edgeIndex( edgeI ), index( grassIndex ), 
		reps (rep)
	{}
	int edgeIndex;
	int index;
	int reps;
};

struct TerrainPolygon;
struct GateInfo;

struct GatePoint
{
	//boost::shared_ptr<TerrainPolygon> poly;
	int vertexIndex;
	GateInfo *info;
};

struct TerrainPoint
{
	TerrainPoint( sf::Vector2i &pos, bool selected );
	~TerrainPoint()
	{
		//delete gate;
	}
	sf::Vector2i pos;
	bool selected;
	std::list<int> grass;
	GateInfo *gate;
	bool firstPoint; 
	TerrainPoint *next;
	TerrainPoint *prev;
	int GetEdgeIndex();

	//int special;
};

typedef std::pair<sf::Vector2i,sf::Vector2i> PointPair;

struct EditSession;

struct TerrainBrush
{
	TerrainBrush( boost::shared_ptr<TerrainPolygon> poly );
	TerrainBrush( TerrainBrush &brush );
	~TerrainBrush();
	void AddPoint( TerrainPoint* tp);
	void UpdateLines();
	void Move( sf::Vector2i delta );
	void Draw( sf::RenderTarget *target );
	int numPoints;
	sf::VertexArray lines;
	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	int left;
	int right;
	int top;
	int bot;
	
};





struct TerrainPolygon : ISelectable
{
	TerrainPolygon( sf::Texture *grassTex );
	TerrainPolygon( TerrainPolygon &poly, bool pointsOnly );
	~TerrainPolygon();
	
	void CopyPoints( TerrainPoint *&start,
		TerrainPoint *&end );

	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	int numPoints;
	void AddPoint( TerrainPoint* tp);
	void RemovePoint( TerrainPoint *tp );
	void DestroyEnemies();
	void ClearPoints();
	std::string material;
	void RemoveSelectedPoints();
	bool IsRemovePointsOkayTerrain(EditSession *edit);
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	void Reset();
	void SoftReset();
	void Draw( bool showPath, double zoomMultiple, sf::RenderTarget * rt, bool showPoints, TerrainPoint *dontShow );
	void FixWinding();
	bool IsClockwise();
	void AlignExtremes( double primLimit );
	void UpdateGrass();
	
	void ShowGrass( bool show );
	void Extend( TerrainPoint* startPoint, TerrainPoint*endPoint, boost::shared_ptr<TerrainPolygon> inProgress );
	void SwitchGrass( sf::Vector2<double> mousePos );
	//bool ContainsPoint( sf::Vector2f p );
	void SetSelected( bool select );
	
	bool IsMovePointsOkay( EditSession *edit,
		sf::Vector2i delta );
	bool IsMovePointsOkay( EditSession *edit,
		sf::Vector2i pointGrabDelta,
		sf::Vector2i *deltas );
	bool IsMovePolygonOkay( EditSession *edit, 
		sf::Vector2i delta );
	void MoveSelectedPoints(sf::Vector2i move);
	void UpdateBounds();

	bool ContainsPoint( sf::Vector2f point );
	bool Intersects( sf::IntRect rect );
	bool IsPlacementOkay();
	//void Move( sf::Vector2i delta );
	void BrushDraw( sf::RenderTarget *target, 
		bool valid );
	void Draw( sf::RenderTarget *target );
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> & select);
	void Activate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select);
	
	bool IsTouching( TerrainPolygon *poly );
	bool Contains( TerrainPolygon *poly );
	//bool IsTouching( TerrainPolygon * p );
	bool BoundsOverlap( TerrainPolygon *poly );
	bool LinesIntersect( TerrainPolygon *poly );
	bool PointTooCloseToPoints( sf::Vector2i point,
		int minDistance );
	bool PointTooClose( sf::Vector2i point,
		int minDistance );
	bool LinesTooClose( TerrainPolygon *poly,
		int minDistance );
	bool PointTooCloseToLines( sf::Vector2i point,
		int minDistance );
	bool SegmentTooClose( sf::Vector2i a,
		sf::Vector2i b, int minDistance );
	bool SegmentWithinDistanceOfPoint(
		sf::Vector2i startSeg,
		sf::Vector2i endSeg,
		sf::Vector2i testPoint,
		int distance );
	bool TooClose( TerrainPolygon *poly,
		bool intersectAllowed,
		int minDistance );




	bool CanApply();
	bool CanAdd();




	bool movingPointMode;
	
	sf::Rect<int> TempAABB();

	void Move( sf::Vector2i move );

	sf::Vertex *lines;
	sf::VertexArray *va;
	sf::VertexArray *grassVA;
	int numGrassTotal;
	sf::Texture *grassTex;
	int vaSize;
	//bool selected;
	int left;
	int right;
	int top;
	int bottom;
	std::list<sf::Vector2i> path;
	
	//enemymap
	std::map<TerrainPoint*,std::list<
		boost::shared_ptr<ActorParams>>> enemies;
	int writeIndex;
	bool isGrassShowing;
	bool finalized;

};



typedef boost::shared_ptr<TerrainPolygon> PolyPtr;


struct GateInfo
{
	enum GateTypes
	{
		RED,
		GREEN,
		BLUE,
		Count
	};

	GateInfo();
	void SetType( const std::string &gType );
	TerrainPoint *point0;
	TerrainPoint *point1;
	boost::shared_ptr<TerrainPolygon> poly0;
	int vertexIndex0;
	boost::shared_ptr<TerrainPolygon> poly1;
	int vertexIndex1;
	sf::VertexArray thickLine;
	EditSession *edit;
	void UpdateLine();
	void WriteFile( std::ofstream &of );
	void Draw( sf::RenderTarget *target );

	GateTypes type;
	
};


struct StaticLight
{
	StaticLight( sf::Color c, sf::Vector2i &pos, int radius, int brightness );
	void Draw( sf::RenderTarget *target );
	sf::Rect<double> GetAABB();
	int radius;
	int brightness;
	sf::Color color;
	sf::Vector2i position;
	void WriteFile( std::ofstream &of );
};



struct ActorType
{
	ActorType( const std::string & name, Panel *panel );
	void Init();
	std::string name;
	sf::Texture iconTexture;
	sf::Texture imageTexture;
	int width;
	int height;
	bool canBeGrounded;
	bool canBeAerial;
	Panel *panel;
};

struct ActorGroup;
struct ActorParams : ISelectable
{
	ActorParams();
	virtual void WriteParamFile( std::ofstream &of ) = 0;
	void WriteFile( std::ofstream &of );
	void AnchorToGround( 
		boost::shared_ptr<TerrainPolygon> poly, 
		int eIndex, double quantity );
	void UpdateGroundedSprite();
	virtual void SetBoundingQuad();

	//ISelectable( ISelectableType type );
	virtual bool ContainsPoint( sf::Vector2f test );
	virtual bool Intersects( sf::IntRect rect );
	virtual bool IsPlacementOkay();
	virtual void Move( sf::Vector2i delta );
	virtual void BrushDraw( sf::RenderTarget *target, 
		bool valid );
	virtual void Draw( sf::RenderTarget *target );
	virtual void Deactivate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select);
	virtual void Activate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select );

	virtual void DrawQuad( sf::RenderTarget *target );

	virtual void SetSelected( bool select );

	virtual bool CanApply() = 0;
	bool CanAdd();

	//sf::Sprite icon;
	sf::Sprite image;
	ActorGroup *group;
	ActorType *type;
	sf::Vector2i position;

	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it
	struct GroundInfo
	{
		TerrainPoint *edgeStart;
		//TerrainPoint *edgeEnd;
		double groundQuantity;
		boost::shared_ptr<TerrainPolygon> ground;
		int GetEdgeIndex();
		//int edgeIndex;
	};
	
	GroundInfo *groundInfo;

	sf::VertexArray boundingQuad;
};

typedef boost::shared_ptr<ActorParams> ActorPtr;
typedef std::map<TerrainPoint*,std::list<ActorPtr>> EnemyMap;


struct PatrollerParams : public ActorParams
{
	PatrollerParams( EditSession *edit,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath, 
		float speed,
		bool loop ); 
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );

	bool CanApply();

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool loop;
	float speed;
};

struct KeyParams : public ActorParams
{
	KeyParams( EditSession *edit,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath, 
		float speed,
		bool loop,
		int stayFrames,
		bool teleport,
		GateInfo::GateTypes gType );
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );
	bool CanApply();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos
	bool loop;
	float speed;
	int stayFrames;
	bool teleport;
	GateInfo::GateTypes gateType;
};

struct CrawlerParams : public ActorParams
{ 
	CrawlerParams( EditSession *edit, 
		boost::shared_ptr<TerrainPolygon> edgePolygon,
		int edgeIndex, double edgeQuantity, 
		bool clockwise, float speed );
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	float speed;
};

struct BasicTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	BasicTurretParams( EditSession *edit,  
		boost::shared_ptr<TerrainPolygon> edgePolygon,
		int edgeIndex, 
		double edgeQuantity, 
		double bulletSpeed, 
		int framesWait );
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	int framesWait;
};

struct FootTrapParams : public ActorParams
{
	FootTrapParams( EditSession *edit,
		boost::shared_ptr<TerrainPolygon> edgePolygon,
		int edgeIndex, 
		double edgeQuantity );
	bool CanApply();
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
};

struct GoalParams : public ActorParams
{
	GoalParams ( EditSession *edit,
		boost::shared_ptr<TerrainPolygon> edgePolygon,
		int edgeIndex, 
		double edgeQuantity );
	bool CanApply();
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
};

//no params for goal and foottrap atm
struct ActorGroup
{
	ActorGroup( const std::string &name );
	std::string name;
	std::list<ActorPtr> actors;
	void Draw( sf::RenderTarget *target );
	void WriteFile( std::ofstream &of );
};

struct Brush;
struct Action;

struct EditSession : GUIHandler
{
	EditSession( sf::RenderWindow *w,
		sf::RenderTexture *preScreenTex );
	~EditSession();
		
	int Run(std::string fileName, 
		sf::Vector2f cameraPos, 
		sf::Vector2f cameraSize );
	void Draw();
	bool OpenFile( std::string fileName );
	void WriteFile(std::string fileName);
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );
	bool IsExtendPointOkay( boost::shared_ptr<TerrainPolygon> poly,
		sf::Vector2f testPoint );
	bool IsPointValid( sf::Vector2i oldPoint, sf::Vector2i point, TerrainPolygon * poly );
	void ExtendAdd();
	bool IsPolygonExternallyValid( TerrainPolygon &poly,
		 TerrainPolygon* ignore );
	bool IsPolygonInternallyValid( TerrainPolygon &poly );
	bool IsPolygonValid( TerrainPolygon &poly,
		TerrainPolygon* ignore );
	sf::Vector2<double> GraphPos( sf::Vector2<double> realPos );
	void SetEnemyEditPanel();
	bool QuadPolygonIntersect( TerrainPolygon* poly, 
		sf::Vector2i a, sf::Vector2i b, 
		sf::Vector2i c, sf::Vector2i d );
	bool CanCreateGate( GateInfo &testGate );
	void SetPanelDefault( ActorType *type );
	
	bool PolyIntersectGate( TerrainPolygon &poly );
	std::list<GateInfo*> gates;
	GateInfo *selectedGate;


	const static double PRIMARY_LIMIT;
	sf::RenderTexture *preScreenTex;
	int validityRadius;
	bool showGrass;
	sf::Texture grassTex;
	bool pointGrab;
	sf::Vector2i pointGrabPos;
	sf::Vector2i pointGrabDelta;
	bool polyGrab;
	//sf::Vector2i polyGrabPos;
	//sf::Vector2f polyGrabPos;
	sf::Vector2i polyGrabPos;
	sf::Vector2i polyGrabDelta;
	sf::Vector2f polyMove;

	bool makingRect;
	sf::Vector2i rectStart;



	void ExtendPolygon();
	bool showPoints;
	boost::shared_ptr<TerrainPolygon> extendingPolygon;
	TerrainPoint *extendingPoint;

	sf::View v;


	bool showTerrainPath;
	
	sf::RenderWindow *w;
	sf::Vector2i playerPosition;
	//sf::Vector2i goalPosition;
	std::string currentFile;
	static double zoomMultiple;
	sf::Vector2f testPoint;
	std::map<std::string, ActorGroup*> groups;
	std::map<std::string, ActorType*> types;
	ActorParams *selectedActor;
	ActorParams *editActor;

	StaticLight *selectedLight;
	bool selectedLightGrabbed;
	sf::Vector2i lightGrabPos;

	bool selectedPlayer;
	bool grabPlayer;
	int playerHalfWidth;
	int playerHalfHeight;
	sf::Vector2i grabPos;
	bool selectedActorGrabbed;

	//CREATE_TERRAIN mode
	void Add( boost::shared_ptr<TerrainPolygon> brush, boost::shared_ptr<TerrainPolygon> poly);	
	bool PointValid( sf::Vector2i prev, sf::Vector2i point );
	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	static LineIntersection LimitSegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );

	double minimumEdgeLength;
	double minAngle;
	
	std::list<boost::shared_ptr<TerrainPolygon>> polygons;
	std::list<boost::shared_ptr<TerrainPolygon>> selectedPolygons;
	//std::list<boost::shared_ptr<TerrainPolygon>> polygons;
	boost::shared_ptr<TerrainPolygon> polygonInProgress;
	std::list<sf::VertexArray*> progressDrawList;
	
	//sf::Text polygonTimeoutText;
	//int polygonTimeoutTextTimer;
	//int polygonTimeoutTextLength;

	//static void TestButton();
	std::list<StaticLight*> lights;
	sf::Vector2i lightPos;
	bool lightActive;

	int enemyEdgeIndex;
	boost::shared_ptr<TerrainPolygon> enemyEdgePolygon;
	double enemyEdgeQuantity;

	bool radiusOption;
	bool lightPosDown;
	double lightRadius;
	int lightBrightness;
	

	sf::Sprite enemySprite;
	sf::RectangleShape enemyQuad;
	ActorType *trackingEnemy;//bool trackingEnemy;
	Panel *showPanel;	
	bool trackingEnemyDown;

	Panel * CreatePopupPanel( const std::string &p );
	Panel *messagePopup;
	Panel *errorPopup;

	Panel *gateSelectorPopup;

	Panel *enemySelectPanel;

	int IsRemovePointsOkay();

	Panel *CreateOptionsPanel( const std::string &name );
	void WriteGrass( boost::shared_ptr<TerrainPolygon>  p, std::ofstream &of );
	int CountSelectedPoints();

	std::list<sf::Vector2i> patrolPath;
	double minimumPathEdgeLength;

	sf::IntRect fullRect;
	
	bool ConfirmationPop( const std::string &question );
	void GridSelectPop( const std::string &type );
	std::string tempGridResult;
	void MessagePop( const std::string &message );
	void ErrorPop( const std::string &error );

	//bool closePopup; //for messsage/error only
	
	//new stuff
	Brush *progressBrush;
	std::list<Action*> doneActionStack;
	std::list<Action*> undoneActionStack;
	void ClearUndoneActions();

	sf::Rect<float> selectRect;
	sf::Vector2i pointMouseDown;

	Brush *selectedBrush;

	bool moveActive;
	bool editMouseDownBox;
	bool editMouseDownMove;
	bool editStartMove;
	int editMoveThresh;
	SelectPtr grabbedObject;
	sf::Vector2i editMouseGrabPos;
	sf::Vector2i editMouseOrigPos;


	//std::list<Action*>::iterator currAction;

	enum ConfirmChoices
	{
		NONE,
		CONFIRM,
		CANCEL
	};
	ConfirmChoices confirmChoice; 

	//0 for no choice
	//1 for confirm
	//2 for cancel

	Panel *confirm;
	sf::View uiView;
	sf::View view;
	//void GoPopup();

	std::list<TerrainBrush*> copyBrushes;
	std::list<TerrainBrush*> pasteBrushes;
	void ClearCopyBrushes();
	void ClearPasteBrushes();
	void CopyToPasteBrushes();
	sf::Vector2i pastePos;

	

	enum Emode
	{
		CREATE_TERRAIN,
		EDIT,
		SELECT_MODE,
		CREATE_PATROL_PATH,
		//PLACE_PLAYER,
		//PLACE_GOAL,
		SELECT_POLYGONS,
		PAUSED,
		CREATE_ENEMY,
		DRAW_PATROL_PATH,
		CREATE_TERRAIN_PATH,
		CREATE_LIGHTS
	};

	Emode mode;
	
};



#endif