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
#include <map>


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
	virtual void Move( boost::shared_ptr<ISelectable> &me,
		sf::Vector2i delta ) = 0;
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
	boost::shared_ptr<GateInfo> gate;
	//GateInfo *gate;
	bool firstPoint; 
	TerrainPoint *next;
	TerrainPoint *prev;
	int GetEdgeIndex();

	bool ContainsPoint( sf::Vector2f test );
	bool Intersects( sf::IntRect rect );
	bool IsPlacementOkay();
	void Move( boost::shared_ptr<ISelectable> &me,
		sf::Vector2i delta );
	void BrushDraw( sf::RenderTarget *target, 
		bool valid );
	void Draw( sf::RenderTarget *target );
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> & select );
	void Activate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select);
	bool CanApply();
	bool CanAdd();
	void SetSelected( bool select );

	static const int POINT_RADIUS = 5;
	//int special;
};

typedef boost::shared_ptr<TerrainPoint> PointPtr;
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

typedef std::pair<TerrainPoint*,sf::Vector2<double>> Inter;
struct TerrainPolygon : ISelectable
{
	enum TerrainWorldType
	{
		MOUNTAIN,
		GLADE,
		DESERT,
		COVE,
		JUNGLE,
		FORTRESS,
		CORE,
		Count
	};

	int terrainVariation;
	TerrainWorldType terrainWorldType;

	TerrainPolygon( sf::Texture *grassTex );
	TerrainPolygon( TerrainPolygon &poly, bool pointsOnly );
	~TerrainPolygon();
	
	void CopyPoints( TerrainPoint *&start,
		TerrainPoint *&end );

	void MovePoint( sf::Vector2i &delta,
		TerrainPoint *tp );
	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	int numPoints;
	void AddPoint( TerrainPoint* tp);
	void InsertPoint( TerrainPoint *tp, TerrainPoint *prevPoint );
	void RemovePoint( TerrainPoint *tp );
	void DestroyEnemies();
	void ClearPoints();
	void SetMaterialType(
		int world, int variation );
	TerrainPoint * HasPointPos( sf::Vector2i &pos );
	//std::string material;
	
	void RemoveSelectedPoints();
	std::list<Inter> GetIntersections( TerrainPolygon *poly );
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

	void SetLayer( int p_layer );

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

	sf::Color selectCol;
	sf::Color fillCol;


	bool CanApply();
	bool CanAdd();




	bool movingPointMode;
	
	sf::Rect<int> TempAABB();

	void Move( SelectPtr &me, sf::Vector2i move );

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
	static EditSession *session;

	int layer; //0 is game layer. 1 is bg

};

typedef boost::shared_ptr<TerrainPolygon> PolyPtr;

struct GateInfo
{
	enum GateTypes
	{
		GREY,
		BLACK,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		WHITE,
		CRITICAL,
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
	bool reformBehindYou;
	int numKeysRequired;
	GateTypes type;
	
};

typedef boost::shared_ptr<GateInfo> GateInfoPtr;

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

struct GroundInfo
{
	GroundInfo();
	TerrainPoint *edgeStart;
	//TerrainPoint *edgeEnd;
	double groundQuantity;
	//not sure if this should be smart
	TerrainPolygon *ground;
	//boost::shared_ptr<TerrainPolygon> ground;
	int GetEdgeIndex();
	//int edgeIndex;
};

struct ActorGroup;
struct ActorParams : ISelectable
{
	enum PosType
	{
		GROUND_ONLY,
		AIR_ONLY,
		GROUND_AND_AIR
	};

	virtual ActorParams *Copy() = 0;
	static EditSession *session;
	ActorParams( PosType posType );
	virtual void WriteParamFile( std::ofstream &of ) = 0;
	void WriteFile( std::ofstream &of );
	void AnchorToGround( TerrainPolygon *poly, 
		int eIndex, double quantity );
	void AnchorToGround( GroundInfo &gi );
	void UnAnchor(boost::shared_ptr<ActorParams> &me );
	void UpdateGroundedSprite();
	virtual void SetPath( std::list<sf::Vector2i> &globalPath );
	void DrawBoundary( sf::RenderTarget *target );
	void DrawMonitor( sf::RenderTarget *target );
	virtual void SetParams();
	virtual void SetPanelInfo();
	static  Panel * CreateOptionsPanel();


	virtual void SetBoundingQuad();
	virtual void UpdateExtraVisuals()
	{}

	//ISelectable( ISelectableType type );
	virtual bool ContainsPoint( sf::Vector2f test );
	virtual bool Intersects( sf::IntRect rect );
	virtual bool IsPlacementOkay();
	virtual void Move( SelectPtr &me, sf::Vector2i delta );
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
	PosType posType;
	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it
	
	
	GroundInfo *groundInfo;
	bool hasMonitor;
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
	PatrollerParams( EditSession *edit,
		sf::Vector2i &pos);
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool loop;
	int speed;
};

struct BatParams : public ActorParams
{
	BatParams( EditSession *edit,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath, 
		int framesBetween,
		//int nodeDistance,
		int bulletSpeed,
		bool loop ); 
	BatParams( EditSession *edit,
		sf::Vector2i &pos );
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	int bulletSpeed;
	//int nodeDistance;
	int framesBetweenNodes;
	bool loop;
	//int speed;
};

struct PulserParams : public ActorParams
{
	PulserParams( EditSession *edit,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath, 
		int framesBetween,
		bool loop ); 
	PulserParams( EditSession *edit,
		sf::Vector2i &pos );
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	int framesBetweenNodes;
	bool loop;
	//int speed;
};

struct HealthFlyParams : public ActorParams
{
	HealthFlyParams( EditSession *edit,
		sf::Vector2i pos, int type ); 
	//HealthFlyParams( EditSession *edit );
	void WriteParamFile( std::ofstream &of );
	void Draw( sf::RenderTarget *target );
	ActorParams *Copy();
	bool CanApply();
	int color;
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
	KeyParams( EditSession *edit );
	ActorParams *Copy();
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
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity, 
		bool clockwise, float speed );
	CrawlerParams( EditSession *edit, 
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity );
	CrawlerParams( EditSession *edit );

	void SetParams();
	void SetPanelInfo();
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
	ActorParams *Copy();
	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	float speed;
};

struct StagBeetleParams : public ActorParams
{ 
	StagBeetleParams( EditSession *edit, 
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity, 
		bool clockwise, float speed );
	//StagBeetleParams( EditSession *edit,
	//	sf::Vector2i &pos, bool facingRight,
	//	float speed );
	StagBeetleParams( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity );

	
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	float speed;
};

struct CrawlerReverserParams : public ActorParams
{
	CrawlerReverserParams( 
		EditSession *edit, 
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity );
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
};

struct BossCrawlerParams : public ActorParams
{
	BossCrawlerParams( 
		EditSession *edit, 
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity );
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
};

struct PoisonFrogParams : public ActorParams
{
	PoisonFrogParams( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpWaitFrames );
	PoisonFrogParams( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity );
	PoisonFrogParams( EditSession *edit );
	void WriteParamFile( std::ofstream &of );
	void UpdatePath();
	void Draw( sf::RenderTarget *target );
	void UpdateExtraVisuals();
	//void SetParams();
	//void SetPanelInfo();
	ActorParams *Copy();
	bool CanApply();
	void SetParams(); 
	void SetPanelInfo();
	int gravFactor;
	sf::Vector2i jumpStrength;
	//int jumpStrengthX;
	//int jumpStrengthY;
	int jumpWaitFrames;
	sf::VertexArray pathQuads;
};

struct BasicTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	BasicTurretParams( EditSession *edit,  
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity, 
		double bulletSpeed, 
		int framesWait );
	BasicTurretParams( EditSession *edit,  
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity );
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	int framesWait;
};

struct CurveTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	CurveTurretParams( EditSession *edit,  
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity, 
		double bulletSpeed, 
		int framesWait,
		sf::Vector2i gravFactor,
		bool relativeGrav );
	ActorParams *Copy();
	CurveTurretParams( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity );
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile( std::ofstream &of );
	bool CanApply();
	void UpdateBulletCurve();
	void Draw( sf::RenderTarget *target );
	void UpdateExtraVisuals();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	bool relativeGrav;
	int framesWait;
	sf::Vector2i gravFactor;
	sf::VertexArray bulletPathQuads;
};

struct FootTrapParams : public ActorParams
{
	FootTrapParams( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity );
	FootTrapParams( EditSession *edit );

	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile( std::ofstream &of );
	void SetParams();
	void SetPanelInfo();
	//void Draw( sf::RenderTarget *target );
};

struct GoalParams : public ActorParams
{
	GoalParams ( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity );
	GoalParams ( EditSession *edit );
	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
};

struct PlayerParams : public ActorParams
{
	PlayerParams( EditSession *edit, 
		sf::Vector2i pos );

	bool CanApply();
	void WriteParamFile( std::ofstream &of );
	void Deactivate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select);
	ActorParams *Copy();
	void Activate( EditSession *edit,
		boost::shared_ptr<ISelectable> & select );
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
struct CompoundAction;

struct PointMoveInfo
{
	PointMoveInfo( TerrainPoint *poi )
		:point( poi ),
		 delta( 0, 0 )
	{}
	TerrainPoint *point;
	//std::list<double> enemyEdgeQuantities;
	//TerrainPolygon *poly;
	sf::Vector2i delta;
};

typedef std::map<TerrainPolygon*,
	std::list<PointMoveInfo>> PointMap;

struct EditSession : GUIHandler
{
	EditSession( sf::RenderWindow *w,
		sf::RenderTexture *preScreenTex );
	~EditSession();
	
	bool AttachActorToPolygon( ActorPtr &actor, TerrainPolygon *poly );
	void AttachActorsToPolygon( std::list<ActorPtr> &actors, TerrainPolygon *poly );
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
	void ClearSelectedPoints();
	bool PolyIntersectGate( TerrainPolygon &poly );

	GroundInfo ConvertPointToGround( sf::Vector2i point );
	void CreateActor( ActorPtr &actor );
	std::list<GateInfoPtr> gates;
	GateInfo *selectedGate;
	
	void MoveSelectedPoints( 
		//sf::Vector2i delta );
		sf::Vector2<double> worldPos );

	std::list<ActorPtr> tempActors;
	GroundInfo worldPosGround;
	ActorParams* tempActor;
	sf::Vector2i airPos;

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

	enum EnvType
	{
		MOUNTAIN,
		GLADE,
		DESERT,
		COVE,
		JUNGLE,
		FORTRESS,
		CORE
	};
	EnvType environmentType;
	int envLevel;

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
	void Add( boost::shared_ptr<TerrainPolygon> brush, 
		boost::shared_ptr<TerrainPolygon> poly);	
	void Sub( TerrainPolygon *brush,
		std::list<PolyPtr> &polys,
		std::list<PolyPtr> &results );
	
	
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
	TerrainPolygon *enemyEdgePolygon;
	double enemyEdgeQuantity;

	bool radiusOption;
	bool lightPosDown;
	double lightRadius;
	int lightBrightness;
	
	int patrolPathLengthSize;

	sf::Sprite enemySprite;
	sf::RectangleShape enemyQuad;
	ActorType *trackingEnemy;//bool trackingEnemy;
	Panel *showPanel;	
	bool trackingEnemyDown;

	Panel * CreatePopupPanel( const std::string &p );
	Panel *messagePopup;
	Panel *errorPopup;

	Panel *gateSelectorPopup;
	Panel *terrainSelectorPopup;

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
	int tempGridX;
	int tempGridY;
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




	//----------------------



	Brush *selectedBrush;
	PointMap selectedPoints;
	//std::list<TerrainPolygon*> pointPolyList;
	




	//----------------------

	CompoundAction *moveAction;

	bool moveActive;
	bool editMouseDownBox;
	bool editMouseDownMove;
	bool editStartMove;
	int editMoveThresh;
	SelectPtr grabbedObject;
	sf::Vector2i editMouseGrabPos;
	sf::Vector2i editMouseOrigPos;

	ActorPtr player;
	ActorType *playerType;
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

	int gatePoints;

	sf::VertexArray fullBounds;
	int leftBound;
	int topBound;
	int boundWidth;
	int boundHeight;
	void UpdateFullBounds();

	GateInfo testGateInfo;
	//TerrainPolygon *gatePoly0;
	//TerrainPolygon *gatePoly1;
	//TerrainPoint *gatePoint0;
	//TerrainPoint *gatePoint1;
	sf::Vector2i gatePoint0;
	sf::Vector2i gatePoint1;
	GateInfoPtr modifyGate;
	
	void PointSelectPoint(sf::Vector2<double> &worldPos,
		bool &emptysp );
	void PointSelectPolygon();
	void ExecuteTerrainCompletion();
	void ExecuteTerrainAdd(
		std::list<PolyPtr> &intersectingPolys);
	void ExecuteTerrainSubtract(
		std::list<PolyPtr> &intersectingPolys);


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
		CREATE_LIGHTS,
		CREATE_GATES
	};

	Emode mode;
	
};

#endif