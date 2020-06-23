#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <list>
#include "Mover.h"
#include "Movement.h"
#include "EffectLayer.h"
#include <SFML/Audio.hpp>
#include "ObjectProperties.h"
#include "ActorParams.h"

struct Wire;
struct Zone;
struct Tileset;
struct Actor;
struct Launcher;
struct Shield;
struct SpecterArea;
struct SpecterTester;
struct Session;

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





struct Enemy : QuadTreeCollider, QuadTreeEntrant, 
	SlowableObject, HittableObject
{
	void UpdateKeySprite();
	bool keyShaderLoaded;
	void SetKey();
	bool IsGoalType();
	virtual void UpdateSpriteFromParams(ActorParams *ap);
	virtual void AddToWorldTrees() {}

	virtual void UpdateFromParams(ActorParams *ap,
		int numFrames );
	virtual void UpdateParamsSettings();
	virtual void UpdateOnPlacement(ActorParams *ap);
	void UpdateOnEditPlacement() {
		UpdateOnPlacement(editParams);
	}
	//virtual Enemy *Copy() { return NULL; }
	void UpdateFromEditParams(int numFrames)
	{
		UpdateFromParams(editParams, numFrames);
	}

	int SetLaunchersStartIndex(int ind);
	virtual void UpdatePath() {}
	void SetNumLaunchers(int num);
	virtual void AddChildrenToSession( Session *s) {}
	virtual void RemoveChildrenFromSession(Session *s) {}
	//new vars
	virtual void SetLevel(int p_level) { level = p_level; }
	virtual void SetSpawnRect();
	//void SetStartGroundInfo
	PositionInfo startPosInfo;
	void SetCurrPosInfo(PositionInfo &pi);
	V2d GetPosition();
	sf::Vector2f GetPositionF();
	//PositionInfo currPosInfo;
	HitParams hitParams;
	Session *sess;
	bool SetHitParams();
	ActorParams *editParams;
	void SetOffGroundHeight(double h);
	void SetGroundOffset(double x);
	V2d groundOffset;
	
	

	double DistFromPlayer(int index = 0);
	double DistFromPlayerSqr(int index = 0);
	V2d AlongGroundDir();
	bool facingRight;
	bool origFacingRight;

	PositionInfo currPosInfo;

	SurfaceMover *surfaceMover;
	GroundMover *groundMover;
	void CreateSurfaceMover(PositionInfo &pi,
		double rad, SurfaceMoverHandler *handler);
	void CreateGroundMover(PositionInfo &pi,
		double rad, bool steeps,
		GroundMoverHandler *handler);
	double GetGroundedAngleRadians();

	void SetNumActions( int num );
	int GetEditIdleLength();
	virtual void SetActionEditLoop();

	int editLoopAction;
	int editIdleAction;
	int editIdleFrame;
	void SetEditorActions(
		int editLoopAction = 0, int editIdleAction=0,
		int editIdleFrame=0);

	std::vector<int> actionLength;
	std::vector<int> animFactor;

	int action;

	void DefaultHitboxesOn(int hFrame = 0);
	void DefaultHurtboxesOn(int hFrame = 0);
	void HurtboxesOff();
	void HitboxesOff();

	virtual void SyncCutObject();



	virtual sf::FloatRect GetAABB(){ 
		return sprite.getGlobalBounds();
	}
	//------------

	Enemy(EnemyType t, ActorParams *ap);
	virtual ~Enemy();
	
	virtual void InitOnRespawn() {}
	virtual void SetupResources() {}
	virtual void SetupDebugResources() {}
	static bool ReadBool(std::ifstream &is,
		bool &b);
	static bool ReadPath(std::ifstream &is,
		int &pLen, std::vector<sf::Vector2i> &localPath);
	virtual void UpdatePreLauncherPhysics() {}
	CuttableObject *cutObject;
	Launcher **launchers;
	CollisionBody hurtBody;
	CollisionBody hitBody;
	HitboxInfo *hitboxInfo;
	sf::SoundBuffer *genericDeathSound;
	virtual void PlayDeathSound();
	virtual int GetNumCamPoints() { return 1; }
	virtual V2d GetCamPoint(int index) { return GetPosition(); }
	virtual void ComboHit();
	virtual void ComboKill( Enemy *e );
	virtual void SetZone(Zone *p_zone) { zone = p_zone; }
	CollisionBody *currHitboxes;
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
	void DrawSprite(
		sf::RenderTarget *target,
		sf::Sprite &spr, sf::Sprite &auraSpr );
	//CollisionBox physBox;
	int currHitboxFrame;
	CollisionBody *currHurtboxes;
	bool highResPhysics;
	int currHurtboxFrame;
	virtual void UpdateEnemyPhysics();
	virtual void HandleHitAndSurvive() {}
	virtual void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void CheckedZoneDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	virtual void SetZoneSpritePosition();
	void CheckedZoneUpdate(sf::FloatRect &rect);
	virtual void ShieldDestroyed( Shield *shield ) {};
	//std::list<CollisionBox> *activeHurtboxes;
	CollisionBox *physicsBox;
	virtual void UpdatePhysics( int substep );
	int numLaunchers;
	virtual bool LaunchersAreDone();
	virtual bool IsSlowed( int index );
	virtual HitboxInfo * IsHit(Actor *player);
	
	virtual void HandleNoHealth();
	virtual void HandleRemove() {}
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
	virtual void DrawMinimap( sf::RenderTarget *target );
	virtual void IHitPlayer(int index = 0) {}
	virtual bool CheckHitPlayer(int index = 0);
	virtual int NumTotalBullets();
	virtual void HandleQuery(QuadTreeCollider * qtc);
	virtual bool IsTouchingBox(const sf::Rect<double> &r);

	void Record(int enemyIndex);
	bool RightWireHitMe( CollisionBox hurtBox );
	bool LeftWireHitMe( CollisionBox hurtBox );

	virtual void EnemyDraw(sf::RenderTarget *target) {}
	virtual void Draw(sf::RenderTarget *target);
	virtual void UpdateZoneSprite();
	sf::Sprite zonedSprite;
	Tileset *ts_zoned;
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
	ComboObject *comboObj;
	SpecterTester *specterTester;
	Shield *currShield;
	int pauseFrames;
	double numPhysSteps;
	Enemy *prev;
	Enemy *next;
	bool spawned;
	sf::Color auraColor;
	sf::Rect<double> spawnRect;
	int frame;
	EnemyType type;
	bool spawnedByClone;
	Zone *zone;
	bool hasMonitor;
	bool dead;
	bool suppressMonitor;
	Tileset *ts_hitSpack;
	Tileset *ts_killSpack;
	sf::Shader keyShader;
	sf::Sprite keySprite;
	
	sf::Shader hurtShader;

	Tileset *ts_blood;
	sf::Sprite sprite;
	sf::Sprite auraSprite;
	
	Tileset *ts_key;
	sf::Color keyColor;
	int world;

	Enemy *tempPrev;
	Enemy *tempNext;

	int level;
	double scale;
	int maxHealth;

	std::string name;
};



struct BasicPathFollower
{
	bool loop;
	int targetNode;
	bool forward;
	std::vector<sf::Vector2i> path;
	int nodeWaitFrames; //need to implement this

	void SetParams(ActorParams *ap);
	void Reset();
	void AdvanceTargetNode();
	void Move(double amount, V2d &pos );
};

#endif