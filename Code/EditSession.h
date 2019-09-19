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
#include <boost/filesystem.hpp>
#include "MainMenu.h"
#include "MapHeader.h"
#include "ShardTypes.h"
#include "Gate.h"
#include "ISelectable.h"

struct ActorParams;
struct MainMenu;
//struct ActorParams;
struct TerrainPolygon;
struct GateInfo;
struct EditSession;
struct ActorGroup;
struct ActorType;
struct Brush;
struct Action;
struct CompoundAction;
struct KinSkin;
struct Background;
struct ScrollingBackground;
struct AirTriggerParams;


struct ActorType;
typedef ActorParams* ParamsMaker(ActorType*);
typedef ActorParams* ParamsLoader(ActorType*,
	std::ifstream &is);

template<typename X> ActorParams *MakeParamsGrounded(
	ActorType *);
template<typename X> ActorParams *MakeParamsAerial(
	ActorType *);
template<typename X> ActorParams *LoadParams(
	ActorType *,std::ifstream &isa);

struct ParamsInfo
{
	ParamsInfo(const std::string &n,
		ParamsLoader *p_pLoader,
		ParamsMaker *pg, ParamsMaker *pa,
		sf::Vector2i &off, sf::Vector2i &p_size,
		bool w_monitor, bool w_level,
		bool w_path, bool w_loop,
		Tileset *p_ts = NULL, int imageTile = 0)
		:name(n), pLoader( p_pLoader ),
		pmGround(pg), pmAir(pa),
		offset(off), size(p_size),
		ts(p_ts), imageTileIndex(imageTile),
		writeMonitor( w_monitor ), writeLevel( w_level ),
		writePath( w_path ), writeLoop ( w_loop )

	{

	}
	std::string name;
	ParamsLoader *pLoader;
	ParamsMaker* pmGround;
	ParamsMaker* pmAir;
	sf::Vector2i offset;
	sf::Vector2i size;
	Tileset *ts;
	int imageTileIndex;

	bool writeMonitor;
	bool writeLevel;
	bool writePath;
	bool writeLoop;
};

struct GrassSeg
{
	GrassSeg( int edgeI, int grassIndex, int rep )
		:edgeIndex( edgeI ), index( grassIndex), 
		reps (rep)
	{}
	int edgeIndex;
	int index;
	int reps;
};

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

	bool ContainsPoint( sf::Vector2f test );
	bool Intersects( sf::IntRect rect );
	bool IsPlacementOkay();
	void Move( boost::shared_ptr<ISelectable> &me,
		sf::Vector2i delta );
	void BrushDraw( sf::RenderTarget *target, 
		bool valid );
	void Draw( sf::RenderTarget *target );
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select );
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	bool CanApply();
	bool CanAdd();
	void SetSelected( bool select );

	static const int POINT_RADIUS = 5;
	//int special;
};

typedef boost::shared_ptr<TerrainPoint> PointPtr;
typedef std::pair<sf::Vector2i,sf::Vector2i> PointPair;

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

struct Inter
{
	Inter()
		:point( NULL )
	{

	}
	Inter(TerrainPoint *p_point, sf::Vector2<double> &p_pos)
		:point( p_point ), position( p_pos )
	{

	}
	TerrainPoint *point;
	sf::Vector2<double> position;
};

struct DetailedInter
{
	DetailedInter()
		:otherPoint(NULL)
	{
		inter.point = NULL;
	}
	DetailedInter(TerrainPoint *p_point, sf::Vector2<double> &p_pos, TerrainPoint *p_otherPoint)
		:inter( p_point, p_pos ), otherPoint( p_otherPoint )
	{

	}
	Inter inter;
	TerrainPoint *otherPoint;
};

struct TerrainRender;
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

	//TerrainRender *tr;
	int terrainVariation;
	TerrainWorldType terrainWorldType;
	int GetNumGrassTotal();
	int GetNumGrass(TerrainPoint *tp, bool &rem);
	void SetupGrass(TerrainPoint *tp, int &i);
	void SetupGrass();
	sf::Shader *pShader;

	void UpdateLines();
	TerrainPolygon( sf::Texture *grassTex );
	TerrainPolygon( TerrainPolygon &poly, bool pointsOnly );
	~TerrainPolygon();
	void UpdateLineColor( sf::Vertex *line, TerrainPoint *p, int index );
	bool SwitchPolygon( bool cw, TerrainPoint *rootPoint,
		TerrainPoint *switchStart);
	void CopyPoints( TerrainPoint *&start,
		TerrainPoint *&end );
	sf::Vector2i TrimSliverPos(sf::Vector2<double> &prevPos,
		sf::Vector2<double> &pos, sf::Vector2<double> &nextPos,
		double minAngle,bool cw);
	void CopyPoints(TerrainPolygon *poly);
	TerrainPolygon *Copy();
	bool PointOnBorder(V2d &point);
	void MovePoint( sf::Vector2i &delta,
		TerrainPoint *tp );
	
	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	bool IsPoint(sf::Vector2i &p);
	TerrainPoint *GetSamePoint(sf::Vector2i &p);
	int numPoints;
	void AddPoint( TerrainPoint* tp);
	void InsertPoint( TerrainPoint *tp, TerrainPoint *prevPoint );
	void RemovePoint( TerrainPoint *tp );
	void DestroyEnemies();
	void ClearPoints();
	TerrainPoint *GetPointAtIndex(int index);
	int GetPointIndex( TerrainPoint *p);
	void SetMaterialType(
		int world, int variation );
	void RemoveSlivers( double minAngle );
	int GetIntersectionNumber(sf::Vector2i &a, sf::Vector2i &b, 
		Inter &inter, TerrainPoint *&outSegStart, bool &outFirstPoint );
	TerrainPoint *GetMostLeftPoint();
	bool SharesPoints(TerrainPolygon *poly);
	TerrainPoint * HasPointPos( sf::Vector2i &pos );
	LineIntersection GetSegmentFirstIntersection(sf::Vector2i &a, sf::Vector2i &b,
		TerrainPoint *&outSegStart, TerrainPoint *&outSegEnd,
		bool ignoreStartPoint = false);
	//std::string material;
	void RemoveSelectedPoints();
	void GetIntersections( TerrainPolygon *poly, std::list<Inter> &outInters);
	void GetDetailedIntersections(TerrainPolygon *poly, std::list<DetailedInter> &outInters);
	bool IsRemovePointsOkayTerrain(EditSession *edit);
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	void FinalizeInverse();
	void Reset();
	void SoftReset();
	void Draw( bool showPath, double zoomMultiple, sf::RenderTarget * rt, bool showPoints, TerrainPoint *dontShow );
	void FixWinding();
	void FixWindingInverse();
	bool IsClockwise();
	void AlignExtremes( double primLimit );
	void UpdateGrass();

	int grassSize;
	int grassSpacing;
	
	
	void ShowGrass( bool show );
	void Extend2( TerrainPoint* startPoint, TerrainPoint*endPoint, boost::shared_ptr<TerrainPolygon> inProgress );
	void Cut2( TerrainPoint* startPoint, TerrainPoint*endPoint, boost::shared_ptr<TerrainPolygon> inProgress );
	
	void SwitchGrass( sf::Vector2<double> mousePos );
	//bool ContainsPoint( sf::Vector2f p );
	void SetSelected( bool select );
	
	bool IsMovePointsOkay( 
		sf::Vector2i delta );
	bool IsMovePointsOkay( 
		sf::Vector2i pointGrabDelta,
		sf::Vector2i *deltas );
	bool IsMovePolygonOkay(  
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
		boost::shared_ptr<ISelectable> select);
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	
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
	TerrainPoint *GetClosePoint(double radius, V2d &pos);

	sf::Color selectCol;
	sf::Color fillCol;

	
	bool CanApply();
	bool CanAdd();

	


	bool movingPointMode;
	
	sf::Rect<int> TempAABB();

	void Move( SelectPtr me, sf::Vector2i move );

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

	int layer; //0 is game layer. 1 is bg

	bool inverse;
};

typedef boost::shared_ptr<TerrainPolygon> PolyPtr;

struct GateInfo : ISelectable
{
	GateInfo();
	void SetType( const std::string &gType );
	void SetShard(int shardW, int shardI);
	TerrainPoint *point0;
	TerrainPoint *point1;
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	boost::shared_ptr<TerrainPolygon> poly0;
	int vertexIndex0;
	boost::shared_ptr<TerrainPolygon> poly1;
	int vertexIndex1;
	sf::VertexArray thickLine;
	EditSession *edit;
	void UpdateLine();
	void WriteFile( std::ofstream &of );
	void Draw( sf::RenderTarget *target );
	void DrawPreview(sf::RenderTarget *target);
	int numKeysRequired;
	sf::Color color;
	Gate::GateType type;

	int shardWorld;
	int shardIndex;
	sf::Sprite shardSpr;
	sf::RectangleShape shardBG;
	
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
	V2d GetPosition();
	//int edgeIndex;
};



typedef boost::shared_ptr<ActorParams> ActorPtr;
typedef std::map<TerrainPoint*,std::list<ActorPtr>> EnemyMap;

struct ActorType
{
	ActorType( ParamsInfo &pi);
	void Init();
	void PlaceEnemy();
	void PlaceEnemy(ActorParams *ap);
	void LoadEnemy(std::ifstream &is,
		ActorPtr &a);
	Panel * CreatePanel();
	bool IsGoalType();
	sf::Sprite GetSprite(int xSize = 0, int ySize = 0);
	sf::Sprite GetSprite(bool grounded);
	bool CanBeGrounded();
	bool CanBeAerial();

	Panel *panel;

	

	ParamsInfo info;
};



//no params for goal and foottrap atm
struct ActorGroup
{
	ActorGroup( const std::string &name );
	std::string name;
	std::list<ActorPtr> actors;
	void Draw( sf::RenderTarget *target );
	void WriteFile( std::ofstream &of );
	void DrawPreview( sf::RenderTarget *target );
};



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

typedef std::map<TerrainPolygon*,std::list<PointMoveInfo>> PointMap;




struct EditSession : GUIHandler, TilesetManager
{
	struct DecorInfo : ISelectable
	{
		DecorInfo(sf::Sprite &s, int lay,
			const std::string &dName, int p_tile)
			:ISelectable(ISelectable::ISelectableType::IMAGE),
			spr(s), layer(lay),
			decorName(dName), tile(p_tile) {}

		bool ContainsPoint(sf::Vector2f test);
		bool Intersects(sf::IntRect rect);
		void Move(boost::shared_ptr<ISelectable> me,
			sf::Vector2i delta);
		void BrushDraw(sf::RenderTarget *target,
			bool valid);
		void Deactivate(EditSession *edit,
			boost::shared_ptr<ISelectable> select);
		void Activate(EditSession *edit,
			boost::shared_ptr<ISelectable> select);
		void SetSelected(bool select);
		void WriteFile(std::ofstream &of);
		void Draw(sf::RenderTarget *target);
		sf::Sprite spr;
		int layer;
		std::string decorName;
		int tile;
		std::list<boost::shared_ptr<DecorInfo>> *myList;
	};

	enum Tool
	{
		TOOL_ADD,
		TOOL_SUBTRACT,
		TOOL_EXTEND,
		TOOL_CUT
	};

	enum ImageEditTool
	{
		ITOOL_EDIT,
		ITOOL_ROTATE,
		ITOOL_SCALE
	};

	TerrainPolygon *GetPolygon(int index, int &edgeIndex);
	
	

	std::list<ParamsInfo> worldEnemyNames[8];
	void AddWorldEnemy(const std::string &name, int w,
		ParamsLoader *pLoader,
		ParamsMaker* pmGround, ParamsMaker *pmAir,
		sf::Vector2i &off, 
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddExtraEnemy(const std::string &name,
		ParamsLoader *pLoader,
		ParamsMaker *pmGround, ParamsMaker *pmAir,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		Tileset *ts = NULL,
		int tileIndex = 0);
	std::list<ParamsInfo> extraEnemyNames;

	void SetupEnemyTypes();
	void SetupEnemyType(ParamsInfo &pi);

	bool IsKeyPressed(int k);
	std::list<Panel*> allPopups;
	static EditSession *GetSession();
	static EditSession *currSession;
	Background *currBackground;
	std::list<ScrollingBackground*> scrollingBackgrounds;

	bool showBG;
	const static int MAX_TERRAINTEX_PER_WORLD = 10;
	sf::Texture *terrainTextures[9 * MAX_TERRAINTEX_PER_WORLD];
	sf::Shader polyShaders[9 * MAX_TERRAINTEX_PER_WORLD];

	void DrawBG(sf::RenderTarget *target);

	int bossType;

	Tool currTool;
	ImageEditTool currImageTool;

	Tileset *ts_shards[7];
	
	//need to clean this up
	int grassSize;
	int grassSpacing;

	EditSession( MainMenu *p_mainMenu );
	~EditSession();
	
	ActorParams * AttachActorToPolygon( ActorPtr actor, TerrainPolygon *poly );
	void AttachActorsToPolygon( std::list<ActorPtr> &actors, TerrainPolygon *poly );
	int Run(const boost::filesystem::path &p_filePath,
		sf::Vector2f cameraPos, 
		sf::Vector2f cameraSize );
	void Draw();
	sf::Vector2f SnapPointToGraph(sf::Vector2f &p, int gridSize);
	V2d SnapPointToGraph(V2d &p, int gridSize);
	sf::Vector2f SnapPosToPoint(sf::Vector2f &p, double radius);
	static bool PointOnLine(V2d &pos, V2d &p0, V2d &p1, double width = 0);
	void TryPlaceGatePoint(V2d &pos);
	bool OpenFile();
	void WriteFile(std::string fileName);
	void RegularOKButton();
	void RegularCreatePathButton();
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );
	bool IsExtendPointOkay( boost::shared_ptr<TerrainPolygon> poly,
		sf::Vector2f testPoint );
	//bool IsPointValid( sf::Vector2i oldPoint, sf::Vector2i point, TerrainPolygon * poly );
	void ExtendAdd();
	bool IsPolygonExternallyValid( TerrainPolygon &poly,
		 TerrainPolygon* ignore );
	bool IsPolygonInternallyValid( TerrainPolygon &poly );
	bool IsPolygonValid( TerrainPolygon &poly,
		TerrainPolygon* ignore );
	void SetEnemyEditPanel();
	void SetDecorEditPanel();
	void SetDecorParams();
	bool QuadPolygonIntersect( TerrainPolygon* poly, 
		sf::Vector2i a, sf::Vector2i b, 
		sf::Vector2i c, sf::Vector2i d );
	bool CanCreateGate( GateInfo &testGate );
	void SetPanelDefault( ActorType *type );
	void ClearSelectedPoints();
	void SelectPoint(TerrainPolygon *poly,
		TerrainPoint *point);
	void DeselectPoint(TerrainPolygon *poly,
		TerrainPoint *point);
	bool PolyIntersectGate( TerrainPolygon &poly );


	void SetInversePoly();
	sf::Sprite scaleSprite;

	int drainSeconds;

	static void s_CreatePreview( EditSession *session, 
		sf::Vector2i imageSize );
	void CreatePreview( sf::Vector2i imageSize );
	sf::RenderTexture *mapPreviewTex;

	GroundInfo ConvertPointToGround( sf::Vector2i point );
	void CreateActor( ActorPtr actor );
	void CreateDecorImage(
		boost::shared_ptr<EditSession::DecorInfo> dec);
	std::list<GateInfoPtr> gates;
	GateInfo *selectedGate;
	MainMenu *mainMenu;
	
	void MoveSelectedPoints( 
		//sf::Vector2i delta );
		sf::Vector2<double> worldPos );

	std::list<ActorPtr> tempActors;
	GroundInfo worldPosGround;
	V2d worldPos;
	ActorParams* tempActor;
	sf::Vector2i airPos;
	MapHeader mapHeader;

	std::string envName;
	int envWorldType;

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
	char numPlayerInfoByte;
	//char GetDefaultNumPlayerInfo( MapHeader::)

	bool makingRect;
	sf::Vector2i rectStart;

	void ExtendPolygon( TerrainPoint *startPoint,
		TerrainPoint *endPoint, PolyPtr inProgress );
	bool showPoints;
	boost::shared_ptr<TerrainPolygon> extendingPolygon;
	TerrainPoint *extendingPoint;

	TerrainPolygon *cutPoly0;
	TerrainPolygon *cutPoly1;
	void ChooseCutPoly( TerrainPolygon *choice );
	bool cutChoose;
	bool cutChooseUp;

	//TerrainPoint *dirExtendingPoint;
	//TerrainPoint *extendEndTemp;

	sf::View v;


	bool showTerrainPath;
	
	sf::RenderWindow *w;
	//sf::Vector2i goalPosition;
	std::string currentFile;
	boost::filesystem::path currentPath;
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
	enum AddResult
	{
		ADD_SUCCESS,
		ADD_INVALID_RESULT,
		ADD_NO_CHANGE,
		ADD_
	};
	AddResult Add( boost::shared_ptr<TerrainPolygon> brush,
		boost::shared_ptr<TerrainPolygon> poly, TerrainPolygon *&outPoly,
		std::list<boost::shared_ptr<GateInfo>> &gateInfoList );
	AddResult InverseAdd(boost::shared_ptr<TerrainPolygon> brush,
		boost::shared_ptr<TerrainPolygon> poly, std::list<TerrainPolygon*> &outPolyList);
	TerrainPoint * GetNextAddPoint(
		TerrainPoint *previousPoint,
		sf::Vector2i &stargSegPos, TerrainPoint *&curr,
		PolyPtr &currPoly, PolyPtr &otherPoly, bool &skipBorderCase,
		bool &replaceLastPoint );
	void Sub(PolyPtr brushPtr,
		std::list<PolyPtr> &polys,
		std::list<PolyPtr> &results );
	void Extend(boost::shared_ptr<TerrainPolygon> extension,
		boost::shared_ptr<TerrainPolygon> poly );
	
	
	bool PointValid( sf::Vector2i prev, sf::Vector2i point );

	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	static LineIntersection LimitSegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d, bool firstLimitOnly = false );

	double minimumEdgeLength;
	//double minAngle;
	
	std::list<boost::shared_ptr<TerrainPolygon>> polygons;
	std::list<boost::shared_ptr<TerrainPolygon>> selectedPolygons;
	
	//std::list<boost::shared_ptr<TerrainPolygon>> polygons;
	boost::shared_ptr<TerrainPolygon> polygonInProgress;
	boost::shared_ptr<TerrainPolygon> inversePolygon;
	std::list<sf::VertexArray*> progressDrawList;
	
	sf::Font arialFont;
	sf::Text cursorLocationText;
	sf::Text scaleText;
	sf::RectangleShape scaleSpriteBGRect;

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
	Panel *bgPopup;
	Panel *shardSelectPopup;

	void CreateShardGridSelector( Panel *p,
		sf::Vector2i &pos );
	void GetShardWorldAndIndex(int selX, int selY,
		int &w, int &li );

	Panel *gateSelectorPopup;
	Panel *terrainSelectorPopup;

	Panel *enemySelectPanel;
	void SetEnemyGridIndex( GridSelector *gs,
		int x, int y,
		const std::string &eName );

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
	Brush *copiedBrush;
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
	//int CompareAngle(bool cw, V2d &origDir,
	//	V2d stayDir, V2d otherDir);
	

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
	void BoxSelectPoints(sf::IntRect rect,
		double radius);
	void PointSelectPolygon();
	void ExecuteTerrainCompletion();
	Action* ExecuteTerrainAdd(
		std::list<PolyPtr> &intersectingPolys);
	void PasteTerrain( Brush *b );
	Action* ExecuteTerrainSubtract( std::list<PolyPtr> &intersectingPolys);
	Action *ChooseAddOrSub(std::list<PolyPtr> &intersectingPolys);
	
	bool HoldingShift();
	bool HoldingControl();

	std::list<boost::shared_ptr<DecorInfo>> decorImagesBehindTerrain;
	std::list<boost::shared_ptr<DecorInfo>> decorImagesBetween;
	std::list<boost::shared_ptr<DecorInfo>> decorImagesFrontTerrain;
	Panel *decorPanel;
	Panel *editDecorPanel;
	void InitDecorPanel();
	std::string currDecorName;
	Tileset *ts_currDecor;
	sf::Sprite tempDecorSprite;
	int currDecorLayer;
	int currDecorTile;

	bool decorTracking;
	void LoadDecorImages();
	std::map<std::string, Tileset*> decorTSMap;
	int *decorTileIndexes;
	std::map<std::string, std::list<int>> decorTileIndexMap;


	bool drawingCreateRect;
	sf::Vector2i createRectStartPoint;
	sf::Vector2i createRectCurrPoint;
	AirTriggerParams *rectCreatingTrigger;

	enum Emode
	{
		CREATE_TERRAIN,
		EDIT,
		SELECT_MODE,
		CREATE_PATROL_PATH,
		CREATE_RECT,
		SET_DIRECTION,
		SELECT_POLYGONS,
		PASTE,
		PAUSED,
		CREATE_ENEMY,
		DRAW_PATROL_PATH,
		CREATE_TERRAIN_PATH,
		CREATE_GATES,
		CREATE_IMAGES,
		EDIT_IMAGES,
	};

	Emode mode;
	
};

typedef boost::shared_ptr<EditSession::DecorInfo> DecorPtr;

bool CompareDecorInfo(EditSession::DecorInfo &di0, EditSession::DecorInfo &di1);

#endif