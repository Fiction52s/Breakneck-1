#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "FlowerPod.h"
#include "StorySequence.h"

using namespace std;
using namespace sf;

FlowerPod::FlowerPod(GameSession *owner, const std::string &typeStr, Edge *g, double q)
	:Enemy(owner, EnemyType::EN_FLOWERPOD, false, 0, false), ground(g),
	edgeQuantity(q)
{
	double width = 64; //112;
	double height = 64;

	podType = GetType(typeStr);

	switch (podType)
	{
	case SEESHARDS:
		storySeq = new StorySequence(owner);
		storySeq->Load("getairdash");
		break;
	}

	ts = owner->GetTileset("curveturret_64x64.png", width, height);
	sprite.setTexture(*ts->texture);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	V2d gPoint = g->GetPoint(edgeQuantity);
	sprite.setPosition(gPoint.x, gPoint.y);

	V2d gn = g->Normal();

	V2d gAlong = normalize(g->v1 - g->v0);

	position = gPoint + gn * height / 2.0;

	double angle = atan2(gn.x, -gn.y);

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setPosition(gPoint.x, gPoint.y);
	sprite.setRotation(angle / PI * 180);

	actionLength[IDLE] = 1;
	actionLength[ACTIVATE] = 20;
	actionLength[BROADCAST] = 120;
	actionLength[HIDE] = 120;
	actionLength[DEACTIVATED] = 120;

	animFactor[IDLE] = 1;
	animFactor[ACTIVATE] = 1;
	animFactor[BROADCAST] = 1;
	animFactor[HIDE] = 1;
	animFactor[DEACTIVATED] = 1;


	//raycast here to either the terrain above me or a max height
	rayStart = position;
	rayEnd = position + gn * 2000.0;
	rcEdge = NULL;
	RayCast(this, owner->terrainTree->startNode, rayStart, rayEnd);

	double boxHeight = 1000;
	if (rcEdge != NULL)
	{
		boxHeight = length(rcEdge->GetPoint(rcQuantity) - gPoint);
	}

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = false;
	hitBox.globalAngle = atan2(-gAlong.y, -gAlong.x);;
	hitBox.offset.x = 0;
	hitBox.offset.y = boxHeight / 2.0 - height / 2.0;
	hitBox.rw = 100;
	hitBox.rh = boxHeight / 2.0;
	hitBody->AddCollisionBox(0, hitBox);

	double size = max(width, height);
	spawnRect = sf::Rect<double>(gPoint.x - size / 2, gPoint.y - size / 2, size, size);

	ResetEnemy();
}

void FlowerPod::ResetEnemy()
{
	action = IDLE;
	frame = 0;
	dead = false;
	frame = 0;
	SetHitboxes(hitBody, 0);
	UpdateHitboxes();
	UpdateSprite();
	sprite.setColor(Color::White);
	storySeq->Reset();
}

void FlowerPod::IHitPlayer(int index)
{
	if (action == IDLE)
	{
		action = ACTIVATE;
		sprite.setColor(Color::Red);
	}
}

FlowerPod::PodType FlowerPod::GetType(const std::string &tStr)
{
	string testStr = tStr;
	std::transform(testStr.begin(), testStr.end(), testStr.begin(), ::tolower);
	if (testStr == "seeshards")
	{
		return SEESHARDS;
	}
	else
	{
		return  SEESHARDS;
	}
}

void FlowerPod::HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion)
{
	V2d dir = normalize(rayEnd - rayStart);
	V2d pos = edge->GetPoint(edgeQuantity);
	double along = dot(dir, edge->Normal());
	if (along < 0 && (rcEdge == NULL || length(edge->GetPoint(edgeQuantity) - rayStart) <
		length(rcEdge->GetPoint(rcQuantity) - rayStart)))
	{
		rcEdge = edge;
		rcQuantity = edgeQuantity;
	}
}

void FlowerPod::ActionEnded()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case ACTIVATE:
			action = BROADCAST;
			owner->currStorySequence = storySeq;
			break;
		case BROADCAST:
			action = HIDE;
			break;
		case HIDE:
			action = DEACTIVATED;
			break;
		case DEACTIVATED:
			//remove from active pool
			break;
		}
	}
}

void FlowerPod::ProcessState()
{
	ActionEnded();

	Actor *player = owner->GetPlayer(0);
	double dist = length(player->position - position);

	switch (action)
	{
	case IDLE:
		break;
	case ACTIVATE:
		break;
	case BROADCAST:
		break;
	}

	switch (action)
	{
	case IDLE:
		break;
	case ACTIVATE:
		break;
	case BROADCAST:
		break;
	}
}

void FlowerPod::FrameIncrement()
{
}

void FlowerPod::UpdateEnemyPhysics()
{

}

void FlowerPod::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

void FlowerPod::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(0));//frame / animationFactor ) );
}

void FlowerPod::UpdateHitboxes()
{
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;// + gn * 8.0;
	//hitBox.globalAngle = 0;
}

void FlowerPod::DirectKill()
{

}