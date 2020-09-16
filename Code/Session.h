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
#include "ScoreDisplay.h"


struct GGPONonGameState;
struct SaveGameState;
struct GGPOSession;
struct GGPOPlayer;

struct QNode;
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
struct Gate;

struct Minimap;
struct AbsorbParticles;

struct Fader;
struct Swiper;
struct HUD;
struct AdventureHUD;
struct Barrier;

struct PoiInfo;
struct CameraShot;
struct MusicInfo;

struct ShardPopup;
struct DeathSequece;

struct SuperSequence;
struct BasicBossScene;
struct Sequence;
struct ShipEnterScene;
struct ShipExitScene;
struct CameraShotParams;
struct ShapeEmitter;
struct TopClouds;
struct PoiParams;
struct StorySequence;
struct GoalFlow;
struct Session;
struct GoalPulse;
struct Rain;
struct ScoreDisplay;
struct InputVisualizer;
struct PState;
struct GameMode;

#define TIMESTEP (1.0 / 60.0)

enum BossFightType
{
	FT_CRAWLER,
	FT_BIRD,
	FT_COYOTE,
	FT_CRAWLER2,
	FT_TIGER,
	FT_BIRD2,
	FT_GATOR,
	FT_SKELETON,
	FT_TIGER2,
	FT_BIRDCHASE,
	FT_BIRD3,
	FT_SKELETON2,
	BossFightType_Count
};

struct Session : TilesetManager, QuadTreeCollider
{
	const static int PLAYER_OPTION_BIT_COUNT = 32 * 8;
	const static int MAX_PLAYERS = 4;
	const static int MAX_BUBBLES = 5;
	const static int MAX_TOTAL_BUBBLES = MAX_PLAYERS * MAX_BUBBLES;

	const static int TERRAIN_WORLDS = 10;
	const static int MAX_TERRAINTEX_PER_WORLD = 10;
	const static int TOTAL_TERRAIN_TEXTURES =
		TERRAIN_WORLDS * MAX_TERRAINTEX_PER_WORLD;

	struct FlowHandler : RayCastHandler
	{
		Session *sess;
		void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	};

	struct FrameRateDisplay
	{
		bool showFrameRate;
		int frameRateCounter;
		int frameRateCounterWait;
		double frameRateTimeTotal;
		sf::Text frameRateText;
		FrameRateDisplay();
		void InitText(sf::Font &f);
		void Reset();
		void Update(double frameTime);
	};

	struct RunningTimerDisplay
	{
		bool showRunningTimer;
		sf::Text runningTimerText;

		void InitText(sf::Font &f);
		RunningTimerDisplay();
		void Reset();
	};

	enum SessionType
	{
		SESS_GAME,
		SESS_EDIT,
	};



	//mark these with GAME soon, since they are the queries for the game, not the editor
	enum QueryMode : int
	{
		QUERY_ENEMY,
		QUERY_BORDER,
		QUERY_SPECIALTERRAIN,
		QUERY_FLYTERRAIN,
		QUERY_INVERSEBORDER,
		QUERY_GATE,
		QUERY_ENVPLANT,
		QUERY_RAIL,
	};

	enum GameState
	{
		RUN,
		CUTPAUSE,
		CUTSCENE,
		PAUSE,
		RACEFIGHT_RESULTS,
		STORY,
		SEQUENCE,
		FROZEN,
		MAP
	};

	int timeSyncFrames;
	GameMode *gameMode;
	bool switchGameState;
	ShipEnterScene *shipEnterScene;
	ShipExitScene *shipExitScene;
	GameState gameState;
	HUD *hud;

	Fader *fader;
	Swiper *swiper;
	sf::RenderTexture *minimapTex;
	sf::RenderTexture *postProcessTex2;
	std::list<Barrier*> barriers;
	std::map<std::string, Barrier*> barrierMap;

	sf::RenderTexture *preScreenTex;
	int numGates;
	std::vector<Gate*> gates;
	std::vector<PolyPtr> allPolysVec;
	std::list<Enemy*> fullEnemyList;
	GateMarkerGroup *gateMarkers;
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
	float fBubbleRadiusSize[MAX_TOTAL_BUBBLES];
	sf::Vector2f fBubblePos[MAX_TOTAL_BUBBLES];
	float fBubbleFrame[MAX_TOTAL_BUBBLES];
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
	bool showNetStats;
	bool cutPlayerInput;
	MainMenu *mainMenu;
	std::vector<GCC::GCController> gcControllers;
	std::vector<Actor*> players;
	
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
	QuadTree * gateTree;
	QuadTree * enemyTree;
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
	sf::Vector2i playerOrigPos[MAX_PLAYERS];
	int numPolyShaders;
	sf::Shader *polyShaders;
	std::vector<Tileset*> ts_polyShaders;
	std::map<std::pair<int, int>, TerrainDecorInfo*> terrainDecorInfoMap;
	std::map<DecorType, DecorLayer*> decorLayerMap;
	GameSession *parentGame;

	Gate *gateList;
	int numBorders;
	QueryMode queryMode;
	sf::Rect<double> tempSpawnRect;
	PolyPtr polyQueryList;
	PolyPtr specialPieceList;
	PolyPtr flyTerrainList;
	int testGateCount;
	RailPtr railDrawList;
	Edge *inverseEdgeList;

	AbsorbParticles *absorbParticles;
	AbsorbParticles *absorbDarkParticles;
	AbsorbParticles *absorbShardParticles;

	std::list<Edge*> globalBorderEdges;
	bool drain;
	bool playerAndEnemiesFrozen;

	std::map<std::string, PoiInfo*> poiMap;
	std::vector<std::map<std::string, std::vector<PoiInfo*>>> bossNodeVectorMap;
	std::map<std::string, CameraShot*> cameraShotMap;
	sf::View view;
	MusicInfo *originalMusic;
	std::map<std::string, MusicInfo*> musicMap; //for sequences to change music? might be useless..
	bool goalDestroyed;
	sf::Vertex blackBorderQuads[4 * 2];
	PolyPtr inversePolygon;
	ShardPopup *shardPop;
	Sequence *getShardSeq;
	Sequence *deathSeq;
	BitField *shardsCapturedField;

	Sequence *activeSequence;
	Sequence *preLevelScene;
	Sequence *postLevelScene;
	TopClouds *topClouds;

	StorySequence *currStorySequence;
	GoalFlow *goalFlow;
	GoalPulse *goalPulse;

	ShapeEmitter *emitterLists[EffectLayer::Count];

	bool hasGoal;
	V2d goalPos;
	V2d goalNodePos;
	V2d goalNodePosFinal;

	std::string rayMode;
	V2d rayStart;
	V2d rayEnd;
	Edge *rcEdge;
	double rcQuantity;
	Edge *rayIgnoreEdge;
	Edge *rayIgnoreEdge1;

	FlowHandler flowHandler;
	Rain *rain;
	ScoreDisplay *scoreDisplay;
	InputVisualizer *inputVis;


	FrameRateDisplay frameRateDisplay;
	RunningTimerDisplay runningTimerDisplay;
	int totalFramesBeforeGoal;

	GGPOSession *ggpo;

	SuperSequence *superSequence;

	PState *playerSimState;
	Actor *currSuperPlayer;

	static Session *GetSession();

	void ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
		V2d &pos, float startAngle = 0);
	void CollectKey();
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
	virtual void ProcessPlayerOptions() {}
	virtual void ProcessTerrain(PolyPtr poly) {}
	virtual void ProcessAllTerrain() {}
	virtual void ProcessBGTerrain(PolyPtr poly) {}
	virtual void ProcessRail(RailPtr rail) {}
	virtual bool ReadActors(std::ifstream &is);
	virtual void ProcessActor(ActorPtr a) {}
	virtual void ProcessAllActors() {}
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
	void CloseOffLimitZones();
	void CleanupZones();
	void DrawZones(sf::RenderTarget *target);
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
	void PlayerApplyHit(int index, HitboxInfo *hi, 
		Actor *attacker, int res, V2d &pos );
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
	void DrawPlayers(sf::RenderTarget *target);
	void DrawPlayerWires(sf::RenderTarget *target);
	void UpdatePlayerWireQuads();
	bool ReadFile();
	bool ReadDecorImagesFile();
	bool ReadDecorInfoFile(int tWorld, int tVar);
	bool ReadHeader(std::ifstream &is);
	bool ReadDecor(std::ifstream &is);
	bool ReadPlayersStartPos(std::ifstream &is);
	bool ReadPlayerOptions(std::ifstream &is);
	bool ReadTerrain(std::ifstream &is);
	bool ReadSpecialTerrain(std::ifstream &is);
	bool ReadBGTerrain(std::ifstream &is);
	bool ReadRails(std::ifstream &is);
	bool ReadGates(std::ifstream &is);
	void AllocatePolyShaders(int numPolyTypes);
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
	void QueryBorderTree(sf::Rect<double>&rect);
	void QueryGateTree(sf::Rect<double>&rect);
	void QuerySpecialTerrainTree(sf::Rect<double>&rect);
	void QueryFlyTerrainTree(sf::Rect<double>&rect);

	void DrawAllMapWires(
		sf::RenderTarget *target);
	void DrawColoredMapTerrain(
		sf::RenderTarget *target,
		sf::Color &c);
	void EnemiesCheckedMiniDraw(
		sf::RenderTarget *target,
		sf::FloatRect &rect);
	void DrawPlayersMini(sf::RenderTarget *target);
	void HandleEntrant(QuadTreeEntrant *qte);
	void TrySpawnEnemy(QuadTreeEntrant *qte);
	void TryAddPolyToQueryList(QuadTreeEntrant *qte);
	void TryAddSpecialPolyToQueryList(QuadTreeEntrant *qte);
	void TryAddFlyPolyToQueryList(QuadTreeEntrant *qte);
	void TryAddGateToQueryList(QuadTreeEntrant *qte);
	void TryAddRailToQueryList(QuadTreeEntrant *qte);
	void SetQueriedInverseEdge(QuadTreeEntrant *qte);
	void TryActivateQueriedEnvPlant(QuadTreeEntrant *qte);
	void SetupHUD();
	void UpdateHUD();
	void DrawHUD(sf::RenderTarget *target);
	void HitlagUpdate();
	void SetupAbsorbParticles();
	void ResetAbsorbParticles();
	void DrawEnemies(sf::RenderTarget *target);
	void DrawHitEnemies(sf::RenderTarget *target);
	void ResetZones();
	void UpdateZones();
	void SetNumGates(int nGates);
	void LockGate(Gate *g);
	void UnlockGate(Gate *g);
	void DrawGates(sf::RenderTarget *target);
	void ResetGates();
	void ResetBarriers();
	void AddBarrier(XBarrierParams *xbp);
	void SetGameSessionState(int s);
	virtual int GetGameSessionState();
	void Fade(bool in, int frames, sf::Color c, bool skipKin = false);
	void CrossFade(int fadeOutFrames,
		int pauseFrames, int fadeInFrames,
		sf::Color c, bool skipKin = false);
	bool IsFading();
	void ClearFade();
	void TotalDissolveGates(int gCat);
	void ReverseDissolveGates(int gCat);
	void SetDrainOn(bool d);
	void RemoveAllEnemies();
	void FreezePlayerAndEnemies(bool freeze);
	void SetGlobalBorders();
	void SetupGlobalBorderQuads(bool *blackBorder, bool &topBorderOn);
	void CleanupGlobalBorders();
	void CleanupBarriers();
	void QueryToSpawnEnemies();
	void DrawBlackBorderQuads(sf::RenderTarget *target);
	void TryCreateShardResources();
	virtual bool IsShardCaptured(int sType);
	void SetupShardsCapturedField();
	void SetActiveSequence(Sequence *activeSeq);
	void ActiveSequenceUpdate();
	void AddEmitter(ShapeEmitter *emit,
		EffectLayer layer);
	void UpdateEmitters();
	void ClearEmitters();
	void DrawActiveSequence(EffectLayer layer,
		sf::RenderTarget *target);
	void DrawEmitters(EffectLayer layer, sf::RenderTarget *target);
	void SetupDeathSequence();
	void DrawDyingPlayers(sf::RenderTarget *target);
	void DrawKinOverFader(sf::RenderTarget *target);
	void UpdateBarriers();
	void TriggerBarrier(Barrier *b);
	void SetupBarrierScenes();
	void SetupEnemyZoneSprites();
	void CleanupCameraShots();
	void AddCameraShot(CameraShotParams *csp);
	void AddPoi(PoiParams *pp);
	void AddBossNode(const std::string &nodeTypeName, PoiParams *pp);
	std::map<std::string, std::vector<PoiInfo*>> & GetBossNodeVectorMap( int w );
	std::vector<PoiInfo*> & GetBossNodeVector(int w, const std::string &name);
	void CleanupBossNodes();
	void CleanupPoi();
	void CleanupTopClouds();
	void DrawTopClouds(sf::RenderTarget *target);
	void UpdateTopClouds();
	void CleanupShipEntrance();
	void CleanupShipExit();
	void UpdateEnemiesPreFrameCalculations();
	void UpdateEnemiesPrePhysics();
	void UpdatePhysics();
	void UpdateEnemiesPostPhysics();
	virtual bool UpdateRunModeBackAndStartButtons() = 0;
	virtual void DrawSpecialTerrain(sf::RenderTarget *target) = 0;
	virtual void DrawTerrain(sf::RenderTarget *target) = 0;
	virtual void DrawFlyTerrain(sf::RenderTarget *target) = 0;
	void DrawStoryLayer(EffectLayer ef, sf::RenderTarget *target);
	void DrawGateMarkers(sf::RenderTarget *target);
	virtual void DrawDecor(EffectLayer ef, sf::RenderTarget *target) = 0;
	void LayeredDraw(EffectLayer ef, sf::RenderTarget *target);
	void SetupGoalFlow();
	void CleanupGoalFlow();
	void CleanupGoalPulse();
	void DrawGoalFlow(sf::RenderTarget *target);
	void UpdateGoalFlow();
	void UpdateGoalPulse();
	void SetupGoalPulse();
	void DrawGoalPulse(sf::RenderTarget *target);
	void CleanupRain();
	void SetupRain();
	void UpdateRain();
	void DrawRain(sf::RenderTarget *target);
	virtual void DrawRails(sf::RenderTarget *target) = 0;
	virtual void DrawRaceFightScore(sf::RenderTarget *target) {}
	virtual void DrawScoreDisplay(sf::RenderTarget *target);
	void DrawFrameRate(sf::RenderTarget *target);
	void DrawRunningTimer(sf::RenderTarget *target);
	void DrawInputVis(sf::RenderTarget *target);
	void UpdateInputVis();
	void SetupInputVis();
	virtual void DrawReplayGhosts(sf::RenderTarget *target) {}
	void DrawGame(sf::RenderTarget *target);
	void UpdateRunningTimerText();
	virtual void UpdateRaceFightScore() {}
	virtual void UpdateEnvPlants() {}
	virtual void UpdateEnvShaders() {}
	virtual void UpdateCamera() = 0;
	virtual void UpdateSoundNodeLists();
	void UpdateScoreDisplay();
	virtual bool RunPreUpdate() { return true; }
	virtual bool RunPostUpdate() { return true; }
	void ActiveStorySequenceUpdate();
	virtual void TryToActivateBonus() {}
	virtual void RecPlayerRecordFrame() {}
	virtual void RepPlayerUpdateInput() {}
	virtual void RecGhostRecordFrame() {}
	virtual void UpdateReplayGhostSprites() {}
	bool RunGameModeUpdate();
	bool FrozenGameModeUpdate();
	bool SequenceGameModeUpdate();
	void DrawGameSequence(sf::RenderTarget *target);
	virtual void SequenceGameModeRespondToGoalDestroyed() = 0;
	void CleanupGateMarkers();
	void ResetEnemies();
	void rResetEnemies(QNode *node);
	bool PlayerIsMovingLeft(int index = 0);
	bool PlayerIsMovingRight(int index = 0);
	bool PlayerIsFacingRight(int index = 0);
	void CleanupPreLevelScene();
	void CleanupPostLevelScene();
	void SetupScoreDisplay();
	void CleanupScoreDisplay();
	void EndLevel();
	
	GGPONonGameState *ngs;
	SaveGameState *currSaveState;

	int GetSaveDataSize();
	//SaveGameState *saveStates[10];
	//bool usedSaveState[10];
	GGPOPlayer *ggpoPlayers;
	bool GGPORunGameModeUpdate();
	void GGPORunFrame();
	bool SaveState(unsigned char **buffer,
		int *len, int *checksum, int frame);
	bool LoadState(unsigned char *buffer, int len);
	virtual void RestartGame() = 0;
	int GetGameMode();
	bool IsMapVersionNewerThanOrEqualTo(int ver1, int ver2);
	AdventureHUD *GetAdventureHUD();
	void SetupSuperSequence();
	void CleanupSuperSequence();
	void DrawPlayerShields(sf::RenderTarget *target);
	void SetupGameMode();
	void ForwardSimulatePlayer(int index, int frames);
	void RevertSimulatedPlayer(int index);
	sf::CircleShape testSimCircle;
};

#endif