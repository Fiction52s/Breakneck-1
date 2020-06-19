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
#include "MovingGeo.h"
#include "Enemy_Shard.h"
#include "Actor.h"

using namespace std;
using namespace sf;

FlowerPod::FlowerPod(ActorParams* ap)//const std::string &typeStr, Edge *g, double q)
	:Enemy(EnemyType::EN_FLOWERPOD, ap )//, false, 0, false), ground(g),
	//edgeQuantity(q)
{
	FlowerPodParams *fpParams = (FlowerPodParams*)ap;

	if (sess->IsSessTypeGame())
	{
		game = GameSession::GetSession();
	}
	else
	{
		game = NULL;
	}

	//healRingGroup.AddGeo(new Ring(32, 20, 200, 10, 20, Vector2f(pos), Vector2f(pos),
	//	Color::Cyan, Color(0, 0, 100, 0), 60));
	//geoGroup.Init();


	ts_flower = sess->GetSizedTileset("Momenta/momentaflower_128x128.png");
	ts_bud = sess->GetSizedTileset("Momenta/momentabud_128x128.png");
	ts_rise = sess->GetSizedTileset("Momenta/momentaflower_rise_128x128.png");

	sprite.setPosition(startPosInfo.GetPositionF());

	V2d gn = startPosInfo.GetEdge()->Normal();

	V2d gAlong = startPosInfo.GetEdge()->Along();//normalize(g->v1 - g->v0);

	camPosition = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), ts_flower->tileHeight );
	
	SetOffGroundHeight(ts_flower->tileHeight / 2.0);

	SetCurrPosInfo(startPosInfo);

	healRing = new Ring(32);
	healRing->CreatePoints();

	

	//podType = GetType(typeStr);
	broadcast = new MomentaBroadcast( this, fpParams->typeStr);
	/*switch (podType)
	{
	case SEESHARDS:
		storySeq = new StorySequence(owner);
		storySeq->Load("getairdash");
		break;
	}*/

	
	sprite.setTexture(*ts_bud->texture);
	sprite.setTextureRect(ts_bud->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height -16);
	

	//healRing->Init();
	healRing->SetColor(Color::Cyan);
	healRing->Set(currPosInfo.GetPositionF(), 100, 110);

	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());

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


	V2d position = GetPosition();
	//raycast here to either the terrain above me or a max height
	rayStart = position;
	rayEnd = position + gn * 2000.0;
	rcEdge = NULL;
	RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

	double boxHeight = 1000;
	if (rcEdge != NULL)
	{
		boxHeight = length(rcEdge->GetPosition(rcQuantity) - startPosInfo.GetPosition());
	}

	BasicRectHitBodySetup(100, boxHeight / 2.0, atan2(-gAlong.y, -gAlong.x),
		V2d(0, boxHeight / 2.0 - ts_flower->tileHeight / 2.0), position);

	ResetEnemy();
}

FlowerPod::~FlowerPod()
{
	delete healRing;
	delete broadcast;
}

void FlowerPod::ResetEnemy()
{
	action = IDLE;
	sprite.setTexture(*ts_bud->texture);
	dead = false;
	frame = 0;
	SetHitboxes(&hitBody, 0);
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
		SetHitboxes(NULL, 0);
	}
}



void FlowerPod::HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion)
{
	V2d dir = normalize(rayEnd - rayStart);
	V2d pos = edge->GetPosition(edgeQuantity);
	double along = dot(dir, edge->Normal());
	if (along < 0 && (rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - rayStart) <
		length(rcEdge->GetPosition(rcQuantity) - rayStart)))
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
			/*if( game != NULL )
				game->currBroadcast = broadcast;*/
			break;
		case BROADCAST:
			
			frame = 0;
			/*if (game != NULL && game->currBroadcast == NULL)
			{
				action = HIDE;
				sprite.setTexture(*ts_rise->texture);
			}*/
		
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

	Actor *player = sess->GetPlayer(0);
	double dist = length(player->position - GetPosition());
	float rad = healRing->innerRadius;
	
	if ( action == BROADCAST && dist <= rad && healingPlayer == NULL)
	{
		if (!sess->cam.manual)
			sess->cam.Ease(Vector2f(camPosition), .8, 300, CubicBezier());
		healingPlayer = player;
		//enter
	}
	else if (  (dist > rad || action != BROADCAST ) && healingPlayer != NULL )
	{
		if (sess->cam.manual)
		{
			sess->cam.EaseOutOfManual(60);
		}
		healingPlayer = NULL;
		//exit
	}

	if (healingPlayer != NULL && healingPlayer->drainCounter == 0)
	{
		sess->GetPlayer(0)->kinRing->powerRing->Fill( player->drainAmount + 1 );//powerWheel->Use( 1 );	
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
	DrawSprite(target, sprite);
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

void FlowerPod::DirectKill()
{

}

MomentaBroadcast::MomentaBroadcast( FlowerPod *p_pod, const std::string &btypeStr)
{
	pod = p_pod;
	ts_basicFlower = pod->sess->GetTileset("Momenta/momentaflower_320x288.png", 320, 288);
	initialFlowerLength = 120;
	//can currently handle 30 chars per line
	
	givenShard = NULL;
	conv = new Conversation;
	
	//textDisp = new TextDisp( pod->owner, 670, 220 );
	
	//textDisp->SetTopLeft(Vector2f(1920 - textDisp->rectSize.x - 350, 50));

	bType = GetType(btypeStr);

	ShardParams sp(pod->sess->types["shard"], 0);// Vector2i(pod->GetPosition()));
	sp.MoveTo(Vector2i( pod->GetPosition() ));
	sp.shInfo.world = 0;
	sp.shInfo.localIndex = 0;
	
	switch (bType)
	{
	case SEESHARDS:
	{
		ts_broadcast = pod->sess->GetTileset("Momenta/momentabroadcast_w1_1_320x288.png", 320, 288);
		conv->Load("momenta1");
		//textDisp->Load("momenta1");
			//script->Load("momenta1");
			//numImages = 5;//script->numSections;
		//imageLength = new int[numImages];
		//for (int i = 0; i < numImages; ++i)
		//{
		//	imageLength[i] = 4000;
		//}
		givenShard = new Shard(&sp);
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
		ts_broadcast = pod->sess->GetTileset("Momenta/momentabroadcast_w1_2_320x288.png", 320, 288);
		conv->Load("momenta2");
		//script->Load("momenta2");
		//numImages = script->numSections;
		//imageLength = new int[numImages];
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

MomentaBroadcast::~MomentaBroadcast()
{
	//delete script;
	delete conv;

	/*if (numImages > 0)
	{
		assert(imageLength != NULL);
		delete[] imageLength;
	}*/
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
			conv->Show();
		}
		else
		{
			++frame;
			return true;
		}
	}


	bool textContinue = conv->Update();

	conv->NextSection();

	if (!textContinue && conv->show)// && !endPadding)
	{
		conv->Hide();
		if (givenShard != NULL)
		{
			Shard *s = givenShard;
			pod->sess->AddEnemy(s);
			s->Launch();
		}
		return false;
		//frame = //imageLength[imageIndex] - numPadding;
		//endPadding = true;
	}
	

	//need to figure out how to add a delay after the text is done. then the images will auto time
	//to the text

	//++frame;
	//if (frame == imageLength[imageIndex])
	//{
	//	++imageIndex;
	//	if (imageIndex < script->numSections )
	//	{
	//		textDisp->SetString(script->GetSection(imageIndex));
	//	}
	//	
	//	frame = 0;
	//	endPadding = false;
	//	if (imageIndex == numImages)
	//	{
	//		basicFlower = true;
	//		textDisp->Hide();
	//		frame = 0;
	//		sprite.setTexture(*ts_basicFlower->texture);
	//		sprite.setTextureRect(ts_basicFlower->GetSubRect(0));
	//		return true;
	//		//return false;
	//	}
	//	sprite.setTextureRect(ts_broadcast->GetSubRect(imageIndex));
	//}

	return true;
}

void MomentaBroadcast::Draw( RenderTarget *target )
{
	//target->draw(sprite);
	conv->Draw(target);
}

void MomentaBroadcast::Reset()
{
	imageIndex = 0;
	frame = 0;
	conv->Reset();
	//textDisp->SetString(script->GetSection(0));
	sprite.setTexture(*ts_basicFlower->texture);
	sprite.setTextureRect(ts_basicFlower->GetSubRect(0));
	endPadding = false;
	basicFlower = true;

	if (givenShard != NULL)
	{
		givenShard->Reset();
	}
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