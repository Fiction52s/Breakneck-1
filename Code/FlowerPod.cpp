#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "FlowerPod.h"
#include "StorySequence.h"
#include "PowerOrbs.h"
#include "ImageText.h"
#include "GoalExplosion.h"

using namespace std;
using namespace sf;

FlowerPod::FlowerPod(GameSession *owner, const std::string &typeStr, Edge *g, double q)
	:Enemy(owner, EnemyType::EN_FLOWERPOD, false, 0, false), ground(g),
	edgeQuantity(q)
{
	healRing = new Ring;
	

	double width = 128; //112;
	double height = 128;

	//podType = GetType(typeStr);
	broadcast = new MomentaBroadcast( this, typeStr);
	/*switch (podType)
	{
	case SEESHARDS:
		storySeq = new StorySequence(owner);
		storySeq->Load("getairdash");
		break;
	}*/

	ts_flower = owner->GetTileset("Momenta/momentaflower_128x128.png", width, height);
	ts_bud = owner->GetTileset("Momenta/momentabud_128x128.png", width, height);
	ts_rise = owner->GetTileset("Momenta/momentaflower_rise_128x128.png", width, height);
	sprite.setTexture(*ts_bud->texture);
	sprite.setTextureRect(ts_bud->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height -16);
	V2d gPoint = g->GetPoint(edgeQuantity);
	sprite.setPosition(gPoint.x, gPoint.y);

	V2d gn = g->Normal();

	V2d gAlong = normalize(g->v1 - g->v0);

	camPosition = gPoint + gn * ( height + 0.0 );

	position = gPoint + gn * (height / 2.0);

	healRing->SetColor(Color::Cyan);
	healRing->Set(Vector2f(position), 100, 110);

	double angle = atan2(gn.x, -gn.y);
	sprite.setRotation(angle / PI * 180);
	//
	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	//sprite.setPosition(gPoint.x, gPoint.y);
	

	actionLength[IDLE] = 12;
	actionLength[ACTIVATE] = 36;
	actionLength[BROADCAST] = 12;// *4;//10
	actionLength[HIDE] = 36;
	actionLength[DEACTIVATED] = 120;


	animFactor[IDLE] = 6;
	animFactor[ACTIVATE] = 6;
	animFactor[BROADCAST] = 6;
	animFactor[HIDE] = 6;
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
	sprite.setTexture(*ts_bud->texture);
	dead = false;
	frame = 0;
	SetHitboxes(hitBody, 0);
	UpdateHitboxes();
	UpdateSprite();
	sprite.setColor(Color::White);
	//storySeq->Reset();
	broadcast->Reset();
	healingPlayer = NULL;
}

void FlowerPod::IHitPlayer(int index)
{
	if (action == IDLE)
	{
		action = ACTIVATE;
		frame = 0;
		sprite.setTexture(*ts_rise->texture);
		//sprite.setColor(Color::Red);
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
			sprite.setTexture(*ts_flower->texture);
			action = BROADCAST;
			frame = 0;
			owner->currBroadcast = broadcast;
			//owner->currStorySequence = storySeq;
			break;
		case BROADCAST:
			
			frame = 0;
			if (owner->currBroadcast == NULL)
			{
				action = HIDE;
				sprite.setTexture(*ts_rise->texture);
			}
		
			break;
		case HIDE:
			action = DEACTIVATED;
			frame = 0;
			break;
		case DEACTIVATED:
			//remove from active pool
			break;
		}
	}
}

void FlowerPod::ProcessState()
{
	//cout << "update" << endl;
	ActionEnded();

	Actor *player = owner->GetPlayer(0);
	double dist = length(player->position - position);
	float rad = healRing->innerRadius;
	
	if ( action == BROADCAST && dist <= rad && healingPlayer == NULL)
	{
		if (!owner->cam.manual)
			owner->cam.Ease(Vector2f(camPosition), .8, 300, CubicBezier());
		healingPlayer = player;
		//enter
	}
	else if (  (dist > rad || action != BROADCAST ) && healingPlayer != NULL )
	{
		if (owner->cam.manual)
		{
			owner->cam.EaseOutOfManual(60);
		}
		healingPlayer = NULL;
		//exit
	}

	if (healingPlayer != NULL && healingPlayer->drainCounter == 0)
	{
		owner->GetPlayer(0)->kinRing->powerRing->Fill( player->drainAmount + 1 );//powerWheel->Use( 1 );	
		//cout << "fill by 2" << endl;
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
	if (action == BROADCAST)
	{
		healRing->Draw(target);
	}
}

void FlowerPod::UpdateSprite()
{
	switch (action)
	{
	case IDLE:
		sprite.setTextureRect(ts_bud->GetSubRect(0));
		break;
	case ACTIVATE:
		sprite.setTextureRect(ts_rise->GetSubRect(frame/animFactor[ACTIVATE]));
		break;
	case BROADCAST:
		sprite.setTextureRect(ts_flower->GetSubRect( (frame / animFactor[BROADCAST]) % 12));
		break;
	case HIDE:
		sprite.setTextureRect(ts_rise->GetSubRect((actionLength[HIDE] - 1) - frame / animFactor[HIDE]));
		break;
	case DEACTIVATED:
		sprite.setTextureRect(ts_rise->GetSubRect(0));
		break;
	}

	//sprite.setTextureRect(ts->GetSubRect(0));//frame / animationFactor ) );
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

MomentaBroadcast::MomentaBroadcast( FlowerPod *p_pod, const std::string &btypeStr)
{

	ts_basicFlower = p_pod->owner->GetTileset("Momenta/momentaflower_320x288.png", 320, 288);
	initialFlowerLength = 120;
	//can currently handle 30 chars per line

	script = new Script;
	

	textDisp = new TextDisp( p_pod->owner );
	//textDisp->SetString(
	//	"hello this is a test hello this\n"
	//	"hello this is a test hello this\n"
	//	"hello this is a test hello this\n"
	//	"hello this is a test hello this");
	
	textDisp->SetTopLeft(Vector2f(1920 - textDisp->rectSize.x - 350, 50));

	bType = GetType(btypeStr);
	pod = p_pod;
	switch (bType)
	{
	case SEESHARDS:
	{
		ts_broadcast = pod->owner->GetTileset("Momenta/momentabroadcast_w1_1_320x288.png", 320, 288);
		script->Load("momenta1");
		numImages = script->numSections;
		imageLength = new int[numImages];
		for (int i = 0; i < numImages; ++i)
		{
			imageLength[i] = 4000;
		}
		/*imageLength[0] = 4000;
		imageLength[1] = 4000;
		imageLength[2] = 4000;
		imageLength[3] = 4000;
		imageLength[4] = 4000;
		imageLength[5] = 400;
		imageLength[6] = 400;
		imageLength[7] = 400;
		imageLength[8] = 400;
		imageLength[9] = 400;*/
		break;
	}	
	case DESTROYGOALS:
	{
		ts_broadcast = pod->owner->GetTileset("Momenta/momentabroadcast_w1_2_320x288.png", 320, 288);
		script->Load("momenta2");
		numImages = script->numSections;
		imageLength = new int[numImages];
		for (int i = 0; i < numImages; ++i)
		{
			imageLength[i] = 4000;
		}
		break;
	}
		
	}
	sprite.setTexture(*ts_broadcast->texture);
	
	
	Reset();
	
	//sprite.setTextureRect(ts_broadcast->GetSubRect(0));
	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	int xSpacing = 10;
	int ySpacing = 10;
	sprite.setPosition((1920 - ts_broadcast->tileWidth) - xSpacing, ySpacing);

	numPadding = 120;//60;
}

bool MomentaBroadcast::Update()
{
	if (basicFlower)
	{
		if (frame == initialFlowerLength)
		{
			if (imageIndex == numImages)
			{
				return false;
			}

			basicFlower = false;
			frame = 0;
			sprite.setTexture(*ts_broadcast->texture);
			textDisp->Show();
		}
		else
		{
			++frame;
			return true;
		}
	}



	if (!textDisp->Update() && !endPadding)
	{
		frame = imageLength[imageIndex] - numPadding;
		endPadding = true;
	}
	

	//need to figure out how to add a delay after the text is done. then the images will auto time
	//to the text

	++frame;
	if (frame == imageLength[imageIndex])
	{
		++imageIndex;
		if (imageIndex < script->numSections )
		{
			textDisp->SetString(script->GetSection(imageIndex));
		}
		
		frame = 0;
		endPadding = false;
		if (imageIndex == numImages)
		{
			basicFlower = true;
			textDisp->Hide();
			frame = 0;
			sprite.setTexture(*ts_basicFlower->texture);
			sprite.setTextureRect(ts_basicFlower->GetSubRect(0));
			return true;
			//return false;
		}
		sprite.setTextureRect(ts_broadcast->GetSubRect(imageIndex));
	}

	return true;
}

void MomentaBroadcast::Draw( RenderTarget *target )
{
	target->draw(sprite);
	textDisp->Draw(target);
}

void MomentaBroadcast::Reset()
{
	imageIndex = 0;
	frame = 0;
	textDisp->Reset();
	textDisp->SetString(script->GetSection(0));
	sprite.setTexture(*ts_basicFlower->texture);
	sprite.setTextureRect(ts_basicFlower->GetSubRect(0));
	endPadding = false;
	basicFlower = true;
}

MomentaBroadcast::BroadcastType MomentaBroadcast::GetType(const std::string &tStr)
{
	string testStr = tStr;
	std::transform(testStr.begin(), testStr.end(), testStr.begin(), ::tolower);
	if (testStr == "seeshards")
	{
		return SEESHARDS;
	}
	else if (testStr == "destroygoals")
	{
		return DESTROYGOALS;
	}
	else
	{
		return  SEESHARDS;
	}
}