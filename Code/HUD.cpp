#include "HUD.h"
#include "GameSession.h"
#include "PowerOrbs.h"
#include "Keymarker.h"
#include "Minimap.h"
#include "Momentumbar.h"
#include "PowerOrbs.h"
#include "SaveFile.h"
#include "Actor.h"
#include "Session.h"
#include "MainMenu.h"
#include "PowerSelectorHUD.h"
#include "BossHealth.h"
#include "TimerHUD.h"
#include "MapHeader.h"

using namespace sf;
using namespace std;

HUD::HUD()
{
	sess = Session::GetSession();
	mini = new Minimap;
}

HUD::~HUD()
{
	delete mini;
}

AdventureHUD::AdventureHUD()
{
	hType = HUDType::ADVENTURE;

	timer = new TimerHUD(false);
	modifierTimer = new TimerHUD(true);

	keyMarkers.push_back(new KeyMarker);
	keyMarkers.push_back(new KeyMarker);

	powerSelector = new PowerSelector;

	timerShowPos = Vector2f(1920 / 2, 50);

	timerHidePos = timerShowPos + Vector2f(0, -200);

	powerSelectorShowPos = Vector2f(288, 140);
	powerSelectorHidePos = Vector2f(288-500, 140);

	flyCountTextShowPos = Vector2f(1920 - 30, 10);
	flyCountTextHidePos = Vector2f((1920 - 30) + 500, 10);

	flyCountText.setCharacterSize(40);
	flyCountText.setFont(sess->mainMenu->arial);
	flyCountText.setFillColor(Color::White);
	flyCountText.setString("x100");
	flyCountText.setOrigin(flyCountText.getLocalBounds().left +
		flyCountText.getLocalBounds().width, 0);
	flyCountText.setString("x0");
	flyCountText.setPosition(flyCountTextShowPos);

	ts_go = sess->GetSizedTileset("Zone/gate_orb_64x64.png");
	ts_go->SetSpriteTexture(goSpr);
	ts_go->SetSubRect(goSpr, 2);

	goSpr.setOrigin(goSpr.getLocalBounds().width, 0);
	

	miniShowPos = mini->minimapSprite.getPosition();
	miniHidePos = Vector2f(-500, miniShowPos.y);

	kinMask = sess->GetPlayer(0)->kinMask;

	kinMaskShowPos = kinMask->GetTopLeft();
	kinMaskHidePos = Vector2f(-500, kinMaskShowPos.y);

	keyMarkerShowPos = Vector2f(1920 - 50, 50);//keyMarkers[0]->GetPosition();
	keyMarkerHidePos = Vector2f(1920 + 200, keyMarkerShowPos.y);

	bossHealthShowPos = Vector2f(1920 - 100, 200);
	bossHealthHidePos = bossHealthShowPos + Vector2f(500, 0);

	keyMarkerYOffset = 80;

	bossHealthBar = NULL;
	
	/*momentumShowPos = momentumBar->GetTopLeft();
	momentumHidePos = Vector2f(-200, momentumShowPos.y);*/

	Reset();
}

AdventureHUD::~AdventureHUD()
{
	for (auto it = keyMarkers.begin(); it != keyMarkers.end(); ++it)
	{
		delete (*it);
	}

	delete powerSelector;

	delete timer;
	delete modifierTimer;
}

void AdventureHUD::UpdateKeyNumbers()
{
	if (keyMarkers[0]->markerType == KeyMarker::KEY)
	{
		keyMarkers[0]->UpdateKeyNumbers();
	}
}

void AdventureHUD::CheckForGo()
{
	if (numActiveKeyMarkers > 0)
	{
		if (sess->currentZone->secretZone)
		{
			return;
		}

		//wait until vibrating is done!
		for (int i = 0; i < numActiveKeyMarkers; ++i)
		{
			if (keyMarkers[i]->action == KeyMarker::VIBRATING)
			{
				return;
			}
		}

		bool allGatesSatisfied = true;

		int numKeys = sess->GetPlayer(0)->numKeysHeld;
		int numEnemiesRemaining = sess->currentZone->GetNumRemainingKillableEnemies();

		for (auto it = sess->currentZone->gates.begin(); it != sess->currentZone->gates.end(); ++it)
		{
			Gate *g = (Gate*)(*it)->info;
			if (g->IsLockedForever() )
			{
				continue;
			}

			if (g->category == Gate::ALLKEY || g->category == Gate::NUMBER_KEY)
			{
				if (g->numToOpen > numKeys)
				{
					allGatesSatisfied = false;
					break;
				}
			}
			else if (g->category == Gate::ENEMY)
			{
				if (numEnemiesRemaining > 0 )
				{
					allGatesSatisfied = false;
					break;
				}
			}
		}

		if (allGatesSatisfied)
		{
			numActiveKeyMarkers = 0;
		}
	}
}


void AdventureHUD::UpdateEnemyNumbers()
{
	if (numActiveKeyMarkers > 0 && keyMarkers[0]->markerType == KeyMarker::ENEMY)
	{
		keyMarkers[0]->UpdateKeyNumbers();
	}
	else if (numActiveKeyMarkers == 2 && keyMarkers[1]->markerType == KeyMarker::ENEMY)
	{
		keyMarkers[1]->UpdateKeyNumbers();
	}
}

void AdventureHUD::SetBossHealthBar(BossHealth *bh)
{
	bossHealthBar = bh;

	if (state == SHOWN)
	{
		bossHealthBar->SetTopLeft(bossHealthShowPos);
	}
	else if (state == HIDDEN)
	{
		bossHealthBar->SetTopLeft(bossHealthHidePos);
	}
	else
	{
		//todo
	}
}

void AdventureHUD::Hide(int frames)
{
	if (frames == 0)
	{
		state = HIDDEN;
		frame = 0;
		mini->SetCenter(miniHidePos);
		for (int i = 0; i < keyMarkers.size(); ++i)
		{
			//keyMarkers[i]->SetPosition(keyMarkerHidePos + Vector2f(0, i * keyMarkerYOffset));
			keyMarkers[i]->SetTopRight(keyMarkerHidePos + Vector2f(0, i * keyMarkerYOffset));
		}
		kinMask->SetTopLeft(kinMaskHidePos);
		powerSelector->SetPosition(powerSelectorHidePos);
		flyCountText.setPosition(flyCountTextHidePos);
		timer->SetCenter(timerHidePos);
		if (bossHealthBar != NULL)
		{
			bossHealthBar->SetTopLeft(bossHealthHidePos);
		}
	}
	else
	{
		processFrames = frames;
		state = EXITING;
		frame = 0;
	}
}

void AdventureHUD::Show(int frames)
{
	if (frames == 0)
	{
		state = SHOWN;
		frame = 0;
		mini->SetCenter(miniShowPos);
		kinMask->SetTopLeft(kinMaskShowPos);
		for (int i = 0; i < keyMarkers.size(); ++i)
		{
			//keyMarkers[i]->SetPosition(keyMarkerShowPos + Vector2f(0, i * keyMarkerYOffset));
			keyMarkers[i]->SetTopRight(keyMarkerShowPos + Vector2f(0, i * keyMarkerYOffset));
		}
		flyCountText.setPosition(flyCountTextShowPos);
		timer->SetCenter(timerShowPos);
		powerSelector->SetPosition(powerSelectorShowPos);
		goSpr.setPosition(keyMarkerShowPos);
		if (bossHealthBar != NULL)
		{
			bossHealthBar->SetTopLeft(bossHealthShowPos);
		}
	}
	else
	{
		processFrames = frames;
		state = ENTERING;
		frame = 0;
	}
}

bool AdventureHUD::IsHidden()
{
	return state == HIDDEN;
}

bool AdventureHUD::IsShown()
{
	return state == SHOWN;
}

void AdventureHUD::Update()
{
	switch (state)
	{
	case SHOWN:
		frame = 0;
		break;
	case ENTERING:
		if (frame == processFrames)
		{
			state = SHOWN;
			frame = 0;
			mini->SetCenter(miniShowPos);
			kinMask->SetTopLeft(kinMaskShowPos);
			for (int i = 0; i < keyMarkers.size(); ++i)
			{
				//keyMarkers[i]->SetPosition(keyMarkerShowPos + Vector2f(0, i * keyMarkerYOffset));
				keyMarkers[i]->SetTopRight(keyMarkerShowPos + Vector2f(0, i * keyMarkerYOffset));
			}
			flyCountText.setPosition(flyCountTextShowPos);
			powerSelector->SetPosition(powerSelectorShowPos);
			timer->SetCenter(timerShowPos);
			if (bossHealthBar != NULL)
			{
				bossHealthBar->SetTopLeft(bossHealthShowPos);
			}
			//momentumBar->SetTopLeft(momentumShowPos);
		}
		else
		{
			double prog = frame/(double)processFrames;
			float a = showBez.GetValue(prog);	
			Vector2f center = miniHidePos * (1.f - a) + a * miniShowPos;
			mini->SetCenter(center);
			Vector2f topLeft = kinMaskHidePos * (1.f - a) + a * kinMaskShowPos;
			kinMask->SetTopLeft(topLeft);
			Vector2f keyMarkerPos = keyMarkerHidePos * (1.f - a) + a * keyMarkerShowPos;
			for (int i = 0; i < keyMarkers.size(); ++i)
			{
				//keyMarkers[i]->SetPosition(neededCenter + Vector2f(0, i * keyMarkerYOffset));
				keyMarkers[i]->SetTopRight(keyMarkerPos + Vector2f(0, i * keyMarkerYOffset));
			}
			goSpr.setPosition(keyMarkerPos);
			Vector2f countPos = flyCountTextHidePos * (1.f - a) + a * flyCountTextShowPos;
			flyCountText.setPosition(countPos);
			Vector2f powerPos = powerSelectorHidePos * (1.f - a) + a * powerSelectorShowPos;
			powerSelector->SetPosition(powerPos);

			Vector2f bossHealthPos = bossHealthHidePos * (1.f - a) + a * bossHealthShowPos;

			if (bossHealthBar != NULL)
			{
				bossHealthBar->SetTopLeft(bossHealthPos);
			}

			Vector2f timerPos = timerHidePos * (1.f - a) + a * timerShowPos;

			timer->SetCenter(timerPos);
		}
		break;
	case EXITING:
		if (frame == processFrames)
		{
			state = HIDDEN;
			frame = 0;
			mini->SetCenter(miniHidePos);
			for (int i = 0; i < keyMarkers.size(); ++i)
			{
				//keyMarkers[i]->SetPosition(keyMarkerHidePos + Vector2f(0, i * keyMarkerYOffset));
				keyMarkers[i]->SetTopRight(keyMarkerHidePos + Vector2f(0, i * keyMarkerYOffset));
			}
			goSpr.setPosition(keyMarkerHidePos);
			kinMask->SetTopLeft(kinMaskHidePos);
			flyCountText.setPosition(flyCountTextHidePos);
			powerSelector->SetPosition(powerSelectorHidePos);
			if (bossHealthBar != NULL)
			{
				bossHealthBar->SetTopLeft(bossHealthHidePos);
			}
			timer->SetCenter(timerHidePos);
		}
		else
		{
			double prog = frame / (double)processFrames;
			float a = showBez.GetValue(prog);
			Vector2f center = miniShowPos * (1.f - a) + a * miniHidePos;
			mini->SetCenter(center);
			Vector2f topLeft = kinMaskShowPos * (1.f - a) + a * kinMaskHidePos;
			kinMask->SetTopLeft(topLeft);
			Vector2f keyMarkerPos = keyMarkerShowPos * (1.f - a) + a * keyMarkerHidePos;
			for (int i = 0; i < keyMarkers.size(); ++i)
			{
				//keyMarkers[i]->SetPosition(neededCenter + Vector2f(0, i * keyMarkerYOffset));
				keyMarkers[i]->SetTopRight(keyMarkerPos + Vector2f(0, i * keyMarkerYOffset));
			}
			goSpr.setPosition(keyMarkerPos);
			Vector2f countPos = flyCountTextShowPos * (1.f - a) + a * flyCountTextHidePos;
			flyCountText.setPosition(countPos);
			Vector2f powerPos = powerSelectorShowPos * (1.f - a) + a * powerSelectorHidePos;
			powerSelector->SetPosition(powerPos);

			Vector2f bossHealthPos = bossHealthShowPos * (1.f - a) + a * bossHealthHidePos;

			if (bossHealthBar != NULL)
			{
				bossHealthBar->SetTopLeft(bossHealthPos);
			}

			Vector2f timerPos = timerShowPos * (1.f - a) + a * timerHidePos;

			timer->SetCenter(timerPos);
		}
		break;
	case HIDDEN:
		frame = 0;
		break;
	}

	mini->Update();

	for (int i = 0; i < keyMarkers.size(); ++i)
	{
		keyMarkers[i]->Update();
	}

	CheckForGo();
	/*if (state != HIDDEN)
	{
		
	}*/

	if (bossHealthBar != NULL)
	{
		bossHealthBar->Update();
	}
	//healthBar->Update();

	
	timer->SetNumFrames(sess->GetPlayer(0)->numFramesToLive);
	timer->Update();

	modifierTimer->SetCenter(timer->GetRightCenter() + Vector2f(100, 0 ));
	modifierTimer->Update();

	++frame;
}


void AdventureHUD::Reset()
{
	show = true;
	state = SHOWN;
	frame = 0;

	numActiveKeyMarkers = 0;

	for (int i = 0; i < keyMarkers.size(); ++i)
	{
		keyMarkers[i]->Reset();
	}

	timer->Reset();
	modifierTimer->Reset();

	Show();

	//mini->SetCenter(miniShowPos);
	//kinMask->SetTopLeft(kinMaskShowPos);
	/*for (int i = 0; i < keyMarkers.size(); ++i)
	{
		keyMarkers[i]->SetPosition(keyMarkerShowPos + Vector2f(0, i * keyMarkerYOffset));
	}*/
	
	//goSpr.setPosition(keyMarkerShowPos);

	//powerSelector->SetPosition(powerSelectorShowPos);
	//if (bossHealthBar != NULL)
	//{
	//	bossHealthBar->SetTopLeft(bossHealthShowPos);
	//}

	
	//sprite.setPosition(288, 140);
	//momentumBar->SetTopLeft(momentumShowPos);
}

void AdventureHUD::Draw(RenderTarget *target)
{
	if (state != HIDDEN)
	{
		//Actor *p0 = owner->GetPlayer(0);
		kinMask->Draw(target);
		
		timer->Draw(target);

		modifierTimer->Draw(target);

		mini->Draw(target);
		//target->draw(owner->minimapSprite, &owner->minimapShader);

		//target->draw(owner->kinMinimapIcon);
		/*if (owner->powerRing != NULL)
		{
			owner->powerRing->Draw(target);
			owner->despOrb->Draw(target);
		}*/

		powerSelector->Draw(target);

		if (numActiveKeyMarkers == 0 && sess->currentZone != NULL )
		{
			target->draw(goSpr);
		}
		else
		{
			for (int i = 0; i < numActiveKeyMarkers; ++i)
			{
				keyMarkers[i]->Draw(target);
			}
		}
		

		if (bossHealthBar != NULL)
		{
			bossHealthBar->Draw(target);
		}

		//target->draw(flyCountText);
		
	}
}

KinMask::KinMask( Actor *a )
	:playerSkinShader("player")
{
	actor = a;
	sess = actor->sess;

	ts_face = sess->GetSizedTileset("HUD/kin_face_320x288.png");
	ts_portraitBG = sess->GetSizedTileset("HUD/kin_portrait_320x288.png");
	face.setTexture(*ts_face->texture);
	face.setTextureRect(ts_face->GetSubRect(0));
	playerSkinShader.SetSubRect( ts_face, ts_face->GetSubRect(0));

	faceBG.setTexture(*ts_portraitBG->texture);
	faceBG.setTextureRect(ts_portraitBG->GetSubRect(0));

	
	

	momentumBar = new MomentumBar(sess);

	SetTopLeft(Vector2f(0, 0));

	Reset();
}

KinMask::~KinMask()
{
	delete momentumBar;
}

void KinMask::Reset()
{
	playerSkinShader.SetSkin(actor->sess->GetPlayerNormalSkin(0));
	/*if (actor->owner != NULL && actor->owner->saveFile != NULL)
	{
		playerSkinShader.SetSkin(actor->owner->saveFile->defaultSkinIndex);
	}*/



	SetExpr(Expr_NEUTRAL);
	Update(0,false);
	frame = 0;

	faceBG.setTextureRect(ts_portraitBG->GetSubRect(0));
}

void KinMask::Draw(RenderTarget *target)
{
	if (actor->kinMode == Actor::K_DESPERATION )
	{
		target->draw(faceBG, &(actor->despFaceShader));
	}
	else
	{
		target->draw(faceBG);
	}

	int faceDeathAnimLength = 11;
	int an = 4;
	int f = frame / an;

	if (expr != Expr_DEATH || ( expr == Expr_DEATH && f < faceDeathAnimLength ) )
	{
		target->draw(face, &playerSkinShader.pShader);
	}

	momentumBar->SetMomentumInfo(actor->speedLevel, actor->GetSpeedBarPart());
	momentumBar->Draw(target);
}

void KinMask::SetExpr(KinMask::Expr ex)
{
	frame = 0;

	expr = ex;
}

void KinMask::Update( int speedLevel, bool desp )
{
	if (expr == Expr_DEATH )
	{
		int faceDeathAnimLength = 11;
		int an = 4;
		int f = frame / an;

		if (f < faceDeathAnimLength)
		{
			face.setTextureRect(ts_face->GetSubRect(5 + f));
			playerSkinShader.SetSubRect(ts_face, ts_face->GetSubRect(5+f));
		}
	}
	else if (expr == Expr_DEATHYELL)
	{
		faceBG.setTextureRect(ts_portraitBG->GetSubRect(5));
		face.setTextureRect(ts_face->GetSubRect(5));
		playerSkinShader.SetSubRect(ts_face, ts_face->GetSubRect(5));
	}
	else
	{ 
		if (expr == Expr_NEUTRAL || expr == Expr_SPEED1 || expr == Expr_SPEED2 )
		{
			switch (speedLevel)
			{
			case 0:
				expr = Expr_NEUTRAL;
				break;
			case 1:
				expr = Expr_SPEED1;
				break;
			case 2:
				expr = Expr_SPEED2;
				break;
			}

			/*if (desp)
			{
				expr = Expr_DESP;
			}

			if (actor->action == Actor::GROUNDHITSTUN || actor->action == Actor::AIRHITSTUN)
			{
				expr = Expr_HURT;
			}*/
		}

		face.setTextureRect(ts_face->GetSubRect(expr));
		playerSkinShader.SetSubRect(ts_face, ts_face->GetSubRect(expr));
		faceBG.setTextureRect(ts_portraitBG->GetSubRect(expr));
	}
	/*switch (expr)
	{
	case Expr_NEUTRAL:
		frame = 0;
		face.setTextureRect(ts_face->GetSubRect(expr + 6));
		break;
	Expr_HURT:
		frame = 0;
		break;
	Expr_SPEED1:
		frame = 0;
		break;
	Expr_SPEED2:
		frame = 0;
		break;
	Expr_DESP:
		frame = 0;
		break;
	Expr_DEATH:
		
		break;
	Expr_NONE:
		frame = 0;
		break;
	}*/

	++frame;
}

void KinMask::SetTopLeft(sf::Vector2f &pos)
{
	face.setPosition(pos);
	faceBG.setPosition(pos);
	momentumBar->SetTopLeft(pos + Vector2f(202, 117));
}

sf::Vector2f KinMask::GetTopLeft()
{
	return face.getPosition();
}