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


struct Session : TilesetManager
{
	enum SessionType
	{
		SESS_GAME,
		SESS_EDIT,
	};



	//cleaning up load stuff:
	void SetupHitboxManager();
	void SetupSoundManager();
	void SetupSoundLists();

	//timeslow stuff
	float *fBubbleRadiusSize;
	sf::Vector2f *fBubblePos;
	float *fBubbleFrame;
	void SetupTimeBubbles();

	SessionType sessType;
	bool IsSessTypeGame();
	bool IsSessTypeEdit();

	//stuff to test editor enemy loading in game

	void SetupEnemyTypes();
	void SetupEnemyType(ParamsInfo &pi);
	std::map<std::string, ActorGroup*> groups;
	std::map<std::string, ActorType*> types;

	std::list<ParamsInfo> worldEnemyNames[8];
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
	std::list<ParamsInfo> extraEnemyNames;

	void AddGeneralEnemies();
	void AddW1Enemies();
	void AddW2Enemies();
	void AddW3Enemies();
	void AddW4Enemies();
	void AddW5Enemies();
	void AddW6Enemies();

	virtual PolyPtr GetPolygon(int index) = 0;
	virtual RailPtr GetRail(int index) //change this to abstract later when gamesession has rails again
	{
		return NULL;
	}



	//--------------------------------------



	//stuff I have to add in for enemies. Might have to adjust

	int keyFrame;
	int numTotalKeys;
	SoundManager *soundManager;
	sf::SoundBuffer *GetSound(const std::string &name);
	SoundNode *ActivateSoundAtPos(V2d &pos, sf::SoundBuffer *buffer, bool loop = false);
	SoundNode *ActivateSound(sf::SoundBuffer *buffer, bool loop = false);
	SoundNode *ActivatePauseSound(sf::SoundBuffer *buffer, bool loop = false);
	SoundNodeList * soundNodeList;
	SoundNodeList * pauseSoundNodeList;
	int GetPauseFrames();
	int pauseFrames;
	sf::Rect<double> screenRect;

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
	const static int MAX_EFFECTS = 100;
	std::vector<Enemy*> effectListVec;
	//Enemy *effectLists[EffectLayer::Count];
	BasicEffect *inactiveEffects;
	std::vector<BasicEffect> allEffectVec;
	void AllocateEffects();

	sf::View uiView;

	void Pause(int frames);

	virtual void CollectKey() {}
	void PlayerConfirmEnemyNoKill(Enemy *, int index = 0);
	void PlayerConfirmEnemyKill(Enemy *, int index = 0);
	void PlayerApplyHit(HitboxInfo *hi, int index = 0);
	void PlayerHitNexus(int index = 0);
	void PlayerHitGoal(int index = 0);
	void SetPlayersGameMode();

	void KillAllEnemies();

	
	virtual void ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
		V2d &pos, float startAngle = 0) {}

	Camera cam;

	V2d GetPlayerKnockbackDirFromVel(int index = 0);
	V2d GetPlayerPos(int index = 0);
	V2d GetPlayerTrueVel(int index = 0);
	void PlayerAddActiveComboObj(ComboObject *, int index = 0);
	void PlayerRemoveActiveComboer(ComboObject *, int index = 0);

	Enemy *activeEnemyList;
	Enemy *activeEnemyListTail;
	Enemy *inactiveEnemyList;

	void AddEnemy(Enemy *e);
	void RemoveEnemy(Enemy *e);

	int totalNumberBullets;
	sf::Vertex *bigBulletVA;
	Tileset *ts_basicBullets;

	void CreateBulletQuads();
	void DrawBullets(sf::RenderTarget *target);
	//-------------------------



	Session( SessionType p_sessType, const boost::filesystem::path &p_filePath);
	virtual ~Session();
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

	static Session *GetSession();
	//static bool IsSessionTypeEdit();
	//static bool IsSessionTypeGame();

	virtual void ProcessDecorFromFile(const std::string &name,
		int tile) {}
	bool ReadHeader( std::ifstream &is );
	virtual void ProcessHeader() {}

	bool ReadDecor(std::ifstream &is);
	virtual void ProcessDecorSpr(const std::string &name,
		Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
		float rotation, sf::Vector2f &scale) {}
	virtual void ProcessAllDecorSpr() {}

	bool ReadPlayerStartPos(std::ifstream &is);
	virtual void ProcessPlayerStartPos() {}

	bool ReadTerrain(std::ifstream &is);
	bool ReadTerrainGrass(std::ifstream &is, PolyPtr poly );
	virtual void ProcessTerrain( PolyPtr poly ){}
	virtual void ProcessAllTerrain() {}

	bool ReadSpecialTerrain(std::ifstream &is);
	virtual void ProcessSpecialTerrain(PolyPtr poly) {}

	bool ReadBGTerrain(std::ifstream &is);
	virtual void ProcessBGTerrain(PolyPtr poly) {}

	bool ReadRails(std::ifstream &is);
	virtual void ProcessRail(RailPtr rail) {}

	virtual bool ReadActors(std::ifstream &is);
	virtual void ProcessActor(ActorPtr a) {}
	virtual void ProcessAllActors() {}

	bool ReadGates(std::ifstream &is);
	virtual void SetNumGates(int numGates) {}
	virtual void ProcessGate(int gType,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex ) {}

	void AllocatePolyShaders(int numPolyTypes );
	//virtual void ProcessActor( const std::string &groupName, int numActors,
	//	const std::string &actorType ) {}
//	virtual void ProcessActor(ActorPtr actor);
//	virtual void ProcessActorGroup(const std::string &groupName, int numActors);
	bool LoadPolyShader(int index, int matWorld, int matVariation);

	virtual int Run() = 0;

	bool OneFrameModeUpdate();
	bool skipped;
	bool oneFrameMode;


	bool cutPlayerInput;
	MainMenu *mainMenu;

	std::vector<GCC::GCController> gcControllers;

	//void TestLoad(); //change this to Load once Load has been changed to work for both sessions

	std::vector<Actor*> players;

	const static int MAX_PLAYERS = 4;

	HitboxManager *hitboxManager;

	Background *background;
	//std::list<ScrollingBackground*> scrollingBackgrounds;

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

	std::map<std::pair<int,int>,TerrainDecorInfo*> terrainDecorInfoMap;
	std::map<DecorType, DecorLayer*> decorLayerMap;


	void SetPlayerInputOn(bool on);
	void UpdateDecorLayers();
	virtual void UpdateDecorSprites();

	GameSession *parentGame;
	void SetParentGame(GameSession *session);

};

#endif