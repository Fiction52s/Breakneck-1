#ifndef __ACTORPARAMS_H__
#define __ACTORPARAMS_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include "ISelectable.h"
#include "EditorActors.h"
#include "EditorTerrain.h"

struct TerrainPolygon;

struct ActorParams : ISelectable
{

	virtual ActorParams *Copy() = 0;
	~ActorParams();
	ActorParams(ActorType *at);
	bool SetLevel(int lev);
	virtual void Init() {};
	virtual void WriteParamFile(std::ofstream &of);
	void WriteBasicParamFile(std::ofstream &of);
	void WriteFile(std::ofstream &of);
	void WriteMonitor(std::ofstream &of);
	void LoadBool(std::ifstream &is, bool &b);
	void WriteBool(std::ofstream &of, bool b);
	void WriteLevel(std::ofstream &of);
	void WritePath( std::ofstream &of );
	void WriteLoop( std::ofstream &of );
	void AnchorToGround(PolyPtr poly,
		int eIndex, double quantity);
	void AnchorToRail(TerrainRail *rail,
		int eIndex, double quantity);
	void AnchorToGround(GroundInfo &gi);
	void AnchorToRail(GroundInfo &gi);
	void UnAnchor();
	void UpdateGroundedSprite();

	virtual std::list<sf::Vector2i> GetGlobalPath();
	virtual void SetPath(std::list<sf::Vector2i> &globalPath);

	void DrawLevel(sf::RenderTarget *target);
	void DrawBoundary(sf::RenderTarget *target);
	void DrawMonitor(sf::RenderTarget *target);
	void LoadGrounded(std::ifstream &is);
	void LoadRailed(std::ifstream &is);
	void LoadAerial(std::ifstream &is);
	void LoadGlobalPath(std::ifstream &is );
	void LoadMonitor(std::ifstream &is);
	void LoadEnemyLevel(std::ifstream &is);
	virtual void SetParams();
	void SetBasicParams();
	void SetBasicPanelInfo();
	virtual void SetPanelInfo();
	
	void PlaceAerial(sf::Vector2i &pos);
	void PlaceGrounded( PolyPtr tp,
		int edgeIndex, double quant);
	void PlaceRailed(TerrainRail *rail,
		int edgeIndex, double quant);


	virtual void SetBoundingQuad();
	virtual void UpdateExtraVisuals()
	{}

	//ISelectable( ISelectableType type );
	virtual bool ContainsPoint(sf::Vector2f test);
	virtual bool Intersects(sf::IntRect rect);
	virtual bool IsPlacementOkay();
	virtual void Move(sf::Vector2i delta);
	virtual void BrushDraw(sf::RenderTarget *target,
		bool valid);
	virtual void Draw(sf::RenderTarget *target);
	virtual void DrawPreview(sf::RenderTarget *target);
	virtual void Deactivate();
	virtual void Activate();

	virtual void DrawQuad(sf::RenderTarget *target);

	virtual void SetSelected(bool select);

	virtual bool CanApply();
	bool CanAdd();

	//sf::Sprite icon;
	sf::Sprite image;
	ActorGroup *group;
	ActorType *type;
	sf::Vector2i position;

	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it


	GroundInfo *groundInfo;
	bool hasMonitor;
	sf::VertexArray boundingQuad;

	std::list<sf::Vector2i> localPath;
	bool loop;
	sf::VertexArray *lines; //local pos

	int enemyLevel;
};

struct PlayerParams : public ActorParams
{
	PlayerParams( ActorType *at,
		sf::Vector2i pos);
	PlayerParams(ActorType *at,
		std::ifstream &is );

	bool CanApply();
	void Deactivate();
	ActorParams *Copy();
	void Activate();
};

//extra

struct GroundTriggerParams : public ActorParams
{
	GroundTriggerParams(ActorType *at, PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		bool facingRight,
		const std::string &typeStr);
	GroundTriggerParams(ActorType *at, PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GroundTriggerParams(ActorType *at,
		std::ifstream &is);
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	std::string typeStr;
	bool facingRight;
};

struct FlowerPodParams : public ActorParams
{
	FlowerPodParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		const std::string &typeStr);
	FlowerPodParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	FlowerPodParams(ActorType *at,
		std::ifstream &is);
	
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	std::string typeStr;
	bool facingRight;
};

struct AirTriggerParams : public ActorParams
{
	AirTriggerParams(ActorType *at, sf::Vector2i &pos, const std::string &typeStr,
		int w, int h);
	AirTriggerParams(ActorType *at,
		sf::Vector2i &pos);
	AirTriggerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	void SetRect(int width, int height, sf::Vector2i &center);
	ActorParams *Copy();
	void Draw(sf::RenderTarget *target);
	std::string trigType;
	int rectWidth;
	int rectHeight;

	sf::RectangleShape triggerRect;
	sf::Text nameText;
};

struct CameraShotParams : public ActorParams
{
	CameraShotParams(ActorType *at, sf::Vector2i &pos, const std::string &typeStr,
		float z );
	CameraShotParams(ActorType *at,
		sf::Vector2i &pos);
	CameraShotParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	void Init();
	void SetZoom(float z);
	void SetZoom(sf::Vector2i &testPoint);
	ActorParams *Copy();
	void Draw(sf::RenderTarget *target);
	std::string camName;
	float zoom;

	const static float CAMWIDTH;
	const static float CAMHEIGHT;
	//int rectWidth;

	sf::RectangleShape camRect;
	sf::Text nameText;
	sf::Text zoomText;
};

struct NexusParams : public ActorParams
{
	NexusParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int nexusIndex);
	NexusParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	NexusParams(ActorType *at,
		std::ifstream &is);
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	int nexusIndex;
};

struct ShipPickupParams : public ActorParams
{
	ShipPickupParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		bool facingRight);
	ShipPickupParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	ShipPickupParams(ActorType *at,
		std::ifstream &is);
	
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool facingRight;
};

struct GoalParams : public ActorParams
{
	GoalParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GoalParams(ActorType *at,
		std::ifstream &is);
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
};

struct PoiParams : public ActorParams
{
	PoiParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		const std::string &name);
	PoiParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	PoiParams(ActorType *at,
		sf::Vector2i &pos,
		const std::string &name );
	PoiParams(ActorType *at,
		sf::Vector2i &pos);
	PoiParams(ActorType *at,
		std::ifstream &is);

	void Draw(sf::RenderTarget *target);

	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();

	std::string name;
	static sf::Font *font;
	sf::Text nameText;
};

struct XBarrierParams : ActorParams
{
	XBarrierParams(ActorType *at,
		sf::Vector2i &pos, const std::string &name);
	XBarrierParams(ActorType *at,
		sf::Vector2i &pos);
	XBarrierParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();
	void Init();
	void Draw(sf::RenderTarget *target);
	ActorParams *Copy();

	sf::Vertex line[2];
	std::string name;
	sf::Text nameText;
	bool hasEdge;
};

struct ExtraSceneParams : ActorParams
{
	ExtraSceneParams(ActorType *at,
		sf::Vector2i &pos, const std::string &name);
	ExtraSceneParams(ActorType *at,
		sf::Vector2i &pos);
	ExtraSceneParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();
	void Init();
	void Draw(sf::RenderTarget *target);
	ActorParams *Copy();

	std::string name;
	sf::Text nameText;
	int extraSceneType;
};

struct KeyParams : public ActorParams
{
	KeyParams(ActorType *at,
		sf::Vector2i &pos, int numKeys,
		int zoneType);
	KeyParams(ActorType *at,
		sf::Vector2i &pos);
	KeyParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();
	int numKeys;
	int zoneType;
};

struct ShardParams : public ActorParams
{
	ShardParams(ActorType *at,
		sf::Vector2i &pos);
	ShardParams(ActorType *at,
		sf::Vector2i &pos, int world,
		int li);
	ShardParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetShard(int world, int realX, int realY);
	void SetShard(int world, int li);
	void SetParams();
	void SetPanelInfo();
	void SetShardFromStr();

	int world;
	int sX;
	int sY;
	int localIndex;
	int GetTotalIndex();

	
	ActorParams *Copy();

	std::string shardStr;

};

struct RaceFightTargetParams : public ActorParams
{
	RaceFightTargetParams(ActorType *at,
		sf::Vector2i &pos);
	RaceFightTargetParams(ActorType *at,
		std::ifstream &is);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();
};

struct BlockerParams : public ActorParams
{
	enum BlockerType
	{
		NORMAL,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		BLACK
	};

	BlockerParams(ActorType *at,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		int bType, bool invinc, int spacing,
		int level);
	BlockerParams(ActorType *at,
		sf::Vector2i &pos);
	BlockerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	bool armored;
	int spacing;

	BlockerType bType;
	//will have multiple types
};


//w1
struct ComboerParams : public ActorParams
{
	ComboerParams(ActorType *at,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		float speed,
		bool loop);
	ComboerParams(ActorType *at,
		sf::Vector2i &pos);
	ComboerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	int speed;
	int swoopSpeed;
};

struct BoosterParams : public ActorParams
{
	BoosterParams(ActorType *at,
		sf::Vector2i &pos, int strength);
	BoosterParams(ActorType *at,
		sf::Vector2i &pos);
	BoosterParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int strength;
};

struct SpringParams : public ActorParams
{
	SpringParams(ActorType *at,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath,
		int moveFrames);
	SpringParams(ActorType *at,
		sf::Vector2i &pos);
	SpringParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	void SetPath(std::list<sf::Vector2i> &globalPath);
	
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	int moveFrames;
};

struct PatrollerParams : public ActorParams
{
	PatrollerParams(ActorType *at,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		float speed,
		bool loop);
	PatrollerParams(ActorType *at,
		sf::Vector2i &pos);
	PatrollerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	int speed;
	int swoopSpeed;
};


struct BasicGroundEnemyParams : public ActorParams
{
	BasicGroundEnemyParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity,
		int level=0);
	BasicGroundEnemyParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	virtual void WriteSpecialParams(std::ofstream &of) {}
	ActorParams *Copy();
};

struct BasicRailEnemyParams : public ActorParams
{
	BasicRailEnemyParams(ActorType *at,
		TerrainRail *rail,
		int edgeIndex, double edgeQuantity,
		int level = 0);
	BasicRailEnemyParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	virtual void WriteSpecialParams(std::ofstream &of) {}
	ActorParams *Copy();
};

struct BasicAirEnemyParams : public ActorParams
{
	BasicAirEnemyParams(ActorType *at,
		sf::Vector2i &pos, int level =0);
	BasicAirEnemyParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPanelInfo();
	void SetParams();
	virtual void SetSpecialParams() {}
	virtual void SetSpecialPanelInfo() {}
	virtual void WriteSpecialParams(std::ofstream &of) {}
	ActorParams *Copy();
	virtual void Draw(sf::RenderTarget *target);
};

struct JugglerParams : public BasicAirEnemyParams
{
	JugglerParams(ActorType *at,
		sf::Vector2i &pos, int level = 0);
	JugglerParams(ActorType *at,
		std::ifstream &is);
	JugglerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity, int level = 0);


	void SetSpecialParams();
	void SetSpecialPanelInfo();
	void WriteSpecialParams( std::ofstream &of);
	ActorParams *Copy();

	int numJuggles;
};

struct GroundedJugglerParams : public BasicGroundEnemyParams
{
	GroundedJugglerParams(ActorType *at,
		std::ifstream &is);
	GroundedJugglerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity, int level = 0);


	void SetSpecialParams();
	void SetSpecialPanelInfo();
	void WriteSpecialParams(std::ofstream &of);
	ActorParams *Copy();

	int numJuggles;
};
//struct AirPathEnemyParamsLoop : public ActorParams
//{
//	AirPathEnemyParamsLoop(ActorType *at,
//		sf::Vector2i &pos, int level = 0);
//	AirPathEnemyParamsLoop(ActorType *at,
//		std::ifstream &is);
//	ActorParams *Copy();
//	void SetPanelInfo();
//	void WriteParamFile(std::ofstream &of);
//	void Draw(sf::RenderTarget *target);
//};

struct CrawlerParams : public ActorParams
{
	CrawlerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity,
		int level );
	CrawlerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	CrawlerParams(ActorType *at,
		std::ifstream &is);
	
	ActorParams *Copy();
};

struct BasicTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	BasicTurretParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	BasicTurretParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
};

struct FootTrapParams : public ActorParams
{
	FootTrapParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	FootTrapParams(ActorType *at, EditSession *edit);
	FootTrapParams(ActorType *at,
		std::ifstream &is);
	
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();
	//void Draw( sf::RenderTarget *target );
};

struct AirdasherParams : public ActorParams
{
	AirdasherParams(ActorType *at,
		sf::Vector2i &pos);
	AirdasherParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();
};

struct BossCrawlerParams : public ActorParams
{
	BossCrawlerParams(ActorType *at,

		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	BossCrawlerParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
};


//w2
struct BatParams : public ActorParams
{
	BatParams(ActorType *at,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		int framesBetween,
		//int nodeDistance,
		int bulletSpeed,
		bool loop);
	BatParams(ActorType *at,
		sf::Vector2i &pos);
	BatParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int bulletSpeed;
	//int nodeDistance;
	int framesBetweenNodes;
	//int speed;
};

struct GravityFallerParams : public ActorParams
{
	GravityFallerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int variation);
	GravityFallerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GravityFallerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	ActorParams *Copy();
	
	void SetParams();
	void SetPanelInfo();

	int variation;
};

struct StagBeetleParams : public ActorParams
{
	StagBeetleParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity,
		bool clockwise, float speed);
	//StagBeetleParams( 
	//	sf::Vector2i &pos, bool facingRight,
	//	float speed );
	StagBeetleParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	StagBeetleParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	float speed;
};

struct PoisonFrogParams : public ActorParams
{
	PoisonFrogParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpWaitFrames);
	PoisonFrogParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	PoisonFrogParams(EditSession *edit);
	PoisonFrogParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void UpdatePath();
	void Draw(sf::RenderTarget *target);
	void UpdateExtraVisuals();
	//void SetParams();
	//void SetPanelInfo();
	ActorParams *Copy();
	
	void SetParams();
	void SetPanelInfo();
	int gravFactor;
	sf::Vector2i jumpStrength;
	//int jumpStrengthX;
	//int jumpStrengthY;
	int jumpWaitFrames;
	sf::VertexArray pathQuads;
};

struct CurveTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	CurveTurretParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		double bulletSpeed,
		int framesWait,
		sf::Vector2i gravFactor,
		bool relativeGrav);
	ActorParams *Copy();
	CurveTurretParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	CurveTurretParams(ActorType *at,
		std::ifstream &is);
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	
	void UpdateBulletCurve();
	void Draw(sf::RenderTarget *target);
	void UpdateExtraVisuals();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	bool relativeGrav;
	int framesWait;
	sf::Vector2i gravFactor;
	sf::VertexArray bulletPathQuads;

	int curveFactor;
};

struct GravityModifierParams : public ActorParams
{
	GravityModifierParams(ActorType *at,
		sf::Vector2i &pos, int strength);
	GravityModifierParams(ActorType *at,
		sf::Vector2i &pos);
	GravityModifierParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();


	ActorParams *Copy();

	int strength;
};


struct BossBirdParams : public ActorParams
{
	BossBirdParams(ActorType *at,

		sf::Vector2i &pos);
	BossBirdParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	
	void Draw(sf::RenderTarget *target);
	sf::VertexArray debugLines;
	void CreateFormation();
	int width;
	int height;
};

struct GravitySpringParams: public ActorParams
{
	GravitySpringParams(ActorType *at,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath,
		int speed);
	GravitySpringParams(ActorType *at,
		sf::Vector2i &pos);
	GravitySpringParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	void Draw(sf::RenderTarget *target);

	
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	int speed;
};


//w3

struct BounceSpringParams : public ActorParams
{
	BounceSpringParams(ActorType *at,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath);
	BounceSpringParams(ActorType *at,
		sf::Vector2i &pos);
	BounceSpringParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	void Draw(sf::RenderTarget *target);


	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();
};

//struct BounceBoosterParams : public ActorParams
//{
//	BounceBoosterParams(ActorType *at,
//		sf::Vector2i &pos,
//		std::list<sf::Vector2i> &globalPath );
//	BounceBoosterParams(ActorType *at,
//		sf::Vector2i &pos);
//	BounceBoosterParams(ActorType *at,
//		std::ifstream &is);
//	void WriteParamFile(std::ofstream &of);
//	void SetPath(
//		std::list<sf::Vector2i> &globalPath);
//	void Draw(sf::RenderTarget *target);
//
//
//	ActorParams *Copy();
//
//	void SetParams();
//	void SetPanelInfo();
//};

struct PulserParams : public ActorParams
{
	PulserParams(ActorType *at,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath,
		int framesBetween,
		bool loop);
	PulserParams(ActorType *at,
		sf::Vector2i &pos);
	PulserParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int framesBetweenNodes;
	//int speed;
};

struct OwlParams : public ActorParams
{
	OwlParams(ActorType *at,
		sf::Vector2i &pos,
		int moveSpeed,
		int bulletSpeed,
		int rhythmFrames);
	OwlParams(ActorType *at,
		sf::Vector2i &pos);
	OwlParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int moveSpeed;
	int bulletSpeed;
	int rhythmFrames;
};

struct BadgerParams : public ActorParams
{
	BadgerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity,
		int speed, int jumpStrength);

	BadgerParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	BadgerParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int speed;
	int jumpStrength;
};

struct CactusParams : public ActorParams
{
	CactusParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity,
		int bulletSpeed, int rhythm,
		int amplitude);

	CactusParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);

	CactusParams(ActorType *at,
		std::ifstream &is);


	void WriteParamFile(std::ofstream &of);
	
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int bulletSpeed;
	int rhythm;
	int amplitude;
};

struct BossCoyoteParams : public ActorParams
{
	BossCoyoteParams(ActorType *at,
		sf::Vector2i &pos);
	BossCoyoteParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	
	void CreateFormation();
	void Draw(sf::RenderTarget *target);
	int radius;
	sf::VertexArray debugLines;
};


//w4


struct TeleporterParams : public ActorParams
{
	TeleporterParams(ActorType *at,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath);
	TeleporterParams(ActorType *at,
		sf::Vector2i &pos);
	TeleporterParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	void Draw(sf::RenderTarget *target);

	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();
};

struct TurtleParams : public ActorParams
{
	TurtleParams(ActorType *at,
		sf::Vector2i &pos);
	TurtleParams(ActorType *at,
		std::ifstream &is);
	/*int moveSpeed,
	int bulletSpeed,
	int rhythmFrames );
	TurtleParams(
	sf::Vector2i &pos );*/
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();
};

struct CoralParams : public ActorParams
{
	CoralParams(ActorType *at,
		sf::Vector2i &pos,
		int moveFrames);
	CoralParams(ActorType *at,
		sf::Vector2i &pos);
	CoralParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int moveFrames;
};

struct CheetahParams : public ActorParams
{
	/*CactusParams(
	PolyPtr edgePolygon,
	int edgeIndex, double edgeQuantity,
	int bulletSpeed, int rhythm,
	int amplitude );*/

	CheetahParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	CheetahParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();
};

struct SpiderParams : public ActorParams
{
	SpiderParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity,
		int speed);

	SpiderParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	SpiderParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int speed;
};

struct RailParams : public ActorParams
{
	RailParams(ActorType *at,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		bool p_accelerate, int p_level );
	RailParams(ActorType *at,
		sf::Vector2i &pos);
	RailParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	bool accelerate;
};

struct BossTigerParams : public ActorParams
{
	BossTigerParams(ActorType *at,
		sf::Vector2i &pos);

	BossTigerParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void CreateFormation();
	
	void Draw(sf::RenderTarget *target);
	sf::VertexArray debugLines;
	int radius1;
	int radius2;
};


//w5
struct SharkParams : public ActorParams
{
	SharkParams(ActorType *at,
		sf::Vector2i &pos,
		int circleFrames);
	SharkParams(ActorType *at,
		sf::Vector2i &pos);
	SharkParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int circleFrames;
};

struct SwarmParams : public ActorParams
{
	SwarmParams(ActorType *at,
		sf::Vector2i &pos,
		int liveFrames);
	SwarmParams(ActorType *at,
		sf::Vector2i &pos);
	SwarmParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int liveFrames;
};

struct GhostParams : public ActorParams
{
	GhostParams(ActorType *at,
		sf::Vector2i &pos,
		int speed);
	GhostParams(ActorType *at,
		sf::Vector2i &pos);
	GhostParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int speed;
};

struct OvergrowthParams : public ActorParams
{
	OvergrowthParams(ActorType *at,
		PolyPtr edgePolygon,
		int edgeIndex, double edgeQuantity);
	OvergrowthParams(ActorType *at,
		std::ifstream &is);
	/*OvergrowthParams(
	PolyPtr edgePolygon,
	int edgeIndex, double edgeQuantity );*/


	void WriteParamFile(std::ofstream &of);
	
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();
};

struct BossGatorParams : public ActorParams
{
	BossGatorParams(ActorType *at,

		sf::Vector2i &pos);
	BossGatorParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	
	void Draw(sf::RenderTarget *target);
	sf::CircleShape circles[5];
	int orbRadius;
	int radius;
};


//w6
struct SpecterParams : public ActorParams
{
	SpecterParams(ActorType *at,
		sf::Vector2i &pos);

	SpecterParams(ActorType *at,
		std::ifstream &is);
	/*int moveSpeed,
	int bulletSpeed,
	int rhythmFrames );
	TurtleParams(
	sf::Vector2i &pos );*/
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();
};

struct CopycatParams : public ActorParams
{
	CopycatParams(ActorType *at,
		sf::Vector2i &pos);
	CopycatParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();
};

struct NarwhalParams : public ActorParams
{
	NarwhalParams(ActorType *at,
		sf::Vector2i &pos,
		sf::Vector2i &dest,
		int moveFrames
	);
	NarwhalParams(ActorType *at,
		sf::Vector2i &pos);
	NarwhalParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPath(std::list<sf::Vector2i> &globalPath);
	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int moveFrames;
	sf::Vector2i dest;
	void Draw(sf::RenderTarget *target);
};

struct GorillaParams : public ActorParams
{
	GorillaParams(ActorType *at,
		sf::Vector2i &pos, int wallWidth,
		int followFrames);
	GorillaParams(ActorType *at,
		sf::Vector2i &pos);
	GorillaParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	
	ActorParams *Copy();

	int wallWidth;
	int followFrames;
};

struct BossSkeletonParams : public ActorParams
{
	BossSkeletonParams(ActorType *at,

		sf::Vector2i &pos);
	BossSkeletonParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
};




#endif