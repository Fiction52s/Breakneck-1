#include "Enemy.h"
#include "Enemy_Teleporter.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

//void Teleporter::UpdateParamsSettings()
//{
//
//	if (springType != TELEPORT)
//	{
//		SpringParams *sParams = (SpringParams*)editParams;
//		speed = sParams->speed;
//		stunFrames = ceil(dist / speed);
//
//		stringstream ss;
//		ss << speed;
//		debugSpeed.setString(ss.str());
//		debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
//	}
//}

void Teleporter::SetLevel(int lev)
{
	level = lev;
	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void Teleporter::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	if (!goesBothWays || !secondary)
	{
		UpdatePath(); //assuming that ap is editparams here
	}
}

void Teleporter::UpdatePath()
{
	Vector2i other = Vector2i(0, -10);
	if (editParams->localPath.size() > 0)
		other = editParams->GetLocalPathPos(0);

	dest = GetPosition() + V2d(other);

	V2d dOther = V2d(other.x, other.y);
	V2d springVec = normalize(dOther);

	double angle = atan2(springVec.x, -springVec.y);
	sprite.setRotation(angle / PI * 180.0);

	dist = length(V2d(other));

	speed = 200;
	stunFrames = floor(dist / speed);

	dir = springVec;

	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = GetPositionF();
	debugLine[1].position = Vector2f(dest);

	if (goesBothWays && !secondary)
	{
		otherParams->SetPosition(dest);
		otherTele->UpdateOnPlacement(otherParams);

		if (otherParams->localPath.size() == 0)
		{
			otherParams->localPath.push_back(Vector2i(GetPosition() - dest));
		}
		else
		{
			otherParams->GetLocalPath()[0] = Vector2i(GetPosition() - dest);
		}

		otherTele->UpdatePath();
		otherTele->UpdateSprite();
		
	}
}

void Teleporter::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);

	if (goesBothWays && !secondary)
	{
		otherTele->AddToWorldTrees();
	}
}

Teleporter::Teleporter(ActorParams *ap)//SpringType sp, Vector2i &pos, Vector2i &other, int p_speed )
	:Enemy(EnemyType::EN_TELEPORTER, ap)//false, 2, false ), springType( sp )
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	SetLevel(ap->GetLevel());

	otherParams = NULL;
	otherTele = NULL;

	V2d position = GetPosition();
	Vector2f positionF(position);

	string &typeName = ap->type->info.name;
	if (typeName == "teleporter")
	{
		goesBothWays = false;
	}
	else if (typeName == "doubleteleporter")
	{
		goesBothWays = true;
	}

	launchSoundBuf = sess->GetSound("Enemies/spring_launch");

	ts_idle = sess->GetSizedTileset("Enemies/spring_idle_2_256x256.png");
	ts_recover = sess->GetSizedTileset("Enemies/spring_recover_2_256x256.png");
	ts_springing = sess->GetSizedTileset("Enemies/spring_spring_2_512x576.png");


	TeleporterParams *tp = (TeleporterParams*)ap;

	secondary = tp->secondary;

	if (secondary)
	{
		sprite.setColor(Color::Red);
	}
	else
	{
		sprite.setColor(Color::Magenta);

		if (goesBothWays)
		{
			CreateSecondary(ap);
		}
	}
	

	double radius = 64;

	BasicCircleHitBodySetup(radius);

	actionLength[IDLE] = 12;
	actionLength[TELEPORTING] = 8;
	actionLength[RECEIVING] = 8;
	actionLength[RECEIVE_RECOVERING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[TELEPORTING] = 4;
	animFactor[RECEIVING] = 4;
	animFactor[RECEIVE_RECOVERING] = 4;
	animFactor[RECOVERING] = 8; //this was 4 originally.

	animationFactor = 10;

	sprite.setTextureRect(ts_idle->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(positionF);

	

	editParams = ap;
	UpdatePath();


	ResetEnemy();
}

Teleporter::~Teleporter()
{
	if (!secondary && goesBothWays)
	{
		//delete otherTele;
		delete otherParams;
	}
}

void Teleporter::AddToGame()
{
	Enemy::AddToGame();

	if (goesBothWays && !secondary)
	{
		otherTele->AddToGame();
	}
}

void Teleporter::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	target->draw(debugLine, 2, sf::Lines);
}

void Teleporter::DirectKill()
{
	//impossible to direct kill
}

void Teleporter::ResetEnemy()
{
	dead = false;

	teleportedPlayer = false;
	receivedHit = NULL;
	action = IDLE;
	sprite.setTexture(*ts_idle->texture);

	frame = 0;
	SetHitboxes(&hitBody);
	//SetHurtboxes(hurtBody, 0);

	UpdateHitboxes();

	UpdateSprite();
}

void Teleporter::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case TELEPORTING:
			action = RECOVERING;
			sprite.setTexture(*ts_recover->texture);
			break;
		case RECEIVING:
			break;
		case RECEIVE_RECOVERING:
			break;
		case RECOVERING:
			action = IDLE;
			sprite.setTexture(*ts_idle->texture);
			teleportedPlayer = false;
			break;
		}
	}
}

void Teleporter::CreateSecondary(ActorParams *ap)
{
	assert(otherTele == NULL);
	if ( goesBothWays && !secondary)
	{
		otherParams = (TeleporterParams*)ap->Copy();
		otherParams->secondary = true;
		otherParams->CreateMyEnemy();
		otherTele = (Teleporter*)otherParams->myEnemy;
		otherTele->otherTele = this;
	}
}

bool Teleporter::TryTeleport()
{
	if (otherTele->teleportedPlayer || (!goesBothWays && secondary))
	{
		return false;
	}

	assert(action == IDLE);
	action = TELEPORTING;
	sprite.setTexture(*ts_springing->texture);
	frame = 0;
	teleportedPlayer = true;
	sess->ActivateSound(launchSoundBuf);

	return true;
}

void Teleporter::ReceivePlayer()
{
	Teleporter *curr = this;
	if (!teleportedPlayer)
	{
		curr = otherTele;
	}

	curr->action = RECOVERING;
	curr->frame = 0;
}

void Teleporter::ProcessState()
{
	ActionEnded();
}


void Teleporter::UpdateSprite()
{
	switch (action)
	{
	case IDLE:
		sprite.setTextureRect(ts_idle->GetSubRect(frame / animFactor[action]));
		break;
	case TELEPORTING:
		sprite.setTextureRect(ts_springing->GetSubRect(frame / animFactor[action]));
		break;
	case RECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	case RECEIVING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	case RECEIVE_RECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	}
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	if (goesBothWays && !secondary)
	{
		//otherTele->UpdateSprite(); //only works correctly while editing, 
		//not while playing. fix this
	}
}

void Teleporter::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);

	if (goesBothWays && !secondary)
	{
		//otherTele->EnemyDraw(target); //also only needed while editing
	}
}