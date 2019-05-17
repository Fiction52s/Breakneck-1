#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Nexus.h"
#include "Sequence.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;


Nexus::Nexus(GameSession *owner, Edge *g, double q)
	:Enemy(owner, EnemyType::EN_NEXUS, false, 0, false),
	ground(g), edgeQuantity(q), dead(false)
{
	double width;
	double height;
	int world = owner->mh->envWorldType;
	switch (world)
	{
	case 0:
		width = 288;
		height = 320;
		ts_nexusOpen = owner->GetTileset("Nexus/nexus_open_1296x904.png", 1296, 904);
		ts_nexusDestroyed = owner->GetTileset("Nexus/nexus_destroyed_1296x904.png", 1296, 904);
		ts_mini = owner->GetTileset("minimap_icons_64x64.png", 64, 64);
		ts_explosion = owner->GetTileset("Goal/goal_w01_b_480x480_0.png", 480, 480);
		ts_explosion1 = owner->GetTileset("Goal/goal_w01_b_480x480_1.png", 480, 480);
		ts_node1 = owner->GetTileset("Goal/nexus_node_1_512x512.png", 512, 512);
		ts_node2 = owner->GetTileset("Goal/nexus_node_2_512x512.png", 512, 512);
		explosionLength = 24;
		explosionAnimFactor = 3;
		explosionYOffset = 80;
		initialYOffset = 30;
		break;
	case 1:
	default:
		width = 288;
		height = 256;
		ts_mini = owner->GetTileset("minimap_icons_64x64.png", 64, 64);
		ts_node1 = owner->GetTileset("Goal/nexus_node_1_512x512.png", 512, 512);
		ts_node2 = owner->GetTileset("Goal/nexus_node_2_512x512.png", 512, 512);
		ts_explosion = owner->GetTileset("Goal/goal_w02_b_288x320.png", 288, 320);
		ts_explosion1 = NULL;
		explosionLength = 15;
		explosionAnimFactor = 2;
		explosionYOffset = 0;
		initialYOffset = 0;
		break;
	}

	nexSprite.setTexture(*ts_nexusOpen->texture);
	nexSprite.setTextureRect(ts_nexusOpen->GetSubRect( 0 ));
	nexSprite.setOrigin(nexSprite.getLocalBounds().width / 2, nexSprite.getLocalBounds().height - 50);
	//575,550
	

	miniSprite.setTexture(*ts_mini->texture);
	miniSprite.setTextureRect(ts_mini->GetSubRect(2));
	miniSprite.setScale(16, 16);

	switch (owner->mh->envWorldType)
	{
	case 0:
		miniSprite.setTextureRect(ts_mini->GetSubRect(5));
		break;
	case 1:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 2:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 3:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 4:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 5:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	}

	miniSprite.setOrigin(miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height);


	V2d gPoint = g->GetPoint(edgeQuantity);

	nexSprite.setPosition(Vector2f(gPoint));

	gn = g->Normal();
	angle = atan2(gn.x, -gn.y);
	V2d along = g->Along();

	position = gPoint + gn * (906.0 - 600.0) + along * (575.0 - 648.0);  //height + V2d( 648-575, 453-550);// / 2.0;

	//miniSprite.setPosition( position.x, position.y );
	miniSprite.setPosition(gPoint.x, gPoint.y);
	miniSprite.setRotation(angle / PI * 180);

	sprite.setTexture(*ts_node1->texture);
	sprite.setTextureRect(ts_node1->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);// - initialYOffset);
	//.setPosition(gPoint.x, gPoint.y);
	sprite.setPosition(Vector2f(position));
	sprite.setRotation(angle / PI * 180);
	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - initialYOffset);

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = false;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 20;
	hurtBox.rw = 80;
	hurtBox.rh = 100;
	hurtBox.globalAngle = angle;
	hurtBox.globalPosition = position;

	hurtBody->AddCollisionBox(0, hurtBox);
	SetHurtboxes(hurtBody, 0);
	//currHurtboxes->GetCollisionBoxes(0)->front().globalPosition = position;
	double angle = 0;

	angle = atan2(gn.x, -gn.y);


	//V2d( hitBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
	//hitBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );

	frame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>(gPoint.x - 160 / 2, gPoint.y - 160 / 2, 160, 160);

	

	//health = 1;
	//numHealth = 1;

	insideSeq = new NexusCore1Seq(owner);

	action = A_SITTING;
}


Nexus::~Nexus()
{
	delete insideSeq;
}

void Nexus::ConfirmKill()
{
	dead = true;
	//owner->absorbParticles->Activate(owner->GetPlayer(0), 64, position);
	HandleNoHealth();
}

void Nexus::ResetEnemy()
{
	frame = 0;
	action = A_SITTING;
	insideSeq->Reset();
	SetHurtboxes(hurtBody, 0);
	nexSprite.setTexture(*ts_nexusOpen->texture);

	sprite.setTexture(*ts_node1->texture);
	sprite.setTextureRect(ts_node1->GetSubRect(0));
	//numHealth = 1;
	UpdateSprite();
	//sprite.setTexture(*ts_node1->texture);
	//sprite.setTextureRect(ts_node1->GetSubRect(0));
	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - initialYOffset);


}

void Nexus::ProcessState()
{
	if (action == A_KINKILLING)
	{
		if (frame == 1)
		{
			owner->cam.manual = true;
			owner->cam.Ease(Vector2f(position), 1, 60, CubicBezier());
		}

		//if (frame == 1)
		//{
		//	//owner->cam.manual = true;
		//	NexusKillStartZoom = owner->cam.zoomFactor;
		//	NexusKillStartPos = owner->cam.pos;
		//	//
		//}
		//if (frame <= 31 && frame > 1)
		//{
		//	CubicBezier bez(0, 0, 1, 1);
		//	float z = bez.GetValue((double)(frame - 1) / 30);

		//	Vector2f po = NexusKillStartPos * (1.f - z) + Vector2f(owner->NexusNodePos.x,
		//		owner->NexusNodePos.y) * z;

		//	CubicBezier bez1(0, 0, 1, 1);
		//	float z1 = bez1.GetValue((double)(frame - 1) / 30);

		//	float zoom = NexusKillStartZoom * (1.f - z) + 1.f * z;

		//	owner->cam.manual = true;
		//	
		//	//owner->cam.manual = true;
		//	//owner->cam.Set(po, zoom, 0);
		//	///Vector2f trueSpot = dropSpot + extra0;
		//	//owner->cam.Set( ( //trueSpot * 1.f/60.f + owner->cam.pos * 59.f/60.f ),
		//	//	1, 0 );

		//}
		if (frame == 46 * 2)
		{
			action = A_EXPLODING;
			//exploding = true;
			//kinKilling = false;
			frame = 0;
		}
	}
	else if (action == A_EXPLODING)
	{
		if (frame == explosionLength * explosionAnimFactor)
		{
			action = A_DESTROYED;
		}
	}
}

V2d Nexus::GetKillPos()
{
	return position;
}

void Nexus::HandleNoHealth()
{
	owner->KillAllEnemies();
	frame = 0;
	owner->GetPlayer(0)->hitNexus = true;
	SetHurtboxes(NULL, 0);
	action = A_KINKILLING;

	//owner->NexusPulse->show = true;
}

void Nexus::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(nexSprite);
	if (action < A_DESTROYED)
	{
		target->draw(sprite);
	}
}

void Nexus::FinishDestruction()
{
	action = A_NEXUSDESTROYED;
	frame = 0;
	nexSprite.setTexture(*ts_nexusDestroyed->texture);
}

void Nexus::DrawMinimap(sf::RenderTarget *target)
{
	target->draw(miniSprite);
}

void Nexus::UpdateSprite()
{
	int trueFrame = 0;
	if (action == A_KINKILLING)
	{
		if (frame / 2 < 12)
		{
			trueFrame = 1;
		}
		else if (frame / 2 < 18)
		{
			trueFrame = 2;
		}
		else if (frame / 2 < 30)
		{
			trueFrame = 3;
		}
		else
		{
			trueFrame = 3;
		}
		sprite.setTexture(*ts_node1->texture);
		sprite.setTextureRect(ts_node1->GetSubRect(trueFrame));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);


	}
	else if (action == A_EXPLODING)
	{
		trueFrame = frame / explosionAnimFactor;
		int numTiles = 16;//ts_explosion->GetNumTiles();
		if (trueFrame >= numTiles)
		{
			trueFrame -= numTiles;
			sprite.setTexture(*ts_node2->texture);
			sprite.setTextureRect(ts_node2->GetSubRect(trueFrame));
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		}
		else
		{
			sprite.setTexture(*ts_node1->texture);
			sprite.setTextureRect(ts_node1->GetSubRect(trueFrame));
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		}
		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
	}
	//else if (action == A_DESTROYED)
	//{
	//	trueFrame = //explosionLength - 1;
	//	int numTiles = 16;//ts_explosion->GetNumTiles();
	//	if (trueFrame >= numTiles)
	//	{
	//		trueFrame -= numTiles;
	//		sprite.setTexture(*ts_node2->texture);
	//		sprite.setTextureRect(ts_node2->GetSubRect(trueFrame));
	//	}
	//	else
	//	{
	//		sprite.setTexture(*ts_node1->texture);
	//		sprite.setTextureRect(ts_node1->GetSubRect(trueFrame));
	//	}

	//	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
	//}


}