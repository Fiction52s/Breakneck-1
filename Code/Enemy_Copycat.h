#ifndef __ENEMY_COPYCAT_H__
#define __ENEMY_COPYCAT_H__

#include "Enemy.h"

struct Copycat : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		THROW,
		Count
	};

	Tileset *ts_bulletExplode;
	Launcher *launcher;
	struct PlayerAttack
	{
		enum Type
		{
			FAIR,
			DAIR,
			UAIR,
			STANDN,
			CLIMBATTACK,
			SLIDEATTACK,
			WALLATTACK,
			Count
		};

		int currAttackIndex;
		Copycat *parent;
		PlayerAttack(Copycat *parent);
		//Action a;
		Type t;
		bool facingRight;
		bool reversed;
		int speedLevel;
		sf::Vector2<float> position;
		sf::Vector2<float> swordPosition;
		float angle;
		bool attackActive;
		int index;
		PlayerAttack *nextAttack;
		PlayerAttack *prevAttack;
		int frame;
		sf::Sprite sprite;
		sf::Sprite swordSprite;
		bool Update();
		void UpdateHitboxes();
		void UpdateSprite();
		void Set(PlayerAttack::Type nt,
			bool facingRight,
			bool reversed,
			int speedLevel,
			const sf::Vector2f &pos,
			const sf::Vector2f &swordPos,
			float angle);
		//void SetType( Type nt );
		void Draw(sf::RenderTarget *target);
		void DebugDraw(sf::RenderTarget *target);

		void CopyHitboxes(int index,
			std::map<int, std::list<CollisionBox>*> &playerBoxes);

		std::map<int, std::list<CollisionBox>*>
			hitboxes[PlayerAttack::Type::Count];
	};


	int GetAttackIndex();
	Tileset *ts_attacks[PlayerAttack::Type::Count];
	Tileset *ts_swords[PlayerAttack::Type::Count * 3];
	void BulletHitTarget(BasicBullet *b);
	PlayerAttack *GetAttack();
	PlayerAttack *PopAttack();
	void RemoveAttack(PlayerAttack *pa);
	void ClearTargets();
	void ClearTarget(int index);
	void SetTarget(int index, const sf::Vector2f &pos);
	void ResetAttacks();
	PlayerAttack *activeAttacksFront;
	PlayerAttack *activeAttacksBack;
	PlayerAttack *inactiveAttacks;
	PlayerAttack **allAttacks;
	int attackBufferSize;

	//std::map<sf::Vector2<double>, int> bulletMap;

	sf::Vector2<float> destPos;
	bool fire;

	Copycat(GameSession *owner, bool hasMonitor,
		sf::Vector2i &pos);
	void QueueAttack(PlayerAttack::Type t,
		bool facingRight,
		bool reversed, int speedLevel,
		const sf::Vector2<float> &pos,
		const sf::Vector2<float> &swordPos,
		float angle);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	void DirectKill();
	void SetTarget();

	std::map<Action, int> actionLength;
	std::map<Action, int> animFactor;

	int currAttackFrame;
	//PlayerAttack currAttack;
	bool activeActive;

	Action action;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;

	sf::Vector2i originalPos;
	int frame;

	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;
	//std::list<CollisionBox> *currHitboxes;


	/*std::map<int, std::list<CollisionBox>*> uairHitboxes;
	std::map<int, std::list<CollisionBox>*> dairHitboxes;
	std::map<int, std::list<CollisionBox>*> standHitboxes;
	std::map<int, std::list<CollisionBox>*> wallHitboxes;
	std::map<int, std::list<CollisionBox>*> steepClimbHitboxes;
	std::map<int, std::list<CollisionBox>*> steepSlideHitboxes;*/


	int attackLength[PlayerAttack::Type::Count];
	int attackFactor[PlayerAttack::Type::Count];

	sf::VertexArray *targetVA;
	//sf::VertexArray bulletVA;

	bool dying;

	Tileset *ts_target;


	sf::Vector2<double> shadowPos;
	sf::Sprite sprite;
	sf::Sprite shadowSprite;
	sf::Sprite shadowSword;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;


	int hitlagFrames;
	int hitstunFrames;
	//int animationFactor;
	bool facingRight;



	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

#endif