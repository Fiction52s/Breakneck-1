#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <list>
#include "Mover.h"
#include "Movement.h"
#include "EffectLayer.h"
#include <SFML/Audio.hpp>
#include "ObjectProperties.h"
#include "ActorParams.h"
#include "SoundManager.h"

struct Wire;
struct Zone;
struct Tileset;
struct Actor;
struct Launcher;
struct Shield;
struct SpecterArea;
struct SpecterTester;
struct Session;
struct SummonGroup;

struct ActorParams;

struct HitParams
{
	HitParams()
	{
		canBeHit = false;
		flashFrames = 0;
		speedBar = 0;
		charge = 0;
		maxHealth = 0;
		cuttable = false;
	}

	void Set(int p_flashFrames, double p_speedBar,
		int p_charge, int p_maxHealth, bool p_cuttable = true)
	{
		canBeHit = true;
		flashFrames = p_flashFrames;
		speedBar = p_speedBar;
		charge = p_charge;
		maxHealth = p_maxHealth;
		cuttable = p_cuttable;
	}

	bool canBeHit;
	int flashFrames;
	double speedBar;
	int charge;
	int maxHealth;
	bool cuttable;
};


struct StoredEnemyData
{
	SlowableObject slowableObjectData;
	HittableObject hittableObjectData;

	//Enemy
	PositionInfo currPosInfo;
	bool facingRight;
	int action;
	int frame;
	bool active;

	Enemy *prev;
	Enemy *next;
	int pauseFrames;
	bool pauseFramesFromAttacking;
	bool dead;
	bool spawned;

	SurfaceMover surfaceMoverData;
	GroundMover groundMoverData;

	int currHitboxFrame;
	int currHurtboxFrame;

	CollisionBody *currHitboxes;
	CollisionBody *currHurtboxes;

	Shield *currShield;

	double scale;

	void Print();

};

struct Enemy : QuadTreeCollider, QuadTreeEntrant, 
	SlowableObject, HittableObject
{
	static int bloodLengths[8];

	virtual int GetNumEnergyAbsorbParticles();
	virtual int GetNumDarkAbsorbParticles();
	virtual int GetNumShardAbsorbParticles();

	virtual int GetNumStoredBytes() { return 0; }
	virtual void StoreBytes(unsigned char *bytes) {
		assert(0);
	}
	virtual void SetFromBytes(unsigned char *bytes) {}
	void StoreBasicEnemyData(StoredEnemyData &ed);
	void SetBasicEnemyData(StoredEnemyData &ed);
	virtual bool CountsForEnemyGate() { return true; }

	int summonFrame;
	const static int summonDuration;

	const static int minSubstepToCheckHits;

	bool facingRight;
	PositionInfo currPosInfo;
	int action;
	int pauseFrames;
	bool pauseBeganThisFrame;
	bool pauseFramesFromAttacking;
	int frame;
	bool dead;
	bool active;
	SummonGroup *summonGroup;
	int currHitboxFrame;
	int currHurtboxFrame;
	std::vector<Launcher*> launchers;
	CuttableObject *cutObject;
	HitboxInfo *hitboxInfo;
	SoundInfo *genericDeathSound;
	SoundInfo *finalDeathSound;
	SoundInfo *keyDeathSound;
	SoundInfo *keyUnlockDeathSound;
	CollisionBody hurtBody;
	CollisionBody hitBody;
	int numLaunchers;
	sf::Sprite zonedSprite;
	SurfaceMover *surfaceMover;
	GroundMover *groundMover;
	Shield *currShield;
	ComboObject *comboObj;
	bool highResPhysics;
	V2d groundOffset;
	bool origFacingRight;
	ActorParams *editParams;
	Session *sess;
	HitParams hitParams;
	PositionInfo startPosInfo;
	std::vector<int> actionLength;
	std::vector<int> animFactor;
	bool keyShaderLoaded;
	int editLoopAction;
	int editIdleAction;
	int editIdleFrame;
	SpecterTester *specterTester;
	double numPhysSteps;
	Enemy *prev;
	Enemy *next;
	bool spawned;
	sf::Rect<double> spawnRect;
	EnemyType type;
	Zone *zone;
	bool suppressMonitor;
	bool hasMonitor;
	sf::Shader keyShader;
	sf::Shader hurtShader;
	sf::Sprite keySprite;
	sf::Sprite sprite;
	Tileset *ts_hitSpack;
	Tileset *ts_killSpack;
	Tileset *ts_blood;
	Tileset *ts_key;
	Tileset *ts_zoned;
	sf::Color keyColor;
	int world;
	Enemy *tempPrev;
	Enemy *tempNext;
	int level;
	double scale;
	int maxHealth;
	std::string name;
	int playerIndex; //-1 for all players
	CollisionBody *currHitboxes;
	CollisionBody *currHurtboxes;
	

	Enemy(EnemyType t, ActorParams *ap);
	Enemy(EnemyType t, int w);
	void OnCreate( ActorParams *ap,
		int w );


	void SetPlayerWorld(int p);

	Tileset * GetTileset(const std::string &s,
		int tileWidth,
		int tileHeight);
	Tileset *GetSizedTileset(const std::string &s);
	SoundInfo * GetSound(const std::string &s);
	void SetSummonGroup(SummonGroup *p_summonGroup);
	bool IsSummoning();
	virtual ~Enemy();
	virtual void UpdatePreFrameCalculations() {}
	void CheckPlayerInteractions( int substep, int i );
	void UpdateKeySprite();
	void SetKey();
	virtual int GetNumSimulationFramesRequired() { return 0; }
	double PlayerDistX( int index = 0);
	double PlayerDistY( int index = 0);
	double PlayerDist(int index = 0);
	double PlayerDistSqr(int index = 0);
	V2d PlayerDir( V2d myOffset,
		V2d playerOffset,
		int index = 0);
	V2d PlayerDir(int index = 0);
	
	bool IsGoalType();
	virtual void AddToGame();
	virtual bool IsHitFacingRight() { return facingRight; }
	virtual void UpdateSpriteFromParams(ActorParams *ap);
	virtual void AddToWorldTrees() {}
	virtual void UpdateFromParams(ActorParams *ap,
		int numFrames );
	virtual void UpdateParamsSettings();
	virtual void UpdateOnPlacement(ActorParams *ap);
	void UpdateOnEditPlacement() {UpdateOnPlacement(editParams);}
	void UpdateFromEditParams(int numFrames){UpdateFromParams(editParams, numFrames);}
	virtual int SetLaunchersStartIndex(int ind);
	virtual void UpdatePath() {}
	void SetNumLaunchers(int num);
	virtual void AddChildrenToSession( Session *s) {}
	virtual void RemoveChildrenFromSession(Session *s) {}
	virtual void SetLevel(int p_level) { level = p_level; }
	virtual void SetSpawnRect();
	void SetCurrPosInfo(PositionInfo &pi);
	V2d GetPosition();
	sf::Vector2f GetPositionF();
	bool SetHitParams();
	void SetOffGroundHeight(double h);
	void SetGroundOffset(double x);
	V2d AlongGroundDir();
	void CreateSurfaceMover(PositionInfo &pi,
		double rad, SurfaceMoverHandler *handler);
	void CreateGroundMover(PositionInfo &pi,
		double rad, bool steeps,
		GroundMoverHandler *handler);
	double GetGroundedAngleRadians();
	void SetNumActions( int num );
	int GetEditIdleLength();
	virtual void SetActionEditLoop();
	void SetEditorActions(
		int editLoopAction = 0, int editIdleAction=0,
		int editIdleFrame=0);
	void DefaultHitboxesOn(int hFrame = 0);
	void DefaultHurtboxesOn(int hFrame = 0);
	void HurtboxesOff();
	void HitboxesOff();
	virtual void SyncCutObject();
	virtual sf::FloatRect GetAABB(){ return sprite.getGlobalBounds();}
	virtual void InitOnRespawn() {}
	virtual void SetupResources() {}
	virtual void SetupDebugResources() {}
	static bool ReadBool(std::ifstream &is,
		bool &b);
	static bool ReadPath(std::ifstream &is,
		int &pLen, std::vector<sf::Vector2i> &localPath);
	virtual void UpdatePreLauncherPhysics() {}
	virtual void PlayDeathSound();
	void PlayKeyDeathSound();
	virtual int GetNumCamPoints() { return 1; }
	virtual V2d GetCamPoint(int index) { return GetPosition(); }
	virtual void ComboHit();
	virtual bool CanComboHit(Enemy *e);
	virtual void ComboKill( Enemy *e );
	virtual void SetZone(Zone *p_zone) { zone = p_zone; }
	
	static void SyncSpriteInfo(sf::Sprite &dest, sf::Sprite &source);
	void SetHitboxes(CollisionBody *cb, int frame = 0);
	void ClearHitboxes() { 
		currHitboxes = NULL;
		currHitboxFrame = -1;
	}
	void ClearHurtboxes() {
		currHurtboxes = NULL;
		currHurtboxFrame = -1;
	}
	void SetHurtboxes(CollisionBody *cb, int frame = 0);
	void DrawSprite( 
		sf::RenderTarget *target,
		sf::Sprite &spr );
	virtual void UpdateEnemyPhysics();
	virtual void HandleHitAndSurvive() {}
	virtual void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void CheckedZoneDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	virtual void SetZoneSpritePosition();
	void CheckedZoneUpdate(sf::FloatRect &rect);
	virtual void ShieldDestroyed( Shield *shield ) {};
	virtual void UpdatePhysics( int substep );
	virtual bool LaunchersAreDone();
	virtual bool IsSlowed( int index );
	virtual int GetSlowFactor(int playerIndex);
	virtual HitboxInfo * IsHit(int pIndex );
	virtual void HandleNoHealth();
	virtual void HandleRemove();
	virtual void ProcessState() = 0;
	virtual void DebugDraw(sf::RenderTarget *target);
	virtual void UpdateHitboxes();
	void BasicUpdateHitboxes();
	void BasicUpdateHitboxInfo();
	virtual void ResetEnemy() = 0;
	virtual V2d TurretSetup();//return finals pos
	virtual void Init(){};
	virtual void Setup();
	virtual void ProcessHit();
	virtual void ProcessShieldHit();
	virtual void ConfirmHitNoKill();
	virtual void ConfirmKill();
	void MovePos(V2d &vel,
		int slowMult,
		int numPhysSteps);
	virtual void HandleWireHit(Wire *w) {}
	virtual void HandleWireAnchored(Wire *w) {}
	virtual void HandleWireUnanchored(Wire *w) {}
	virtual bool CanBeHitByPlayer() { return true; }
	virtual bool CanBeHitByComboer() { return true; }
	virtual bool CanBeHitByWireTip(bool red) { return false; }
	virtual bool CanBeAnchoredByWire(bool red) { return true; }
	virtual void RecordEnemy();
	virtual void DirectKill();
	virtual void HandleEntrant(QuadTreeEntrant *qte) {}
	virtual void UpdatePrePhysics();
	virtual void UpdatePostPhysics();
	virtual void UpdateSprite() {}
	virtual void FrameIncrement(){}
	virtual bool SpawnableByCamera() { return true; }
	virtual void DrawMinimap( sf::RenderTarget *target );
	virtual void IHitPlayer(int index = 0) {}
	virtual void IHitPlayerShield(int index = 0) { IHitPlayer(index); }
	virtual bool CheckHitPlayer(int index = 0);
	bool BasicCheckHitPlayer( CollisionBody *body, int index);
	virtual int NumTotalBullets();
	virtual void HandleQuery(QuadTreeCollider * qtc);
	virtual bool IsTouchingBox(const sf::Rect<double> &r);
	void Record(int enemyIndex);
	bool RightWireHitMe( CollisionBox hurtBox );
	bool LeftWireHitMe( CollisionBox hurtBox );
	virtual void EnemyDraw(sf::RenderTarget *target) {}
	virtual void Draw(sf::RenderTarget *target);
	virtual void UpdateZoneSprite();
	virtual void ZoneDraw(sf::RenderTarget *target);
	void BasicCircleHurtBodySetup(double radius, 
		double angle, V2d &offset, V2d &pos );
	void BasicCircleHitBodySetup(double radius, 
		double angle, V2d &offset, V2d &pos);
	void BasicCircleHurtBodySetup(double radius);
	void BasicCircleHitBodySetup(double radius);
	void BasicRectHurtBodySetup(
		double w, double h, double angle, V2d &offset,
		V2d &pos);
	void BasicRectHurtBodySetup(
		double w, double h, double angle, V2d &offset);
	void BasicRectHitBodySetup(
		double w, double h, double angle, V2d &offset,
		V2d &pos);
	void BasicRectHitBodySetup(
		double w, double h, double angle, V2d &offset);
	void Reset();
	virtual bool CanTouchSpecter();
	virtual void CheckSpecters();
	void CheckTouchingSpecterField(SpecterArea *sa);
	virtual bool IsTouchingSpecterField(SpecterArea *sa);
	virtual std::vector<CollisionBox>  * GetComboHitboxes();

	Actor *GetFocusedPlayer();
	V2d GetFocusedPlayerPos();
	double GetFocusedPlayerDist();
	V2d GetFocusedPlayerDir();
	
};



struct BasicPathFollower
{

	struct MyData
	{
		bool forward;
		int targetNode;
	};

	MyData data;
	bool loop;
	std::vector<sf::Vector2i> path;
	int nodeWaitFrames; //need to implement this

	void SetParams(ActorParams *ap);
	void Reset();
	void AdvanceTargetNode();
	void Move(double amount, V2d &pos );

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif