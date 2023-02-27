#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "SaveFile.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "VisualEffects.h"
#include "PauseMenu.h"
#include "ShardMenu.h"
#include "Actor.h"
#include "PowerSequence.h"

#include "ParticleEffects.h"

#include "Enemy_PowerItem.h"
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

//std::map<std::string, ShardType> Shard::shardTypeMap;
//std::map<ShardType, std::string> Shard::shardStrMap;

//void Shard::UpdateParamsSettings()
//{
//	int oldShardType = shardType;
//
//	ShardParams *sParams = (ShardParams*)editParams;
//	shardWorld = sParams->shInfo.world;
//	localIndex = sParams->shInfo.localIndex;
//	shardType = Shard::GetShardType(shardWorld, localIndex);
//
//	if (shardType != oldShardType)
//	{
//		switch (shardWorld)
//		{
//		case 0://ShardType::SHARD_W1_TEACH_JUMP:
//		default:
//			ts = sess->GetTileset("Shard/shards_w1_192x192.png", 192, 192);
//			sprite.setTexture(*ts->texture);
//			sprite.setTextureRect(ts->GetSubRect(localIndex));
//			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
//			break;
//		}
//	}
//}

void PowerItem::Setup()
{
	caught = false;
	geoGroup.SetBase(GetPositionF());
}

PowerItem::PowerItem(ActorParams *ap)//Vector2i pos, int w, int li )
	:Enemy(EnemyType::EN_POWERITEM, ap)//, false, w+1 )
{
	SetNumActions(Count);
	SetEditorActions(FLOAT, FLOAT, 0);

	radius = 400;
	powerIndex = -1;

	powerIndex = ap->GetLevel() - 1;

	switch (powerIndex)
	{
	case 0:
		sprite.setColor(COLOR_BLUE);
		break;
	case 1:
		sprite.setColor(COLOR_GREEN);
		break;
	case 2:
		sprite.setColor(COLOR_YELLOW);
		break;
	case 3:
		sprite.setColor(COLOR_ORANGE);
		break;
	case 4:
		sprite.setColor(COLOR_RED);
		break;
	case 5:
		sprite.setColor(COLOR_MAGENTA);
		break;
	}

	ts = GetSizedTileset("Enemies/poweritem_128x128.png");

	//UpdateParamsSettings();

	alreadyCollected = false;

	//int upgradeIndex = Actor::UPGRADE_POWER_AIRDASH + powerIndex;
	/*if (sess->GetPlayer(0)->HasUpgrade(upgradeIndex))
	{
		alreadyCollected = true;
	}*/

	/*if (sess->IsShardCaptured(shardType))
	{
		alreadyCollected = true;
	}*/

	

	/*if (!alreadyCollected)
	{
		
	}*/

	sess->TryCreatePowerItemResources();

	testEmitter = NULL;
	ts_sparkle = NULL;
	ts_explodeCreate = NULL;
	sparklePool = NULL;

	powerSeq = NULL;

	

	testEmitter = new ShapeEmitter(6, 300);// PI / 2.0, 2 * PI, 1.0, 2.5);
	testEmitter->CreateParticles();
	testEmitter->SetPos(GetPositionF());
	testEmitter->SetRatePerSecond(30);

	ts_sparkle = GetSizedTileset("Menu/shard_sparkle_64x64.png");

	ts_explodeCreate = GetSizedTileset("FX/shard_explode_01_256x256.png");

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);

	hitBody.hitboxInfo = NULL;

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	geoGroup.Init();

	powerSeq = new GetPowerSequence;
	powerSeq->Init();
	powerSeq->powerItem = this;


	sprite.setTexture(*ts->texture);

	actionLength[FLOAT] = 120;
	actionLength[DISSIPATE] = 10;
	actionLength[LAUNCH] = 60;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;
	animFactor[LAUNCH] = 1;

	ResetEnemy();

	SetSpawnRect();
}

PowerItem::~PowerItem()
{
	if (testEmitter != NULL)
	{
		delete testEmitter;
	}

	if (sparklePool != NULL)
	{
		delete sparklePool;
	}

	if (powerSeq != NULL)
	{
		delete powerSeq;
		powerSeq = NULL;
	}
}

void PowerItem::ResetEnemy()
{
	//implement for power
	int upgradeIndex = Actor::UPGRADE_POWER_AIRDASH + powerIndex;
	alreadyCollected = sess->GetPlayer(0)->HasUpgrade(upgradeIndex);
	

	SetCurrPosInfo(startPosInfo);

	geoGroup.Reset();
	totalFrame = 0;

	if (sparklePool != NULL)
	{
		sparklePool->Reset();
	}

	action = FLOAT;
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();

	if (!alreadyCollected)
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

void PowerItem::FrameIncrement()
{
	++totalFrame;
}

void PowerItem::IHitPlayer(int index)
{
	if (action == FLOAT)
		DissipateOnTouch();
}

void PowerItem::DissipateOnTouch()
{
	action = DISSIPATE;
	frame = 0;

	sess->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, GetPosition(), true, 0, 12, 3, true);

	HitboxesOff();
	HurtboxesOff();

	powerSeq->Reset();
	sess->SetActiveSequence(powerSeq);

	Actor *player = sess->GetPlayer(0);
	player->SetAction(Actor::GETSHARD);
	player->frame = 0;
	player->velocity = V2d(0, 0);
	//Capture();
}

void PowerItem::Capture()
{
	int upgradeIndex = Actor::UPGRADE_POWER_AIRDASH + powerIndex;

	sess->UnlockUpgrade(upgradeIndex);

	if (powerIndex == 5)
	{
		//left wire also
		sess->UnlockUpgrade(upgradeIndex + 1);
	}

	sess->TrySaveCurrentSaveFile();
}

void PowerItem::DirectKill()
{

}

void PowerItem::Launch()
{
	action = LAUNCH;
	frame = 0;
}

void PowerItem::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
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
		int t = totalFrame % floatFrames;
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
	if (!alreadyCollected)
	{
		sparklePool->Update();
		if (!geoGroup.Update())
		{
			geoGroup.Reset();
			geoGroup.Start();
		}

		Vector2f sparkleCenter(GetPositionF());

		if (totalFrame % 60 == 0)
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

void PowerItem::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOAT)
	{
		DissipateOnTouch();
	}
}

void PowerItem::UpdateSprite()
{
	int tile = 0;

	sprite.setTextureRect(ts->GetSubRect(powerIndex));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void PowerItem::EnemyDraw(sf::RenderTarget *target)
{
	if (!alreadyCollected)
	{
		geoGroup.Draw(target);

		if (action != DISSIPATE)
		{
			target->draw(sprite);
			sparklePool->Draw(target);
		}

		//testEmitter->Draw(target);
	}
	else
	{
		target->draw(sprite);
	}
}

void PowerItem::DrawMinimap(sf::RenderTarget *target)
{
}

PowerPopup::PowerPopup()
{
	sess = Session::GetSession();

	powerIndex = -1;

	nameText.setCharacterSize(50);
	nameText.setFillColor(Color::Red);
	nameText.setFont(sess->mainMenu->arial);

	SetRectColor(bgQuad, Color(0, 0, 0, 200));

	Color borderColor = Color(100, 100, 100, 100);
	SetRectColor(topBorderQuad, borderColor);
	SetRectColor(powerBorderQuad, borderColor);

	ts_powers = sess->GetSizedTileset("Enemies/poweritem_128x128.png");
	powerSpr.setTexture(*ts_powers->texture);

	borderHeight = 2;

	width = 1400;

	powerBorder = 20;

	descBorder = Vector2f(10, 10);

	nameHeight = nameText.getFont()->getLineSpacing(nameText.getCharacterSize());

	tutBox = new TutorialBox(40, Vector2f(0, 0), Color::Transparent, Color::White, 0);

	float descLineHeight = tutBox->text.getFont()->getLineSpacing(tutBox->text.getCharacterSize());

	float extraHeight = 10;

	powerSize = 128;

	height = nameHeight + borderHeight + descLineHeight * 4 + descBorder.y * 2 + extraHeight;

	powerRel = Vector2f(powerBorder, nameHeight + borderHeight + powerBorder);
}

PowerPopup::~PowerPopup()
{
	delete tutBox;
}

void PowerPopup::Reset()
{
	state = SHOW;
	frame = 0;
}

void PowerPopup::Update()
{
	tutBox->UpdateButtonIconsWhenControllerIsChanged();
	++frame;
}

void PowerPopup::SetName(const std::string &name)
{
	nameText.setString(name);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, 0);
}

void PowerPopup::SetPower(int index)
{
	powerIndex = index;

	powerSpr.setTextureRect(ts_powers->GetSubRect(index));

	switch (index)
	{
	case 0:
		SetName("Airdash");
		tutBox->SetText("You unlocked Airdash!\n"
			"-Hold DASH in the air to hover!\n"
			"-Hold DASH and a direction to airdash in any of the 8 directions!");
		break;
	case 1:
		SetName("Reverse Gravity Cling");
		tutBox->SetText("You unlocked Reverse Gravity Cling!\n"
			"-Hold DASH and up while touching a ceiling to reverse gravity and stick!");
		break;
	case 2:
		SetName("Bounce Scorpion");
		tutBox->SetText("You unlocked Bounce Scorpion!\n"
			"Use RLEFT to change modes, and SHIELD to toggle!");
		break;
	case 3:
		SetName("Surface Grind");
		tutBox->SetText("You unlocked Surface Grind!\n"
			"Use RRIGHT to change modes, then hold SHIELD to move on any surface!");
		break;
	case 4:
		SetName("Time Slow Bubbles");
		tutBox->SetText("You unlocked Time Slow Bubbles!\n"
			"Use RDOWN to change modes, then press SHIELD to create time bubbles.\n"
			"Enemies are slowed down while in the bubbles. Hold SHIELD to slow down yourself too!");
		break;
	case 5:
		SetName("Double Wires");
		tutBox->SetText("You unlocked Double Wires!\n"
			"Use the triggers to swing and move around anywhere!");
		break;
	}
}

void PowerPopup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	SetRectTopLeft(bgQuad, width, height, topLeft);
	SetRectTopLeft(topBorderQuad, width, borderHeight, topLeft + Vector2f(0, nameHeight));

	powerSpr.setPosition(topLeft + powerRel);

	float remaining = height - nameHeight;

	float powerBorderLeft = powerBorder * 2 + powerSize;

	SetRectTopLeft(powerBorderQuad, borderHeight, remaining, topLeft + Vector2f(powerBorderLeft, nameHeight + borderHeight));

	Vector2f center = topLeft + Vector2f(width / 2, height / 2);

	nameText.setPosition(center.x, topLeft.y);

	tutBox->SetTopLeft(topLeft + Vector2f(powerBorderLeft + borderHeight + descBorder.x, nameHeight + borderHeight + descBorder.y));
}

void PowerPopup::SetCenter(sf::Vector2f &pos)
{
	SetTopLeft(Vector2f(pos.x - width / 2, pos.y - height / 2));
}

void PowerPopup::Draw(RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(topBorderQuad, 4, sf::Quads);
	target->draw(powerBorderQuad, 4, sf::Quads);

	target->draw(powerSpr);
	target->draw(nameText);

	tutBox->Draw(target);
}