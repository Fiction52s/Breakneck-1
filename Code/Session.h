#ifndef __SESSION_H__
#define __SESSION_H__

#include <SFML/Audio.hpp>
#include "Input.h"
#include "Tileset.h"
#include "Physics.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include "ISelectable.h"
#include "DecorTypes.h"
#include "EffectLayer.h"
#include "Camera.h"

//#include "ActorParamsBase.h"
#include "PositionInfo.h"
#include "EditorActors.h"
#include "Enemy_BasicEffect.h"
#include "BitField.h"


struct SoundManager;
struct SoundNode;


struct Actor;
struct MainMenu;
struct QuadTree;
struct MapHeader;
struct DecorLayer;
struct Background;
struct HitboxManager;

struct TerrainDecorInfo;

struct SoundNodeList;
struct ComboObject;

struct Enemy;

struct GameSession;

struct ZoneNode;
struct Zone;
struct ZonePropertiesObj;

struct GateMarkerGroup;
struct KeyMarker;
struct Gate;

struct Session : TilesetManager
{
	const static int PLAYER_OPTION_BIT_COUNT = 32 * 8;

	enum SessionType
	{
		SESS_GAME,
		SESS_EDIT,
	};

	int numGates;
	std::vector<Gate*> gates;
	std::vector<PolyPtr> allPolysVec;
	std::list<Enemy*> fullEnemyList;
	GateMarkerGroup *gateMarkers;
	KeyMarker *keyMarker;
	ZoneNode *zoneTree;
	ZoneNode *currentZoneNode;
	Zone *zoneTreeStart;
	Zone *zoneTreeEnd;
	Zone *activatedZoneList;
	Zone *currentZone;
	Zone *originalZone;
	std::list<Zone*> zones;
	std::list<ZonePropertiesObj*> zoneObjects;
	BitField playerOptionsField;
	//timeslow stuff
	float *fBubbleRadiusSize;
	sf::Vector2f *fBubblePos;
	float *fBubbleFrame;
	SessionType sessType;
	std::map<std::string, ActorGroup*> groups;
	std::map<std::string, ActorType*> types;
	std::list<ParamsInfo> worldEnemyNames[8];
	std::list<ParamsInfo> extraEnemyNames;
	int keyFrame;
	int numTotalKeys;
	SoundManager *soundManager;
	SoundNodeList * soundNodeList;
	SoundNodeList * pauseSoundNodeList;
	int pauseFrames;
	sf::Rect<double> screenRect;
	const static int MAX_EFFECTS = 100;
	std::vector<Enemy*> effectListVec;
	BasicEffect *inactiveEffects;
	std::vector<BasicEffect> allEffectVec;
	sf::View uiView;
	Camera cam;
	Enemy *activeEnemyList;
	Enemy *activeEnemyListTail;
	Enemy *inactiveEnemyList;
	int totalNumberBullets;
	sf::Vertex *bigBulletVA;
	Tileset *ts_basicBullets;
	bool skipped;
	bool oneFrameMode;
	bool showDebugDraw;
	bool cutPlayerInput;
	MainMenu *mainMenu;
	std::vector<GCC::GCController> gcControllers;
	std::vector<Actor*> players;
	const static int MAX_PLAYERS = 4;
	HitboxManager *hitboxManager;
	Background *background;
	QuadTree * terrainTree;
	QuadTree *specialTerrainTree;
	QuadTree *flyTerrainTree;
	QuadTree *railEdgeTree;
	QuadTree *barrierTree;
	QuadTree *borderTree;
	QuadTree *grassTree;
	QuadTree *activeItemTree;
	QuadTree *staticItemTree;
	int substep;
	double currentTime;
	double accumulator;
	sf::Clock gameClock;
	Collider collider;
	int totalGameFrames;
	boost::filesystem::path filePath;
	std::string filePathStr;
	MapHeader *mapHeader;
	std::map<std::string, Tileset*> decorTSMap;
	sf::RenderWindow *window;
	sf::Vector2i playerOrigPos;
	int numPolyShaders;
	sf::Shader *polyShaders;
	std::vector<Tileset*> ts_polyShaders;
	std::map<std::pair<int, int>, TerrainDecorInfo*> terrainDecorInfoMap;
	std::map<DecorType, DecorLayer*> decorLayerMap;
	GameSession *parentGame;

	static Session *GetSession();

	virtual void ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
		V2d &pos, float startAngle = 0) {}
	virtual void CollectKey() {}
	virtual PolyPtr GetPolygon(int index) = 0;
	virtual RailPtr GetRail(int index) //change this to abstract later when gamesession has rails again
	{
		return NULL;
	}
	virtual void ProcessDecorFromFile(const std::string &name,
		int tile) {}
	virtual void ProcessHeader() {}
	virtual void ProcessDecorSpr(const std::string &name,
		Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
		float rotation, sf::Vector2f &scale) {}
	virtual void ProcessAllDecorSpr() {}
	virtual void ProcessPlayerStartPos() {}
	virtual void ProcessPlayerOptions() {}
	virtual void ProcessTerrain(PolyPtr poly) {}
	virtual void ProcessAllTerrain() {}
	virtual void ProcessBGTerrain(PolyPtr poly) {}
	virtual void ProcessRail(RailPtr rail) {}
	virtual bool ReadActors(std::ifstream &is);
	virtual void ProcessActor(ActorPtr a) {}
	virtual void ProcessAllActors() {}
	virtual void SetNumGates(int numGates) {}
	virtual void ProcessGate(int gCat,
		int gVar,
		int numToOpen,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex) {}
	virtual void ProcessSpecialTerrain(PolyPtr poly) {}
	virtual int Run() = 0;
	virtual void DebugDraw(sf::RenderTarget *target);
	virtual void UpdateDecorSprites();


	Session(SessionType p_sessType, const boost::filesystem::path &p_filePath);
	virtual ~Session();
	void CleanupGates();
	void SetupGateMarkers();
	void SetupKeyMarker();
	void CloseOffLimitZones();
	void CleanupZones();
	void DrawZones( sf::RenderTarget *target );
	void CreateZones();
	void SetupZones();
	void ActivateZone(Zone * z, bool instant = false);
	void SetPlayerOptionField(int pIndex);
	void SetupHitboxManager();
	void SetupSoundManager();
	void SetupSoundLists();
	void SetupTimeBubbles();
	bool IsSessTypeGame();
	bool IsSessTypeEdit();
	void SetupEnemyTypes();
	void SetupEnemyType(ParamsInfo &pi);
	void AddWorldEnemy(const std::string &name, 
		int w,
		EnemyCreator *p_enemyCreator,
		ParamsCreator *p_paramsCreator,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		bool p_canBeAerial,
		bool p_canBeGrounded,
		bool p_canBeRailGrounded,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddBasicGroundWorldEnemy(const std::string &name, int w,
		EnemyCreator *p_enemyCreator,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddBasicRailWorldEnemy(const std::string &name, int w,
		EnemyCreator *p_enemyCreator,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddBasicAerialWorldEnemy(const std::string &name, int w,
		EnemyCreator *p_enemyCreator,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddExtraEnemy(const std::string &name,
		EnemyCreator *p_enemyCreator,
		ParamsCreator *p_paramsCreator,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		bool p_canBeAerial,
		bool p_canBeGrounded,
		bool p_canBeRailGrounded,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddGeneralEnemies();
	void AddW1Enemies();
	void AddW2Enemies();
	void AddW3Enemies();
	void AddW4Enemies();
	void AddW5Enemies();
	void AddW6Enemies();
	sf::SoundBuffer *GetSound(const std::string &name);
	SoundNode *ActivateSoundAtPos(V2d &pos, sf::SoundBuffer *buffer, bool loop = false);
	SoundNode *ActivateSound(sf::SoundBuffer *buffer, bool loop = false);
	SoundNode *ActivatePauseSound(sf::SoundBuffer *buffer, bool loop = false);
	int GetPauseFrames();
	BasicEffect * ActivateEffect(
		EffectLayer layer,
		Tileset *ts,
		V2d pos,
		bool pauseImmune,
		double angle,
		int frameCount,
		int animationFactor,
		bool right,
		int startFrame = 0,
		float depth = 1.f);
	void DeactivateEffect(BasicEffect *be);
	void DrawEffects(EffectLayer layer, sf::RenderTarget *target);
	void ClearEffects();
	void UpdateEffects(bool pauseImmuneOnly = false);
	void AllocateEffects();
	void Pause(int frames);
	void PlayerConfirmEnemyNoKill(Enemy *, int index = 0);
	void PlayerConfirmEnemyKill(Enemy *, int index = 0);
	void PlayerApplyHit(HitboxInfo *hi, int index = 0);
	void PlayerHitNexus(int index = 0);
	void PlayerHitGoal(int index = 0);
	void SetPlayersGameMode();
	void KillAllEnemies();
	V2d GetPlayerKnockbackDirFromVel(int index = 0);
	V2d GetPlayerPos(int index = 0);
	V2d GetPlayerTrueVel(int index = 0);
	void PlayerAddActiveComboObj(ComboObject *, int index = 0);
	void PlayerRemoveActiveComboer(ComboObject *, int index = 0);
	void AddEnemy(Enemy *e);
	void RemoveEnemy(Enemy *e);
	void CreateBulletQuads();
	void DrawBullets(sf::RenderTarget *target);
	Actor *GetPlayer(int i);
	ControllerState &GetPrevInput(int index);
	ControllerState &GetCurrInput(int index);
	ControllerState &GetPrevInputUnfiltered(int index);
	ControllerState &GetCurrInputUnfiltered(int index);
	GameController &GetController(int index);
	void UpdatePlayerInput(int index);
	void UpdateAllPlayersInput();
	void UpdateControllers();
	void UpdateControllersOneFrameMode();
	bool IsKeyPressed(int k);
	bool IsMousePressed(int m);
	void DrawPlayers(sf::RenderTarget *target );
	void DrawPlayerWires(sf::RenderTarget *target);
	void UpdatePlayerWireQuads();
	bool ReadFile();
	bool ReadDecorImagesFile();
	bool ReadDecorInfoFile( int tWorld, int tVar );
	bool ReadHeader( std::ifstream &is );
	bool ReadDecor(std::ifstream &is);
	bool ReadPlayerStartPos(std::ifstream &is);
	bool ReadPlayerOptions(std::ifstream &is);
	bool ReadTerrain(std::ifstream &is);
	bool ReadTerrainGrass(std::ifstream &is, PolyPtr poly );
	bool ReadSpecialTerrain(std::ifstream &is);
	bool ReadBGTerrain(std::ifstream &is);
	bool ReadRails(std::ifstream &is);
	bool ReadGates(std::ifstream &is);
	void AllocatePolyShaders(int numPolyTypes );
	bool LoadPolyShader(int index, int matWorld, int matVariation);
	bool OneFrameModeUpdate();
	void DebugDrawActors(sf::RenderTarget *target);
	void SetPlayerInputOn(bool on);
	void UpdateDecorLayers();
	void SetParentGame(GameSession *session);
	void UpdateGates();
	void UpdatePlayersInHitlag();
	void UpdatePlayersPrePhysics();
	void UpdatePlayersPostPhysics();
};

#endif