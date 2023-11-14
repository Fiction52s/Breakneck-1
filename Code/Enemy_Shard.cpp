#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Shard.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "VisualEffects.h"
#include "PauseMenu.h"
#include "ShardMenu.h"
#include "Actor.h"
#include "ShardSequence.h"

#include "ParticleEffects.h"
#include "TutorialBox.h"
#include "PlayerRecord.h"

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

void Shard::UpdateParamsSettings()
{
	int oldShardType = shardType;

	ShardParams *sParams = (ShardParams*)editParams;
	shardWorld = sParams->shInfo.world;
	localIndex = sParams->shInfo.localIndex;
	shardType = GetShardTypeFromWorldAndIndex(shardWorld, localIndex);

	if (shardType != oldShardType)
	{
		ts = Shard::GetShardTileset(shardWorld, sess);

		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(localIndex));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	}
}

int Shard::GetShardTypeFromWorldAndIndex(int w, int li)
{
	return w * ShardInfo::MAX_SHARDS_PER_WORLD + li;
}

int Shard::GetNumShardsTotal()
{
	return ShardInfo::MAX_SHARDS;
	/*int total = 0;
	for (int i = 0; i < 8; ++i)
	{
		total += GetNumShardsForWorld(i);
	}

	return total;*/
}

int Shard::GetNumShardsForWorld(int w)
{
	switch (w)
	{
	case 0:
		return 10;
	case 1:
		return 10;
	case 2:
		return 10;
	case 3:
		return 12;
	case 4:
		return 11;
	case 5:
		return 14;
	case 6:
		return 0;
	case 7:
		return 0;
	}

	return 0;
}

void Shard::Setup()
{
	geoGroup.SetBase(GetPositionF());
}

Shard::Shard(ActorParams *ap )//Vector2i pos, int w, int li )
	:Enemy( EnemyType::EN_SHARD, ap )//, false, w+1 )
{
	SetNumActions(Count);
	SetEditorActions(FLOAT, FLOAT, 0);

	ShardParams *sParams = (ShardParams*)ap;
	editParams = ap;

	shardSeq = NULL;

	radius = 400;
	shardType = -1;
	//SetCurrPosInfo(startPosInfo);

	UpdateParamsSettings();

	data.alreadyCollected = false;

	sess->TryCreateShardResources();

	testEmitter = NULL;
	ts_sparkle = NULL;
	ts_explodeCreate = NULL;
	sparklePool = NULL;
	
	testEmitter = new ShapeEmitter(6, 300);// PI / 2.0, 2 * PI, 1.0, 2.5);
	testEmitter->CreateParticles();
	testEmitter->SetPos(GetPositionF());
	testEmitter->SetRatePerSecond(30);

	ts_sparkle = GetSizedTileset("Menu/shard_sparkle_64x64.png");

	ts_explodeCreate = GetSizedTileset("FX/shard_explode_01_256x256.png");

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	BasicCircleHurtBodySetup(64);
	BasicCircleHitBodySetup(64);

	hitBody.hitboxInfo = NULL;

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	geoGroup.Init();

	shardSeq = new GetShardSequence;
	shardSeq->Init();
	shardSeq->shard = this;
	
	

	actionLength[FLOAT] = 120;
	actionLength[DISSIPATE] = 10;
	actionLength[LAUNCH] = 60;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;
	animFactor[LAUNCH] = 1;

	ResetEnemy();

	SetSpawnRect();
}

Shard::~Shard()
{
	if (testEmitter != NULL)
	{
		delete testEmitter;
	}
	
	if (sparklePool != NULL)
	{
		delete sparklePool;
	}

	if (shardSeq != NULL)
	{
		delete shardSeq;
		shardSeq = NULL;
	}
}

void Shard::ResetEnemy()
{
	data.alreadyCollected = sess->IsShardCaptured(shardType);

	/*if (!data.alreadyCollected)
	{
		
	}
	else
	{
		if (!sess->IsShardCaptured(shardType))
		{
			data.alreadyCollected = true;
		}
	}*/
	


	SetCurrPosInfo(startPosInfo);

	geoGroup.Reset();
	data.totalFrame = 0;

	if (sparklePool != NULL)
	{
		sparklePool->Reset();
	}
	
	action = FLOAT;
	frame = 0;
	receivedHit.SetEmpty();

	UpdateHitboxes();

	UpdateSprite();

	if (!data.alreadyCollected)
	{
		SetHitboxes(&hitBody, 0);
		SetHurtboxes(&hurtBody, 0);

		sprite.setColor(Color(255, 255, 255, 255));
		testEmitter->Reset();
		testEmitter->SetOn(false);
	}
	else
	{
		sprite.setColor(Color(255, 255, 255, 40));
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);
	}

	rootPos = GetPosition();
}

Tileset *Shard::GetShardTileset(int w, TilesetManager *ttm)
{
	Tileset *ts = NULL;


	string tsName = "Shard/shards_w" + to_string(w + 1) + "_192x192.png";
	ts = ttm->GetSizedTileset(tsName);

	return ts;
}

void Shard::FrameIncrement()
{
	++data.totalFrame;
}

void Shard::IHitPlayer(int index)
{
	if( action == FLOAT )
		DissipateOnTouch();
}

void Shard::DissipateOnTouch()
{
	action = DISSIPATE;
	frame = 0;

	sess->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, GetPosition(), true, 0, 12, 3, true);

	HitboxesOff();
	HurtboxesOff();

	shardSeq->Reset();
	sess->SetActiveSequence(shardSeq);
	
	Actor *player = sess->GetPlayer(0);
	player->SetAction(Actor::GETSHARD);
	player->frame = 0;
	player->velocity = V2d(0, 0);
	//Capture();
}

void Shard::Capture()
{
	assert(shardType != -1);

	int upgradeIndex = shardType + Actor::SHARD_START_INDEX;

	sess->currShardField.SetBit(shardType, true);

	sess->UnlockUpgrade(upgradeIndex);

	sess->TrySaveCurrentSaveFile();
}

void Shard::DirectKill()
{

}

void Shard::Launch()
{
	action = LAUNCH;
	frame = 0;
}

void Shard::ProcessState()
{
	if( frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLOAT:
			frame = 0;
			break;
		case DISSIPATE:
			numHealth = 0;
			dead = true;
			Capture();
			break;
		case LAUNCH:
			action = FLOAT;
			frame = 0;
			break;
		}
	}


	if (action == FLOAT)
	{
		int floatFrames = 240;
		double floatAmount = 4.0;
		int t = data.totalFrame % floatFrames;
		float tf = t;
		tf /= (floatFrames - 1);
		double f = cos(2 * PI * tf);
		f -= .5;
		currPosInfo.position = rootPos;
		currPosInfo.position.y += f * floatAmount;
	}
	else if (action == LAUNCH)
	{
		currPosInfo.position.y -= 5.0;
		rootPos = currPosInfo.position;

		cout << "position.y: " << currPosInfo.position.y << endl;
	}
	
	//testEmitter->Update();
	if (!data.alreadyCollected)
	{
		sparklePool->Update();
		if (!geoGroup.Update())
		{
			geoGroup.Reset();
			geoGroup.Start();
		}

		Vector2f sparkleCenter(GetPositionF());

		if (data.totalFrame % 60 == 0)
		{
			Vector2f off(rand() % 101 - 50, rand() % 101 - 50);
			EffectInstance ei;

			int r = rand() % 3;
			if (r == 0)
			{
				ei.SetParams(sparkleCenter + off,
					Transform(Transform::Identity), 11, 5, 0);
			}
			else if (r == 1)
			{
				ei.SetParams(sparkleCenter + off,
					Transform(Transform::Identity), 10, 5, 11);
			}
			else if (r == 2)
			{
				ei.SetParams(sparkleCenter + off,
					Transform(Transform::Identity), 10, 5, 11);
			}

			sparklePool->ActivateEffect(&ei);
		}
	}
	

	
}

void Shard::ProcessHit()
{
	if (!dead && HasReceivedHit() && action == FLOAT)
	{
		DissipateOnTouch();
	}
}

void Shard::UpdateSprite()
{
	int tile = 0;

	//sprite.setScale(2, 2);
	//sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setPosition(GetPositionF());
}

void Shard::EnemyDraw( sf::RenderTarget *target )
{
	if (!data.alreadyCollected)
	{
		geoGroup.Draw(target);

		if (action != DISSIPATE)
		{
			target->draw(sprite);
			sparklePool->Draw(target);
		}

		testEmitter->Draw(target);
	}
	else
	{
		target->draw(sprite);
	}
}

void Shard::DrawMinimap( sf::RenderTarget *target )
{
}

int Shard::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Shard::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Shard::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

void Shard::SetExtraIDsAndAddToVectors()
{
	shardSeq->SetIDs();
}

ShardPopup::ShardPopup()
{
	//563 x 186
	sess = Session::GetSession();

	w = -1;
	li = -1;

	nameText.setCharacterSize(50);
	nameText.setFillColor(Color::Red);
	nameText.setFont(sess->mainMenu->arial);

	SetRectColor(bgQuad, Color(0, 0, 0, 200));

	Color borderColor = Color(100, 100, 100, 100);
	SetRectColor(topBorderQuad, borderColor);
	SetRectColor(shardBorderQuad, borderColor);
	
	borderHeight = 2;

	width = 1400;

	shardBorder = 20;

	descBorder = Vector2f(10, 10);

	nameHeight = nameText.getFont()->getLineSpacing(nameText.getCharacterSize());

	tutBox = new TutorialBox(40, Vector2f(0, 0), Color::Transparent, Color::White, 0);

	float descLineHeight = tutBox->text.getFont()->getLineSpacing(tutBox->text.getCharacterSize());

	float extraHeight = 10;

	shardSize = 192;

	height = nameHeight + borderHeight + descLineHeight * 4 + descBorder.y * 2 + extraHeight;

	shardRel = Vector2f(shardBorder, nameHeight + borderHeight + shardBorder );
}

ShardPopup::~ShardPopup()
{
	delete tutBox;
}

void ShardPopup::Reset()
{
	state = SHOW;
	frame = 0;
}

void ShardPopup::Update()
{
	tutBox->UpdateButtonIconsWhenControllerIsChanged();
	++frame;
}

void ShardPopup::SetShard(int p_w, int p_li)
{
	w = p_w;
	li = p_li;

	Tileset *ts_shard = Shard::GetShardTileset(w, sess);
	shardSpr.setTexture(*ts_shard->texture);
	shardSpr.setTextureRect(ts_shard->GetSubRect(li));
	//shardSpr.setOrigin(shardSpr.getLocalBounds().width / 2, shardSpr.getLocalBounds().height / 2);

	string nameStr = sess->shardMenu->GetShardName(w, li);
	nameText.setString(nameStr);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, 0);

	string descStr = sess->shardMenu->GetShardDesc(w, li);
	tutBox->SetText(descStr);
}

void ShardPopup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	SetRectTopLeft(bgQuad, width, height, topLeft);
	SetRectTopLeft(topBorderQuad, width, borderHeight, topLeft + Vector2f(0, nameHeight));

	shardSpr.setPosition(topLeft + shardRel);

	float remaining = height - nameHeight;

	float shardBorderLeft = shardBorder * 2 + shardSize;

	SetRectTopLeft(shardBorderQuad, borderHeight, remaining, topLeft + Vector2f(shardBorderLeft, nameHeight + borderHeight));


	Vector2f center = topLeft + Vector2f(width / 2, height / 2);
	
	nameText.setPosition(center.x, topLeft.y);

	tutBox->SetTopLeft(topLeft + Vector2f(shardBorderLeft + borderHeight + descBorder.x, nameHeight + borderHeight + descBorder.y));
}

void ShardPopup::SetCenter(sf::Vector2f &pos)
{
	SetTopLeft(Vector2f( pos.x - width / 2, pos.y - height / 2));
}

void ShardPopup::Draw(RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(topBorderQuad, 4, sf::Quads);
	target->draw(shardBorderQuad, 4, sf::Quads);

	target->draw(shardSpr);
	target->draw(nameText);

	tutBox->Draw(target);
}