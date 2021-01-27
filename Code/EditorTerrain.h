#ifndef __EDITORTERRAIN_H__
#define __EDITORTERRAIN_H__

#include <list>
#include <SFML\Graphics.hpp>
#include "ISelectable.h"
#include "VectorMath.h"
#include "clipper.hpp"
#include "EditorGateInfo.h"
#include "Physics.h"
#include "TerrainDecor.h"
#include "PositionInfo.h"
#include "Grass.h"
#include "SpecialTerrainTypes.h"
//#include "Enemy_HealthFly.h"
//#include "ActorParamsBase.h"

struct HealthFly;
struct TouchGrassCollection;

struct Session;

struct ISelectable;
struct GateInfo;
struct EditSession;
struct TerrainPolygon;
struct ActorParams;
struct TerrainRail;
struct Brush;

struct QuadTree;
struct GrassDiff;

struct TransformTools;

typedef std::set<std::pair<__int64,__int64>> ClipperIntPointSet;

struct GrassInfo
{
	GrassInfo()
		:gState(-1),gType(-1),index(-1)
	{

	}
	GrassInfo(int s, int t, int i)
		:gState(s), gType(t), index( i )
	{

	}

	int index;
	int gState;
	int gType;
};

struct GrassSeg
{
	//CollisionBox explosion; this might have some use in gamesession
	GrassSeg(int edgeI, int grassIndex, int rep,
		int p_gType)
		:edgeIndex(edgeI), index(grassIndex),
		reps(rep), gType( p_gType )
	{}
	int edgeIndex;
	int index;
	int reps;
	int gType;
};

struct TerrainPoint
{
	TerrainPoint(sf::Vector2i &pos, bool selected);
	~TerrainPoint()
	{
	}
	sf::Vector2i pos;
	bool selected;
	std::list<int> grass;
	GateInfoPtr gate;
	int grassStartIndex;
	bool firstPoint;
	int GetIndex();

	bool HasPrimaryGate(sf::Vector2i &gateDir);
	bool ContainsPoint(sf::Vector2f test);
	bool Intersects(sf::IntRect rect);

	sf::Vector2i oldPos;

	int index;
	static const int POINT_RADIUS = 5;
};

struct PointMoveInfo
{
	PointMoveInfo()
		:poly( NULL ), rail( NULL )
	{}

	TerrainPoint *GetPolyPoint();
	TerrainPoint *GetRailPoint();

	PolyPtr poly;
	RailPtr rail;
	int pointIndex;
	sf::Vector2i newPos;
	sf::Vector2i origPos;
	sf::Vector2i oldPos; //used for moving
	std::vector<GrassInfo> grassVec;
	std::vector<GrassInfo> prevGrassVec;
};

struct BorderInfo
{
	const static int NUM_BORDER_SIZES = 2;
	const static int NUM_BORDER_VARIATIONS = 4;
	int numQuads[NUM_BORDER_SIZES];

	BorderInfo()
	{
		Clear();
	}
	void Add(BorderInfo &bi)
	{
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			numQuads[i] += bi.numQuads[i];
		}
	}

	int GetNumSizesWithCountAboveZero()
	{
		int total = 0;
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			if (numQuads[i] > 0)
				++total;
		}
		return total;
	}

	int DecrementSizeWithCountAboveZero(int index)
	{
		int aboveZeroCounter = 0;
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			if (numQuads[i] > 0)
			{
				if (aboveZeroCounter == index)
				{
					--(numQuads[i]);
					return i;
				}
				else
					aboveZeroCounter++;
			}
		}
	}

	int GetTotal()
	{
		int total = 0;
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			total += numQuads[i];
		}
		return total;
	}
	void Clear()
	{
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			numQuads[i] = 0;
		}
	}
};

struct BorderSizeInfo
{
	void SetWidth(int w);
	int width;
	int startInter;
	int inter;
	int len;
	int edgeLen;
	int startLen;
};

struct TerrainPolygon : ISelectable, QuadTreeCollider, RayCastHandler,
	QuadTreeEntrant
{
	enum QueryType
	{
		CHECK_EMPTY,
		INTERSECT_QUAD,
		ATTACH_PLAYER,
	};

	enum RenderMode
	{
		RENDERMODE_NORMAL,
		RENDERMODE_MOVING_POINTS,
		RENDERMODE_TRANSFORM,
		RENDERMODE_FLIES,
		RENDERMODE_PHASED,
	};

	enum WaterType
	{
		WATER_NORMAL,
		WATER_GLIDE,
		WATER_LOWGRAV,
		WATER_HEAVYGRAV,
		WATER_BUOYANCY,
		WATER_ACCEL,
		WATER_ZEROGRAV,
		WATER_LAUNCHER,
		WATER_MOMENTUM,
		WATER_TIMESLOW,
		WATER_POISON,
		WATER_FREEFLIGHT,
		WATER_INVERTEDINPUTS,
		WATER_REWIND,
		WATER_SWORDPROJECTILE,
		WATER_SUPER,
		WATER_Count,
	};

	enum TerrainWorldType : int
	{
		MOUNTAIN,
		GLADE,
		DESERT,
		COVE,
		JUNGLE,
		FORTRESS,
		CORE,
		SECRETCORE,
		W1_SPECIAL,
		W2_SPECIAL,
		W3_SPECIAL,
		W4_SPECIAL,
		W5_SPECIAL,
		W6_SPECIAL,
		W7_SPECIAL,
		W8_SPECIAL,
		FLY,
		Count
	};

	enum EdgeAngleType : int
	{
		EDGE_FLAT,
		EDGE_FLATCEILING,
		EDGE_SLOPED,
		EDGE_STEEPSLOPE,
		EDGE_SLOPEDCEILING,
		EDGE_STEEPCEILING,
		EDGE_WALL,
	};

	enum State : int
	{
		IDLE,
		FADINGOUT,
		INACTIVE,
		TEMPORARILYINACTIVE,
		FADINGIN,
	};
	int frame;
	State state;

	int waterType;
	void ResetState();
	static int GetWaterWorld(int waterT);
	static int GetWaterIndexInWorld(int waterT);
	static sf::Color GetWaterColor(int waterT);
	bool IsSpecialTerrainType(int w, int var);
	void UpdateWaterType();
	bool IsPhaseType();
	bool IsInversePhaseType();
	bool IsSometimesActiveType();
	void FadeOut();
	bool IsActive();


	void MakeGlobalPath(
		V2d &startPos,
		std::vector<sf::Vector2i> &path);

	void SetFlyTransform( PolyPtr poly, TransformTools *tr);
	void UpdateState();
	sf::Vector2f flyTransScale;
	float flyTransRotate;
	void GenerateMyFlies();
	std::vector<HealthFly*> myFlies;
	void AddFliesToQuadTree(QuadTree *tree);
	void AddFliesToWorldTrees();
	sf::Vertex *flyQuads;
	Tileset *ts_fly;

	Edge *playerEdge;
	double playerQuant;
	Edge * CheckPlayerOnLine( V2d &edgePos, double &quant );
	V2d playerEdgePos;

	Session *sess;
	PolyPtr mostRecentCopy;
	TerrainPolygon *queryNext; //for quadtree setuff

	void DrawPoints(sf::RenderTarget *target,
		double zoomMultiple, TerrainPoint *dontShow);

	ActorPtr GetClosestEnemy(int index, double &minQuant);
	ActorPtr GetFurthestEnemy(int index, double &maxQuant);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);

	//touchgrass

	void QueryTouchGrass(QuadTreeCollider *qtc, sf::Rect<double> &r);
	void UpdateTouchGrass();
	void DrawTouchGrass(sf::RenderTarget *target);
	void AddTouchGrass(int gt);
	void SetupTouchGrass();
	void DestroyTouchGrass();
	void ResetTouchGrass();
	std::list<TouchGrassCollection*> touchGrassCollections;

	

	void SetGrassVecOn(
		int pointIndex, std::vector<GrassInfo> &gList);
	void SetGrassFromPointMoveInfoVectors(std::vector<PointMoveInfo> &pMoveVec);
	void FillGrassVec(TerrainPoint *point,
		std::vector<GrassInfo> &gVec );
	//trying to get decor working

	static DecorType GetDecorType(const std::string &dStr);
	void DrawDecor(sf::RenderTarget *target);
	void GenerateDecor();
	void UpdateDecorSprites();
	static void UpdateDecorLayers();
	DecorExpression * CreateDecorExpression(DecorType dType,
		int bgLayer);
	void AddDecorExpression(DecorExpression *expr);
	void DestroyDecor();
	bool IsEmptyRect(sf::Rect<double> &rect);


	void HandleEntrant(QuadTreeEntrant *qte);
	void HandleRayCollision(Edge *edge,
		double edgeQuantity, double rayPortion);
	
	int numEdgesHitByRay;
	Edge * rcEdge;
	double rcQuant;
	Edge *ignoreEdge;
	double rcPortion;
	V2d rayStart;
	V2d rayEnd;

	void QueryQuadTree(QuadTree *tree,
		QueryType qType, sf::Rect<double> & r);

	QuadTree *queryTree;
	QueryType queryType;
	V2d quadCheck[4];

	TerrainDecorInfo *tdInfo;

	bool emptyResult;
	bool resultQuadTouching;
	//sf::Rect<double> queryRect;

	std::list<DecorExpression*> decorExprList;
	std::list<DecorLayer*> DecorLayers;
	QuadTree *decorTree;
	QuadTree *myTerrainTree;

	std::vector<Grass> activeGrass;
	//normal stuff

	RenderMode renderMode;
	void SetRenderMode(RenderMode rm);

	void CancelTransformation();
	PolyPtr CompleteTransformation(TransformTools *tr);
	void UpdateTransformation( TransformTools *tr);

	static double GetSteepThresh() { return .4; }

	EdgeAngleType GetEdgeAngleType(int index);
	static EdgeAngleType GetEdgeAngleType(Edge * e);
	static EdgeAngleType GetEdgeAngleType(V2d &normal);

	static bool IsFlatGround(sf::Vector2<double> &normal);
	static bool IsSlopedGround(sf::Vector2<double> &normal);
	static bool IsSteepGround(sf::Vector2<double> &normal);
	static bool IsWall(sf::Vector2<double> &normal);

	void DrawBorderQuads(sf::RenderTarget *target);

	void Scale(float f);
	void Rotate( sf::Vector2f &center, float degrees);

	void SetBorderTileset();
	void UpdateGrassType();
	static sf::IntRect GetBorderSubRect(int tileWidth, EdgeAngleType et, int var);

	sf::Vertex *borderQuads;
	int totalNumBorderQuads;
	void GenerateBorderMesh();
	void GenerateWaterBorderMesh();
	Tileset *ts_border;

	static int GetBorderQuadIntersect(int tileWidth);

	static double GetExtraForInward(Edge *e);
	static double GetSubForOutward(Edge *e);

	static bool IsAcute(Edge *e0);
	static V2d GetBisector(Edge *e);

	Tileset *ts_grass;
	void SetupEdges();
	std::vector<Edge> edges;
	void AddEdgesToQuadTree(QuadTree *tree);
	void CalcEdgeAABBs();
	//QuadTree *edgeTree;
	bool isBrushTest;
	bool Load(std::ifstream &is);
	bool LoadGrass(std::ifstream &is);

	bool CheckRectIntersectEdges(
		sf::Rect<double> &r);
	
	void MakeInverse();
	static sf::Vector2i GetExtreme(TerrainPoint *p0,
		TerrainPoint *p1);
	Edge *GetEdge(int index);
	Edge *GetPrevEdge(int index);
	TerrainPoint *GetPoint(int index);
	TerrainPoint *GetInverseOuterRectPoint(int index);
	TerrainPoint *GetFinalizeInversePoint(int index);
	TerrainPoint *GetEndPoint();
	TerrainPoint *GetStartPoint();
	TerrainPoint *GetNextPoint(int index);
	TerrainPoint *GetPrevPoint(int index);
	void DeactivateGates();
	void ActivateGates();

	bool AABBIntersection(PolyPtr poly);
	bool IsOtherAABBWithinMine(PolyPtr poly);
	bool PointsTooCloseToEachOther(double radius );
	bool LinesIntersectMyself();
	bool HasSlivers();
	bool IntersectsMyOwnEnemies();
	bool IntersectsMyOwnGates();
	//TerrainRender *tr;
	int terrainVariation;
	TerrainWorldType terrainWorldType;
	int GetNumGrassTotal();
	int GetNumGrass(int i, bool &rem);
	int GetNumGrassChanges();

	void SetupGrass(int i, int &grassIndex);
	void SetupGrass();
	void SetupGrassAfterGameSessionLoad(std::list<GrassSeg> &segments);
	void AddGrassToQuadTree(QuadTree *tree );
	void SetGrassOn(int gIndex, bool on,
		int gType );
	void SetGrassFromAction(int gIndex, int state, int gType,
		bool show );
	void SetGrassState(int gIndex, int state,
		int gType );
	Grass::GrassType GetDefaultGrassType();
	V2d GetGrassCenter(int gIndex);
	void CopyMyGrass(PolyPtr other);


	sf::Shader *pShader;
	bool IsValidInProgressPoint(sf::Vector2i point);
	void UpdateLinePositions();
	TerrainPolygon();
	TerrainPolygon(TerrainPolygon &poly, bool pointsOnly,
		bool storeSelectedPoints = false );
	~TerrainPolygon();
	void UpdateLineColor(int i);
	void UpdateLineColors();
	static sf::Vector2i TrimSliverPos(sf::Vector2<double> &prevPos,
		sf::Vector2<double> &pos, sf::Vector2<double> &nextPos,
		double minAngle, bool cw);
	void CopyPoints(PolyPtr poly,
		bool storeSelected = false );
	PolyPtr Copy();
	PolyPtr InverseCopy();
	bool copiedInverse;
	int GetSpecialPolyIndex();
	bool IsTouchingEnemiesFromPoly(PolyPtr p);
	

	std::vector<sf::Vector2f> triBackups;//for transforms

	std::vector<std::vector<TerrainPoint>> pointVector;
	std::vector<TerrainPoint> &PointVector();
	void Reserve(int nPoints);

	TerrainPoint *GetSamePoint(sf::Vector2i &p);
	TerrainPoint * AddPoint(sf::Vector2i &p, bool sel);
	TerrainPoint * AddInverseBorderPoint(sf::Vector2i &p, bool sel);
	int GetNumPoints();
	void RemoveLastPoint();
	void ClearPoints();
	void SetMaterialType(
		int world, int variation);
	void SetGrassType(int gType);
	void UpdateMaterialType();
	void RemoveSlivers();
	

	bool TryFixAllSlivers();

	bool TryToMakeInternallyValid();
	bool FixSliver(int i);
	bool FixSliver(int i, std::set<int> &brokenSlivers,
		bool &error );

	int FixNearPrimary(int i,bool currLocked = false);

	//std::string material;
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	//void MovePointsFinalize();
	void FinalizeInverse();
	void Reset();
	void SoftReset();
	void Draw(bool showPath, double zoomMultiple, sf::RenderTarget * rt, bool showPoints, TerrainPoint *dontShow);
	void DrawFlies(sf::RenderTarget *target);
	void DrawGrass(sf::RenderTarget *target);
	void FixWinding();
	void FixWindingInverse();
	bool IsClockwise();
	bool AlignExtremes();
	bool AlignExtremes(
		std::list<PointMoveInfo> &lockPoints,
		std::list<PointMoveInfo> &addedLockPoints );
		//std::vector<PointMoveInfo> &lockPoints);
	//bool AlignExtremesMovePoints(
	//	std::map<TerrainPoint*, sf::Vector2f> &affectedPoints);
	void UpdateGrass();

	void BrushSave(std::ofstream & of);
	void WriteFile(std::ofstream &of);
	void WriteGrass(std::ofstream &of);

	int grassSize;
	int grassSpacing;


	int AddGrassChanges(GrassDiff* gDiffArray);
	void ShowGrass(bool show);
	void ProcessGrass(std::list<GrassSeg> &segments );
	void SwitchGrass(V2d &mousePos, bool on,
		bool sessionIsCaller,
		int gType );
	void SetSelected(bool select);

	void UpdateBounds();

	void SetLayer(int p_layer);
	bool IsInternallyValid();
	sf::IntRect GetAABB();
	sf::FloatRect GetAngledAABB(float rotation);
	V2d GetDCenter();
	void MovePoint( int index, sf::Vector2i &delta);
	void SetPointPos(int index, sf::Vector2i &p);


	bool IsInsideArea(V2d &point);
	bool ContainsPoint(sf::Vector2f point);


	bool IntersectsActorParams(ActorPtr a);
	bool Intersects(sf::IntRect rect);
	bool IntersectsGate(GateInfo *gi);
	bool IsPlacementOkay();
	//void Move( sf::Vector2i delta );
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void MiniDraw(sf::RenderTarget *target);
	void SetTerrainColor(sf::Color c);
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void Activate();
	void AddGatesToList(std::list<GateInfoPtr> &gates);
	bool IsTouching(PolyPtr poly);
	bool Contains(PolyPtr poly);

	int LinesIntersect(PolyPtr poly);
	void TryFixPointsTouchingLines();
	bool LinesIntersectInProgress(sf::Vector2i p );
	bool IsCompletionValid();
	void CopyPointsToClipperPath(ClipperLib::Path & p);
	void AddPointsFromClipperPath(ClipperLib::Path &p);
	void AddPointsFromClipperPath(ClipperLib::Path &p,
		ClipperIntPointSet &fusedPoints );
	void AddPointsFromClipperPath(ClipperLib::Path &p,
		ClipperLib::Path &clipperIntersections,
		std::list<TerrainPoint*> &intersections );
	bool PointTooCloseToPoints(sf::Vector2i point,
		int minDistance);
	bool PointTooClose(sf::Vector2i point,
		int minDistance, bool inProgress = false);
	bool PointTooCloseToLines(sf::Vector2i point,
		int minDistance, bool inProgress = false);
	bool PointsTooCloseToSegInProgress(sf::Vector2i point,
		int minDistance, bool finalPoint = false );
	bool SegmentWithinDistanceOfPoint(
		sf::Vector2i startSeg,
		sf::Vector2i endSeg,
		sf::Vector2i testPoint,
		int distance);
	void AddGatesToBrush(Brush *b,
		std::list<GateInfoPtr> &gateInfoList);
	void AddEnemiesToBrush(Brush *b);
	TerrainPoint *GetClosePoint(double radius, V2d &pos);
	bool IsCloseToFirstPoint(double radius, V2d &p);
	int GetNumSelectedPoints();
	PolyPtr CreateCopyWithSelectedPointsRemoved();
	sf::Color selectCol;
	sf::Color fillCol;


	bool CanApply();
	bool CanAdd();




	//bool movingPointMode;

	void Move(sf::Vector2i move);

	sf::Vertex *lines;
	sf::VertexArray *va;
	sf::Vertex *grassVA;

	enum GrassState : int
	{
		G_OFF_DONT_SHOW,
		G_OFF,
		//G_OFF_EDITED,
		//G_ON_EDITED,
		G_ON
	};

	
	std::vector<GrassInfo> grassStateVec;
	//0 is off and not showing
	//1 is off and showing
	//2 is on and showing
	int numGrassTotal;
	int vaSize;
	//bool selected;
	int left;
	int right;
	int top;
	int bottom;
	std::list<sf::Vector2i> path;

	const static int inverseExtraBoxDist = 500;

	//enemymap
	std::map<TerrainPoint*, std::list<ActorPtr>> enemies;
	std::vector<std::pair<ActorPtr, PositionInfo>> enemyPosBackups;
	void BackupEnemyPositions();
	void StoreEnemyPositions(std::vector<std::pair<ActorPtr, PositionInfo>>&b);
	void BackupGrass();
	bool isGrassBackedUp;
	int NumGrassChanged();
	bool grassChanged;
	std::vector<GrassInfo> grassStateVecBackup;

	
	int writeIndex;
	bool finalized;

	int layer; //0 is game layer. 1 is bg

	bool inverse;
};







typedef std::map<PolyPtr, std::list<PointMoveInfo>> PointMap;



typedef std::map<PolyPtr, std::vector<PointMoveInfo>> PointVectorMap;

struct PointMover
{
	Brush *MakeBrush();
	//PointVectorMap myMap;
	//std::map<ActorPtr, PositionInfo> enemyBackups;
	//std::map<ActorPtr, PositionInfo> newEnemyPos;
	std::map<PolyPtr,std::vector<PointMoveInfo>> movePoints;
	std::map < RailPtr, std::vector<PointMoveInfo>> railMovePoints;
	std::vector <std::pair<ActorPtr, PositionInfo>> oldEnemyPosInfo;
	std::vector <std::pair<ActorPtr, PositionInfo>> newEnemyPosInfo;

};

typedef std::map<RailPtr, std::list<PointMoveInfo>> RailPointMap;

#endif