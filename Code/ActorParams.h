#ifndef __ACTORPARAMS_H__
#define __ACTORPARAMS_H__

#include <fstream>
#include "EditSession.h"


struct ActorParams : ISelectable
{

	virtual ActorParams *Copy() = 0;
	ActorParams(ActorType *at);
	virtual void Init() {};
	virtual void WriteParamFile(std::ofstream &of) = 0;
	void WriteFile(std::ofstream &of);
	void AnchorToGround(TerrainPolygon *poly,
		int eIndex, double quantity);
	void AnchorToGround(GroundInfo &gi);
	void UnAnchor(boost::shared_ptr<ActorParams> &me);
	void UpdateGroundedSprite();
	virtual void SetPath(std::list<sf::Vector2i> &globalPath);
	void DrawBoundary(sf::RenderTarget *target);
	void DrawMonitor(sf::RenderTarget *target);
	void LoadGrounded(std::ifstream &is);
	void LoadAerial(std::ifstream &is);
	void LoadGlobalPath(std::ifstream &is );
	void LoadMonitor(std::ifstream &is);
	virtual void SetParams();
	virtual void SetPanelInfo();


	virtual void SetBoundingQuad();
	virtual void UpdateExtraVisuals()
	{}

	//ISelectable( ISelectableType type );
	virtual bool ContainsPoint(sf::Vector2f test);
	virtual bool Intersects(sf::IntRect rect);
	virtual bool IsPlacementOkay();
	virtual void Move(SelectPtr me, sf::Vector2i delta);
	virtual void BrushDraw(sf::RenderTarget *target,
		bool valid);
	virtual void Draw(sf::RenderTarget *target);
	virtual void DrawPreview(sf::RenderTarget *target);
	virtual void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	virtual void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);

	virtual void DrawQuad(sf::RenderTarget *target);

	virtual void SetSelected(bool select);

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


	GroundInfo *groundInfo;
	bool hasMonitor;
	sf::VertexArray boundingQuad;
};

struct PlayerParams : public ActorParams
{
	PlayerParams( ActorType *at,
		sf::Vector2i pos);
	PlayerParams(ActorType *at,
		std::ifstream &is );

	bool CanApply();
	void WriteParamFile(std::ofstream &of);
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable>  select);
	ActorParams *Copy();
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
};

//extra

struct GroundTriggerParams : public ActorParams
{
	GroundTriggerParams(ActorType *at, TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		bool facingRight,
		const std::string &typeStr);
	GroundTriggerParams(ActorType *at, TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GroundTriggerParams(ActorType *at,
		std::ifstream &is);
	bool CanApply();
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
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		const std::string &typeStr);
	FlowerPodParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	FlowerPodParams(ActorType *at,
		std::ifstream &is);
	bool CanApply();
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

	bool CanApply();
	void SetRect(int width, int height, sf::Vector2i &center);
	ActorParams *Copy();
	void Draw(sf::RenderTarget *target);
	std::string trigType;
	int rectWidth;
	int rectHeight;

	sf::RectangleShape triggerRect;
	sf::Text nameText;
};

struct NexusParams : public ActorParams
{
	NexusParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int nexusIndex);
	NexusParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	NexusParams(ActorType *at,
		std::ifstream &is);
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	int nexusIndex;
};

struct ShipPickupParams : public ActorParams
{
	ShipPickupParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		bool facingRight);
	ShipPickupParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	ShipPickupParams(ActorType *at,
		std::ifstream &is);
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool facingRight;
};

struct GoalParams : public ActorParams
{
	GoalParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GoalParams(ActorType *at,
		std::ifstream &is);
	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	//void Draw( sf::RenderTarget *target );
};

struct PoiParams : public ActorParams
{
	enum Barrier
	{
		NONE,
		X,
		Y
	};

	Barrier barrier;
	PoiParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity, Barrier bType,
		const std::string &name);
	PoiParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	PoiParams(ActorType *at,
		sf::Vector2i &pos,
		Barrier bType,
		const std::string &name,
		bool hasCameraProperties, float camZoom);
	PoiParams(ActorType *at,
		sf::Vector2i &pos);
	PoiParams(ActorType *at,
		std::ifstream &is);

	void Draw(sf::RenderTarget *target);
	//PoiParams( EditSession *edit );

	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();

	std::string name;
	static sf::Font *font;
	sf::Text nameText;

	bool hasCamProperties;
	float camZoom;

	sf::RectangleShape camRect;
	//void Draw( sf::RenderTarget *target );
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

	bool CanApply();
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

	bool CanApply();
	ActorParams *Copy();

	std::string shardStr;

};

struct RaceFightTargetParams : public ActorParams
{
	RaceFightTargetParams(ActorType *at,
		sf::Vector2i &pos);
	RaceFightTargetParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
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
		int bType, bool invinc, int spacing);
	BlockerParams(ActorType *at,
		sf::Vector2i &pos);
	BlockerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<int> &angleList);
	std::list<int> GetAngleList();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalChain();

	void SetParams();
	void SetPanelInfo();

	//std::list<int> angleList;
	bool armored;
	//sf::VertexArray *lines; //local pos

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

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
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool loop;
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

	bool CanApply();
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
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();
	std::list<sf::Vector2i> GetGlobalPath();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

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
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool loop;
	int speed;
	int swoopSpeed;
};

struct CrawlerParams : public ActorParams
{
	CrawlerParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		bool clockwise, float speed);
	CrawlerParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	CrawlerParams(ActorType *at,
		std::ifstream &is);

	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	ActorParams *Copy();
	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	int speed;
	int dist;
};

struct BasicTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	BasicTurretParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		double bulletSpeed,
		int framesWait);
	BasicTurretParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	BasicTurretParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	int framesWait;
};

struct FootTrapParams : public ActorParams
{
	FootTrapParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	FootTrapParams(ActorType *at, EditSession *edit);
	FootTrapParams(ActorType *at,
		std::ifstream &is);
	bool CanApply();
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

	bool CanApply();
	ActorParams *Copy();
};

struct BossCrawlerParams : public ActorParams
{
	BossCrawlerParams(ActorType *at,

		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	BossCrawlerParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

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

struct GravityFallerParams : public ActorParams
{
	GravityFallerParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int variation);
	GravityFallerParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GravityFallerParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	ActorParams *Copy();
	bool CanApply();
	void SetParams();
	void SetPanelInfo();

	int variation;
};

struct StagBeetleParams : public ActorParams
{
	StagBeetleParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		bool clockwise, float speed);
	//StagBeetleParams( 
	//	sf::Vector2i &pos, bool facingRight,
	//	float speed );
	StagBeetleParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	StagBeetleParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpWaitFrames);
	PoisonFrogParams(ActorType *at,
		TerrainPolygon *edgePolygon,
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

struct CurveTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	CurveTurretParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		double bulletSpeed,
		int framesWait,
		sf::Vector2i gravFactor,
		bool relativeGrav);
	ActorParams *Copy();
	CurveTurretParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	CurveTurretParams(ActorType *at,
		std::ifstream &is);
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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

struct BossBirdParams : public ActorParams
{
	BossBirdParams(ActorType *at,

		sf::Vector2i &pos);
	BossBirdParams(ActorType *at,
		std::ifstream &is);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void Draw(sf::RenderTarget *target);
	sf::VertexArray debugLines;
	void CreateFormation();
	int width;
	int height;
};


//w3
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
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

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

	bool CanApply();
	ActorParams *Copy();

	int moveSpeed;
	int bulletSpeed;
	int rhythmFrames;
};

struct BadgerParams : public ActorParams
{
	BadgerParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		int speed, int jumpStrength);

	BadgerParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	BadgerParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int speed;
	int jumpStrength;
};

struct CactusParams : public ActorParams
{
	CactusParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		int bulletSpeed, int rhythm,
		int amplitude);

	CactusParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);

	CactusParams(ActorType *at,
		std::ifstream &is);


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void CreateFormation();
	void Draw(sf::RenderTarget *target);
	int radius;
	sf::VertexArray debugLines;
};


//w4
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

	bool CanApply();
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

	bool CanApply();
	ActorParams *Copy();

	int moveFrames;
};

struct CheetahParams : public ActorParams
{
	/*CactusParams(
	TerrainPolygon *edgePolygon,
	int edgeIndex, double edgeQuantity,
	int bulletSpeed, int rhythm,
	int amplitude );*/

	CheetahParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	CheetahParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();
};

struct SpiderParams : public ActorParams
{
	SpiderParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		int speed);

	SpiderParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	SpiderParams(ActorType *at,
		std::ifstream &is);

	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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
		bool energized);
	RailParams(ActorType *at,
		sf::Vector2i &pos);
	RailParams(ActorType *at,
		std::ifstream &is);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<int> &angleList);
	std::list<int> GetAngleList();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalChain();

	void SetParams();
	void SetPanelInfo();

	//std::list<int> angleList;
	bool armored;
	//sf::VertexArray *lines; //local pos

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool energized;
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
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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

	bool CanApply();
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

	bool CanApply();
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

	bool CanApply();
	ActorParams *Copy();

	int speed;
};

struct OvergrowthParams : public ActorParams
{
	OvergrowthParams(ActorType *at,
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	OvergrowthParams(ActorType *at,
		std::ifstream &is);
	/*OvergrowthParams(
	TerrainPolygon *edgePolygon,
	int edgeIndex, double edgeQuantity );*/


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
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

	bool CanApply();
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

	bool CanApply();
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

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	void SetPath(
		std::list<sf::Vector2i> &globalPath);

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

	bool CanApply();
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
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
};




#endif