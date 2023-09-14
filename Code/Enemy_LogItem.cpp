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
#include "Actor.h"
#include "LogSequence.h"

#include "ParticleEffects.h"

#include "Enemy_LogItem.h"
#include "TutorialBox.h"
#include "LogMenu.h"

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

void LogItem::UpdateParamsSettings()
{
	int oldlogType = logType;

	LogParams *lParams = (LogParams*)editParams;
	logWorld = lParams->lInfo.world;
	localIndex = lParams->lInfo.localIndex;

	tile = sess->logMenu->GetLogTile(logWorld, localIndex);
	sprite.setTextureRect(ts->GetSubRect(tile));
	logType = logWorld * LogInfo::MAX_LOGS_PER_WORLD + localIndex;//GetlogTypeFromWorldAndIndex(logWorld, localIndex);//logWorld * 22 + localIndex;//log::GetlogType(logWorld, localIndex);

	/*if (logType != oldlogType)
	{
		ts = log::GetlogTileset(logWorld, sess);

		sprite.setTexture(*ts->texture);
		
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	}*/
}

void LogItem::Setup()
{
	geoGroup.SetBase(GetPositionF());
}

LogItem::LogItem(ActorParams *ap)//Vector2i pos, int w, int li )
	:Enemy(EnemyType::EN_LOGITEM, ap)//, false, w+1 )
{
	SetNumActions(Count);
	SetEditorActions(FLOAT, FLOAT, 0);

	radius = 400;
	logType = 0;

	logSeq = NULL;

	ts = GetSizedTileset("Logs/logs_64x64.png");
	ts_shine = GetSizedTileset("Logs/logs_shine_64x64.png");
	UpdateParamsSettings();

	data.alreadyCollected = false;

	sess->TryCreateLogResources();

	sprite.setScale(1.5, 1.5);
	shineSprite.setScale(1.5, 1.5);

	/*if (sess->IslogCaptured(logType))
	{
	data.alreadyCollected = true;
	}*/

	//if (!data.alreadyCollected)
	//{
	//	sess->TryCreatePowerItemResources();
	//}


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

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);

	hitBody.hitboxInfo = NULL;


	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	geoGroup.Init();

	logSeq = new GetLogSequence;
	logSeq->Init();
	logSeq->log = this;

	sprite.setTexture(*ts->texture);
	shineSprite.setTexture(*ts_shine->texture);

	actionLength[FLOAT] = 120;
	actionLength[DISSIPATE] = 10;
	actionLength[LAUNCH] = 60;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;
	animFactor[LAUNCH] = 1;

	if (sess->IsSessTypeGame())
	{
		logPreview = new LogPreview;
		logPreview->SetInfo(sess->logMenu->GetLogInfo(world, localIndex));
	}
	else
	{
		logPreview = NULL;
	}

	ResetEnemy();

	SetSpawnRect();
}

LogItem::~LogItem()
{
	if (testEmitter != NULL)
	{
		delete testEmitter;
	}

	if (sparklePool != NULL)
	{
		delete sparklePool;
	}

	if (logSeq != NULL)
	{
		delete logSeq;
		logSeq = NULL;
	}

	if (logPreview != NULL)
	{
		delete logPreview;
	}

}

void LogItem::ResetEnemy()
{
	data.alreadyCollected = sess->HasLog(logType);

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

void LogItem::FrameIncrement()
{
	++data.totalFrame;
}

void LogItem::IHitPlayer(int index)
{
	if (action == FLOAT)
		DissipateOnTouch();
}

void LogItem::DissipateOnTouch()
{
	action = DISSIPATE;
	frame = 0;

	/*sess->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, GetPosition(), true, 0, 12, 3, true);*/

	HitboxesOff();
	HurtboxesOff();

	Capture();

	logSeq->Reset();
	sess->SetActiveSequence(logSeq);

	Actor *player = sess->GetPlayer(0);
	player->SetAction(Actor::GETSHARD);
	player->frame = 0;
	player->velocity = V2d(0, 0);
}

void LogItem::Capture()
{
	sess->UnlockLog(logType);

	sess->TrySaveCurrentSaveFile();
}

void LogItem::DirectKill()
{

}

void LogItem::Launch()
{
	action = LAUNCH;
	frame = 0;
}

void LogItem::ProcessState()
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

void LogItem::ProcessHit()
{
	if (!dead && HasReceivedHit() && action == FLOAT)
	{
		DissipateOnTouch();
	}
}

void LogItem::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	int shineAnimFactor = 2;
	int shineTile = (data.totalFrame / shineAnimFactor) % (32);

	shineSprite.setTextureRect(ts_shine->GetSubRect(shineTile));
	shineSprite.setOrigin(shineSprite.getLocalBounds().width / 2, shineSprite.getLocalBounds().height / 2);
	shineSprite.setPosition(GetPositionF());
}

void LogItem::EnemyDraw(sf::RenderTarget *target)
{
	if (!data.alreadyCollected)
	{
		geoGroup.Draw(target);

		if (action != DISSIPATE)
		{
			target->draw(sprite);
			target->draw(shineSprite);
			sparklePool->Draw(target);
		}
		//testEmitter->Draw(target);
	}
	else
	{
		target->draw(sprite);
	}
}

void LogItem::DrawMinimap(sf::RenderTarget *target)
{
}

int LogItem::GetLogTypeFromWorldAndIndex(int w, int li)
{
	return w * LogInfo::MAX_LOGS_PER_WORLD + li;
}

int LogItem::GetNumLogsTotal()
{
	return LogInfo::MAX_LOGS_PER_WORLD * 8;
}

int LogItem::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void LogItem::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void LogItem::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

void LogItem::SetSequenceIDsAndAddThemToAllSequencesVec()
{
	logSeq->SetIDAndAddToAllSequencesVec();
}

LogPopup::LogPopup()
{
	//563 x 186
	sess = Session::GetSession();

	logWorld = -1;
	logLocalIndex = -1;

	ts_log = sess->GetSizedTileset("Logs/logs_64x64.png");

	if (sess->IsSessTypeEdit())
	{
		logPreview = new LogPreview;
	}
	else
	{
		logPreview = NULL;
	}

	nameText.setCharacterSize(50);
	nameText.setFillColor(Color::Red);
	nameText.setFont(sess->mainMenu->arial);

	Color bgColor = Color(0, 0, 0, 200);

	SetRectColor(bgQuad, bgColor);
	SetRectColor(previewBGQuad, bgColor);

	Color borderColor = Color(100, 100, 100, 100);
	SetRectColor(topBorderQuad, borderColor);

	borderHeight = 2;

	width = 1400;

	logBorder = 20;

	descBorder = Vector2f(10, 10);

	nameHeight = nameText.getFont()->getLineSpacing(nameText.getCharacterSize());

	tutBox = new TutorialBox(40, Vector2f(0, 0), Color::Transparent, Color::White, 0);

	float descLineHeight = tutBox->text.getFont()->getLineSpacing(tutBox->text.getCharacterSize());

	float extraHeight = 10;

	logSize = 192;

	previewBGWidth = 400;
	previewBGHeight = 400;

	height = nameHeight + borderHeight + descLineHeight * 4 + descBorder.y * 2 + extraHeight;

	logRel = Vector2f(logBorder, nameHeight + borderHeight + logBorder);
}

LogPopup::~LogPopup()
{
	delete tutBox;

	if (logPreview != NULL && sess->IsSessTypeEdit())
	{
		delete logPreview;
	}
}

void LogPopup::Reset()
{
	state = SHOW;
	frame = 0;
}

void LogPopup::Update()
{
	tutBox->UpdateButtonIconsWhenControllerIsChanged();
	logPreview->Update();

	++frame;
}

void LogPopup::SetLog( int w, int i )
{
	logWorld = w;
	logLocalIndex = i;

	ts_log->SetSubRect(logSpr, sess->logMenu->GetLogTile(logWorld, logLocalIndex));

	string nameStr = sess->logMenu->GetLogName(logWorld, logLocalIndex);
	nameText.setString(nameStr);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, 0);

	string descStr = sess->logMenu->GetLogDesc(logWorld, logLocalIndex);
	tutBox->SetText(descStr);

	/*if (sess->IsSessTypeEdit())
	{
		logPreview->SetInfo(sess->logMenu->GetLogInfo(logWorld, logLocalIndex));
	}*/
}

void LogPopup::SetInfoInEditor()
{
	//loads resources in the editor. meant to be called right when freezing so the hitch doesn't feel weird when hitting the log
	if (sess->IsSessTypeEdit())
	{
		logPreview->SetInfo(sess->logMenu->GetLogInfo(logWorld, logLocalIndex));
	}
}

void LogPopup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	float previewBottomBorder = 100;

	Vector2f previewQuadPos = topLeft + Vector2f(width / 2 - previewBGWidth / 2, -previewBGHeight - previewBottomBorder);

	SetRectTopLeft(previewBGQuad, previewBGWidth, previewBGHeight, previewQuadPos);

	logPreview->SetCenter(previewQuadPos + Vector2f(previewBGWidth / 2, previewBGHeight / 2));

	SetRectTopLeft(bgQuad, width, height, topLeft);
	SetRectTopLeft(topBorderQuad, width, borderHeight, topLeft + Vector2f(0, nameHeight));

	logSpr.setPosition(topLeft + logRel);

	float remaining = height - nameHeight;

	Vector2f center = topLeft + Vector2f(width / 2, height / 2);

	nameText.setPosition(center.x, topLeft.y);

	tutBox->SetTopLeft(topLeft + Vector2f(borderHeight + descBorder.x, nameHeight + borderHeight + descBorder.y));
}

void LogPopup::SetCenter(sf::Vector2f &pos)
{
	SetTopLeft(Vector2f(pos.x - width / 2, pos.y - height / 2));
}

void LogPopup::Draw(RenderTarget *target)
{
	target->draw(previewBGQuad, 4, sf::Quads);

	target->draw(bgQuad, 4, sf::Quads);
	target->draw(topBorderQuad, 4, sf::Quads);
	
	logPreview->Draw(target);

	target->draw(logSpr);
	target->draw(nameText);

	tutBox->Draw(target);
}