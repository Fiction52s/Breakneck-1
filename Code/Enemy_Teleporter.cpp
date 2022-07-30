#include "Enemy.h"
#include "Enemy_Teleporter.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"
#include "EditSession.h"

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

	double spriteAngle = angle / PI * 180.0;

	sprite.setRotation(spriteAngle);
	recoverSprite.setRotation(spriteAngle);
	boostSprite.setRotation(spriteAngle);
	particleSprite.setRotation(spriteAngle);

	dist = length(V2d(other));

	speed = 50;
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

	tilesetChoice = 1;
	recoverTileseChoice = 1;
	recoverStartFrame = 16;
	startFrame = 5;

	if (tilesetChoice == 0)
	{
		ts = GetSizedTileset("Enemies/Launchers/launcher_1_384x384.png");
	}
	else if (tilesetChoice == 1)
	{
		ts = GetSizedTileset("Enemies/Launchers/launcher_2_384x384.png");
	}

	if (recoverTileseChoice == 0)
	{
		ts_recover = GetSizedTileset("Enemies/Launchers/launcher_recover_1_384x384.png");
	}
	else if (recoverTileseChoice == 1)
	{
		ts_recover = GetSizedTileset("Enemies/Launchers/launcher_recover_2_384x384.png");
	}
	else if (recoverTileseChoice == 2)
	{
		ts_recover = GetSizedTileset("Enemies/Launchers/launcher_recover_3_384x384.png");
	}

	ts_particles = GetSizedTileset("Enemies/Launchers/launcher_particles_256x256.png");
	ts_boost = GetSizedTileset("Enemies/Launchers/launcher_explode_512x512.png");

	particleSprite.setTexture(*ts_particles->texture);
	boostSprite.setTexture(*ts_boost->texture);
	recoverSprite.setTexture(*ts_recover->texture);
	sprite.setTexture(*ts->texture);

	launchSoundBuf = GetSound("Enemies/spring_launch");


	TeleporterParams *tp = (TeleporterParams*)ap;

	secondary = tp->secondary;

	if (secondary)
	{
		//sprite.setColor(Color::Red);
	}
	else
	{
		//sprite.setColor(Color::Magenta);

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
	animFactor[RECOVERING] = 4; //8

	animationFactor = 10;

	/*sprite.setTextureRect(ts_idle->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(positionF);
*/
	

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

	frame = 0;
	SetHitboxes(&hitBody);
	//SetHurtboxes(hurtBody, 0);

	UpdateHitboxes();

	UpdateSprite();

	if (goesBothWays && !secondary)
	{
		otherTele->Reset();
	}
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
			break;
		case RECEIVING:
			break;
		case RECEIVE_RECOVERING:
			break;
		case RECOVERING:
			action = IDLE;
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
	if ( action != IDLE || (otherTele != NULL && otherTele->teleportedPlayer) )
	{
		return false;
	}

	assert(action == IDLE);
	action = TELEPORTING;
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
		sprite.setTextureRect(ts->GetSubRect((frame / animFactor[action]) / 3 + startFrame));
		particleSprite.setTextureRect(ts_particles->GetSubRect(frame / animFactor[action]));
		break;
	case TELEPORTING:
		boostSprite.setTextureRect(ts_boost->GetSubRect(frame / animFactor[action]));
		sprite.setTextureRect(ts->GetSubRect(4 + startFrame));
		break;
	case RECOVERING:
		recoverSprite.setTextureRect(ts->GetSubRect(frame / animFactor[action] + recoverStartFrame));
		sprite.setTextureRect(ts->GetSubRect(4 + startFrame));
		break;
	}

	particleSprite.setOrigin(particleSprite.getLocalBounds().width / 2,
		particleSprite.getLocalBounds().height / 2);
	particleSprite.setPosition(GetPositionF());

	boostSprite.setOrigin(boostSprite.getLocalBounds().width / 2,
		boostSprite.getLocalBounds().height / 2);
	boostSprite.setPosition(GetPositionF());

	recoverSprite.setOrigin(recoverSprite.getLocalBounds().width / 2, recoverSprite.getLocalBounds().height / 2);
	recoverSprite.setPosition(GetPositionF());

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	if (goesBothWays && !secondary)
	{
		EditSession *edit = EditSession::GetSession();

		if (edit != NULL && edit->mode != EditSession::TEST_PLAYER)
		{
			otherTele->UpdateSprite();
		}
	}
}

void Teleporter::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);

	if (action == RECOVERING)
	{
		target->draw(recoverSprite);
	}
	else if (action == IDLE)
	{
		target->draw(particleSprite);
	}

	if (action == TELEPORTING)
	{
		target->draw(boostSprite);
	}

	if (goesBothWays && !secondary)
	{
		EditSession *edit = EditSession::GetSession();

		if (edit != NULL && edit->mode != EditSession::TEST_PLAYER)
		{
			otherTele->EnemyDraw(target);
		}
	}
}

int Teleporter::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Teleporter::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Teleporter::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}