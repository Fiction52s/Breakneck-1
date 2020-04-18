#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <list>
#include "Mover.h"
#include "Movement.h"
#include "EffectLayer.h"
#include <SFML/Audio.hpp>
#include "ObjectProperties.h"

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

struct EnemyParams
{
	EnemyParams()
	{
		canBeHit = false;
		worldIndex = 0;
		flashFrames = 0;
		speedBar = 0;
		charge = 0;
		maxHealth = 0;
	}

	void Set(int p_worldIndex,
		int p_flashFrames, double p_speedBar,
		int p_charge, int p_maxHealth)
	{
		canBeHit = true;
		worldIndex = p_worldIndex;
		flashFrames = p_flashFrames;
		speedBar = p_speedBar;
		charge = p_charge;
		maxHealth = p_maxHealth;
	}

	bool canBeHit;
	int worldIndex;
	int flashFrames;
	double speedBar;
	int charge;
	int maxHealth;
};





struct Enemy : QuadTreeCollider, QuadTreeEntrant, 
	SlowableObject, HittableObject
{

	//new vars
	EnemyParams hitParams;
	Session *sess;
	bool SetHitParams();

	//------------

	Enemy(EnemyType t,
		bool hasMonitor, int world, bool cuttable = true);
	virtual ~Enemy();
	

	static bool ReadBool(std::ifstream &is,
		bool &b);
	static bool ReadPath(std::ifstream &is,
		int &pLen, std::list<sf::Vector2i> &localPath);
	virtual void UpdatePreLauncherPhysics() {}
	CuttableObject *cutObject;
	Launcher **launchers;
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;
	sf::SoundBuffer *genericDeathSound;
	virtual void PlayDeathSound();
	virtual int GetNumCamPoints() { return 1; }
	virtual V2d GetCamPoint(int index) { return position; }
	virtual void ComboHit();
	virtual void ComboKill( Enemy *e );
	virtual void SetZone(Zone *p_zone) { zone = p_zone; }
	CollisionBody *currHitboxes;
	static void SyncSpriteInfo(sf::Sprite &dest, sf::Sprite &source);
	void SetHitboxes(CollisionBody *cb, int frame);
	void ClearHitboxes() { 
		currHitboxes = NULL;
		currHitboxFrame = -1;
	}
	void ClearHurtboxes() {
		currHurtboxes = NULL;
		currHurtboxFrame = -1;
	}
	void SetHurtboxes(CollisionBody *cb, int frame);
	void DrawSpriteIfExists( 
		sf::RenderTarget *target,
		sf::Sprite &spr );
	//CollisionBox physBox;
	int currHitboxFrame;
	CollisionBody *currHurtboxes;
	bool highResPhysics;
	int currHurtboxFrame;
	virtual void UpdateEnemyPhysics() {}
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
	virtual void Setup() {};
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

	void SetupBodies(int numHurtboxes = 1,
		int numHitboxes = 1);
	void AddBasicHurtCircle(double rad, int index = 0);
	void AddBasicHitCircle(double rad, int index = 0);
	void AddBasicHitCircle(CollisionBody *bod, double rad, int index = 0);
	void AddBasicHurtCircle(CollisionBody *bod, double rad, int index = 0);
	void AddBasicHitRect(CollisionBody *bod, double w, double h, int index = 0);
	void AddBasicHurtRect(CollisionBody *bod, double w, double h, int index = 0);

	void Reset();

	virtual bool CanTouchSpecter();
	virtual void CheckSpecters();
	void CheckTouchingSpecterField(SpecterArea *sa);
	virtual bool IsTouchingSpecterField(SpecterArea *sa);
	
	virtual std::list<CollisionBox> *GetComboHitboxes();
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
	sf::Vector2<double> position;
	Zone *zone;
	bool hasMonitor;
	bool dead;
	bool suppressMonitor;
	Tileset *ts_hitSpack;
	Tileset *ts_killSpack;
	sf::Shader *keyShader;
	sf::Shader *hurtShader;

	Tileset *ts_blood;

	sf::Sprite *keySprite;
	Tileset *ts_key;
	sf::Color keyColor;
	int world;

	Enemy *tempPrev;
	Enemy *tempNext;

	int level;
	float scale;
	int maxHealth;
};



struct PathFollower
{

};

#endif