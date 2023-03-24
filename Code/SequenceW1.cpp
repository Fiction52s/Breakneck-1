#include "SequenceW1.h"
#include "Actor.h"
#include "Session.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "Enemy_CrawlerQueen.h"
#include "SaveFile.h"
#include "PauseMenu.h"
#include "GroundedWarper.h"
#include "Enemy_SequenceCrawler.h"
#include "AdventureManager.h"

using namespace std;
using namespace sf;

CrawlerAttackScene::CrawlerAttackScene()
	:BasicBossScene(BasicBossScene::RUN)
{
	GameSession *game = GameSession::GetSession();
	if (game != NULL)
	{
		assert(myBonus == NULL);
		myBonus = game->CreateBonus("FinishedScenes/W1/crawlerfight1");
	}
	else
	{
		myBonus = NULL;
	}
}

CrawlerAttackScene::~CrawlerAttackScene()
{
	if (myBonus != NULL)
		delete myBonus;
}

void CrawlerAttackScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[KINSTOP] = 30;
	stateLength[ROCKSFALL] = 30;
	stateLength[CRAWLERSWOOP] = 9 * 3;
	stateLength[DIGGINGAROUND] = 180;
}

void CrawlerAttackScene::AddShots()
{
	AddShot("scenecam");
	AddShot("crawlerdig1");
	AddShot("crawlerdig2");
	//AddShot("crawlerdig3");
	//AddShot("cavecam");
	//AddShot("fightcam");
}

void CrawlerAttackScene::AddPoints()
{
	AddStartAndStopPoints();
	
	//AddPoint("crawlersurface");
	//AddPoint("crawlerthrowkin");
}

void CrawlerAttackScene::AddGroups()
{
	//AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");
	//SetConvGroup("pre_crawler");
}

void CrawlerAttackScene::AddEnemies()
{
	PoiInfo *surface = points["crawlersurface"];

	//hard to spawn the queen here now...

	//BossCrawlerParams bcParams( owner->types["bosscrawler"], surface->edge)

	//queen = new CrawlerQueen(surface->edge, surface->edgeQuantity, false);
	//AddEnemy("queen", queen);
}

void CrawlerAttackScene::AddFlashes()
{
	AddFlashedImage("detailedgrab", sess->GetTileset("Story/Crawler_Dig_01_860x830.png", 860, 830),
		0, 30, 60, 30, Vector2f(1160, 540));

	AddFlashedImage("crawlerface", sess->GetTileset("Story/Crawler_Dig_02_828x875.png", 828, 875),
		0, 30, 60, 30, Vector2f(1350, 325));

	AddFlashedImage("kinface", sess->GetTileset("Story/Crawler_Dig_03_510x565.png", 510, 565),
		0, 30, 60, 30, Vector2f(625, 325));
}

void CrawlerAttackScene::SpecialInit()
{
	ts_queenGrab = sess->GetTileset("Bosses/Crawler/crawler_queen_grab_320x320.png", 320, 320);
	queenGrabSprite.setTexture(*ts_queenGrab->texture);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(0));
}

void CrawlerAttackScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	//BasicBossScene::ReturnToGame();

	
	

	//queen->StartFight();

	//EaseShot("fightcam", 60);
}

void CrawlerAttackScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (seqData.state)
	{
	case ENTRANCE:
		EntranceUpdate();
		break;
	case KINSTOP:
		if (seqData.frame == 0)
		{
			sess->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30);
			player->SetKinMode(Actor::K_NORMAL);
			player->SetAction(Actor::SEQ_LOOKUP);
		}
		break;
	case ROCKSFALL:
		RumbleDuringState(3, 3);
		break;
	case CRAWLERSWOOP:
	{
		if (seqData.frame == 15)
		{
			player->SetAction(Actor::SEQ_LOOKUPDISAPPEAR);
		}

		UpdateCrawlerSwoop();
		break;
	}
	case DIGGINGAROUND:
		RumbleDuringState(10, 10);
		if (seqData.frame == 0)
		{
			EaseShot("crawlerdig1", 90);
		}
		else if (seqData.frame == 90)
		{
			EaseShot("crawlerdig2", 90);
		}
		else if (seqData.frame == 150)
		{
			GameSession *game = GameSession::GetSession();

			if (game != NULL)
			{
				game->SetBonus(myBonus, V2d(0,0));
				game->ClearFade();
			}
			//sess->Fade(false, 30, Color::Black);
		}
		/*else if (frame == 120)
		{
			EasePoint("crawlerdig3", 1, 60);
		}*/

		if (seqData.frame == 20)
		{
			Flash("detailedgrab");
		}
		break;
	}
}

void CrawlerAttackScene::UpdateCrawlerSwoop()
{
	Actor *player = sess->GetPlayer(0);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(seqData.frame / 3));
	queenGrabSprite.setOrigin(queenGrabSprite.getLocalBounds().width / 2,
		queenGrabSprite.getLocalBounds().height);

	Edge *ground = player->ground;
	V2d gPoint = ground->GetPosition(player->edgeQuantity);
	V2d gNorm = ground->Normal();

	queenGrabSprite.setPosition(Vector2f(gPoint - gNorm * 30.0));
}

void CrawlerAttackScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (seqData.state == CRAWLERSWOOP)
	{
		target->draw(queenGrabSprite);
	}

	BasicBossScene::Draw(target, layer);
}

TextTestSeq::TextTestSeq()
{
	sess = Session::GetSession();

	stateLength[TALK] = 10000;
	stateLength[END] = 30;


	sceneLabel.setFont(sess->mainMenu->arial);
	sceneLabel.setFillColor(Color::White);
	sceneLabel.setCharacterSize(100);
	sceneLabel.setPosition(500, 500);

	AddGroup("W2/w2_bird_fight_pre");
	AddGroup("W2/w2_bird_fight_post");

	AddGroup("W3/w3_bird_crawler");
	AddGroup("W3/w3_coy_fight_pre");
	AddGroup("W3/w3_coy_fight_post");
	AddGroup("W3/w3_coy_skeleton");
	AddGroup("W3/w3_skeleton_kin");

	//AddGroup("W4/w4_bird_tiger");
	AddGroup("W4/w4_crawler_kin");
	AddGroup("W4/w4_tiger_fight_pre");
	AddGroup("W4/w4_tiger_fight_post");

	AddGroup("W5/w5_bird_tiger_past");
	AddGroup("W5/w5_bird_kin");
	AddGroup("W5/w5_gator_fight_pre");
	AddGroup("W5/w5_gator_fight_post");

	AddGroup("W6/w6_coy_kin_enter");
	AddGroup("W6/w6_bird_tiger_enter");
	AddGroup("W6/w6_coy_skele_fight");
	AddGroup("W6/w6_skeleton_film");
	AddGroup("W6/w6_coy_skele_kill");
	AddGroup("W6/w6_skele_nexus");
	AddGroup("W6/w6_tiger_fight_pre");
	AddGroup("W6/w6_tiger_kill");

	AddGroup("W7/w7_bird_fight_chase");
	AddGroup("W7/w7_bird_kill");
	AddGroup("W7/w7_core_fight");
	AddGroup("W7/w7_core_skele_kill");
	AddGroup("W7/w7_game_credits");

	AddGroup("W8/w8_kin_bear_fight");
	AddGroup("W8/w8_kin_bear_kill");
	AddGroup("W8/w8_true_credits");


	//conv = new Conversation(owner);
	//conv->Load("conv");

	Reset();
}

void TextTestSeq::AddGroup(const std::string &name)
{
	ConversationGroup *cg = new ConversationGroup;
	cg->Load(name);
	groups.push_back(cg);
}

TextTestSeq::~TextTestSeq()
{
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete (*it);
	}
	//delete conv;
	//delete textDisp;
}

void TextTestSeq::Init()
{

}

bool TextTestSeq::Update()
{
	Actor *player = sess->GetPlayer(0);

	if (seqData.frame == stateLength[seqData.state] && seqData.state != END)
	{
		int s = seqData.state;
		s++;
		seqData.state = (State)s;
		seqData.frame = 0;

		if (state == END)
		{
		}
	}

	if (seqData.state == END)
	{
		player->SetAction(Actor::STAND);
		player->frame = 0;
		sess->hud->Show(60);
		sess->cam.EaseOutOfManual(60);
		//player->set
		return false;
	}

	ConversationGroup *cg = groups[gIndex];
	Conversation *conv = cg->GetConv(cIndex);
	switch (state)
	{
	case TALK:
		if (seqData.frame == 0)
		{
			sess->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30, CubicBezier());
			conv->Show();
			sess->hud->Hide(60);
		}

		if (sess->GetCurrInput(0).A && !sess->GetPrevInput(0).A)
		{
			conv->NextSection();
		}
		if (sess->GetCurrInput(0).B)
		{
			conv->SetRate(1, 5);
		}
		else
		{
			conv->SetRate(1, 1);
		}


		if (!conv->Update())
		{
			if (cIndex < cg->numConvs - 1)
			{
				++cIndex;

				Conversation *newconv = cg->GetConv(cIndex);

				newconv->Show();
			}
			else
			{
				if (gIndex < groups.size() - 1)
				{
					++gIndex;
					UpdateSceneLabel();
					cIndex = 0;
					sess->CrossFade(10, 0, 10, Color::Black);

					ConversationGroup *newcg = groups[gIndex];
					Conversation *newconv = newcg->GetConv(cIndex);

					newconv->Show();

				}
				else
				{
					seqData.frame = stateLength[TALK] - 1;
				}
			}
		}
		break;
	}

	++seqData.frame;

	return true;
}
void TextTestSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	View v = target->getView();
	target->setView(sess->uiView);

	ConversationGroup *cg = groups[gIndex];
	Conversation *conv = cg->GetConv(cIndex);

	target->draw(sceneLabel);

	conv->Draw(target);
	target->setView(v);
}
void TextTestSeq::Reset()
{
	seqData.state = TALK;
	seqData.frame = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		(*it)->Reset();
	}
	gIndex = 0;
	cIndex = 0;

	UpdateSceneLabel();
	//sceneLabel.setString( )
}

void TextTestSeq::UpdateSceneLabel()
{
	ConversationGroup *cg = groups[gIndex];
	sceneLabel.setString(cg->sceneName);
}

CrawlerPreFightScene::CrawlerPreFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void CrawlerPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[DIGGINGAROUND] = 150;
	stateLength[THROWOUT] = 60;
	stateLength[CRAWLERFACE] = 10000;
	stateLength[CRAWLERTALK] = 10000;
	stateLength[KINTALK] = 10000;

	queen = (CrawlerQueen*)sess->GetEnemy(EnemyType::EN_CRAWLERQUEEN);
}

void CrawlerPreFightScene::AddShots()
{
	AddShot("cavecam");
	AddShot("fightcam");
	AddShot("crawlerdig1");
	AddShot("crawlerdig2");
}

void CrawlerPreFightScene::AddPoints()
{
	AddPoint("crawlersurface");
	AddPoint("crawlerthrowkin");
}

void CrawlerPreFightScene::AddGroups()
{
	AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");
	SetConvGroup("pre_crawler");
}

void CrawlerPreFightScene::AddEnemies()
{
	PoiInfo *surface = points["crawlersurface"];

	//hard to spawn the queen here now...

	//BossCrawlerParams bcParams( owner->types["bosscrawler"], surface->edge)

	//queen = new CrawlerQueen(surface->edge, surface->edgeQuantity, false);
	//AddEnemy("queen", queen);
}

void CrawlerPreFightScene::AddFlashes()
{
	AddFlashedImage("detailedgrab", sess->GetTileset("Story/Crawler_Dig_01_860x830.png", 860, 830),
		0, 30, 60, 30, Vector2f(1160, 540));

	AddFlashedImage("crawlerface", sess->GetTileset("Story/Crawler_Dig_02_828x875.png", 828, 875),
		0, 30, 60, 30, Vector2f(1350, 325));

	AddFlashedImage("kinface", sess->GetTileset("Story/Crawler_Dig_03_510x565.png", 510, 565),
		0, 30, 60, 30, Vector2f(625, 325));
}

void CrawlerPreFightScene::SpecialInit()
{
}

void CrawlerPreFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();
	queen->StartFight();

	EaseShot("fightcam", 60);
	sess->SetPlayerInputOn(true);
}

void CrawlerPreFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (seqData.state)
	{
	case DIGGINGAROUND:
	{
		RumbleDuringState(10, 10);
		if (seqData.frame == 0)
		{
			//sess->Fade(true, 60, Color::Black;
			sess->FreezePlayer(true);
			sess->AddEnemy(queen);
			queen->SeqWait();
			sess->hud->Hide();
			
			SetCameraShot("crawlerdig1");
			EaseShot("crawlerdig2", 90);
			//EasePoint("crawlerdig1", 1, 60);
			
		}
		else if (seqData.frame == 90)
		{
			EaseShot("cavecam", 60);
		}

		if (seqData.frame == 20)
		{
			//Flash("detailedgrab");
		}
		break;
	}
		
	case THROWOUT:
		if (seqData.frame == 0)
		{
			//sess->currentZone->ReformAllGates();
			sess->FreezePlayer(false);
			player->StartSeqKinThrown(GetPointPos("crawlersurface"), V2d(-10, -10));
		}
		else if (seqData.frame == 30)
		{
			queen->StartInitialUnburrow();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	case CRAWLERFACE:
	{
		if (seqData.frame == 0)
		{
			queen->StartAngryYelling();
		}

		BasicFlashUpdateState("crawlerface");
		break;
	}
	case CRAWLERTALK:
	{
		ConvUpdate();
		break;
	}
	case KINTALK:
	{
		BasicFlashUpdateState("kinface");
		break;
	}
	}
}

void CrawlerPreFightScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	BasicBossScene::Draw(target, layer);
}

CrawlerPostFightScene::CrawlerPostFightScene()
	:BasicBossScene( BasicBossScene::APPEAR )
{
	//queen = NULL;
	seqCrawler = NULL;
	//warper = sess->GetWarper("FinishedScenes/W1/nexus1");
}

void CrawlerPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = explosionFadeFrames;//fadeFrames;// +explosionFadeFrames;
	stateLength[FADE_IN] = fadeFrames;
	stateLength[CRAWLER_RETREAT] = 80;
	stateLength[WAIT] = 60;
	//stateLength[PLAYMOVIE] = 1000000;

	seqCrawler = (SequenceCrawler*)sess->GetEnemy(EnemyType::EN_SEQUENCECRAWLER);
}

void CrawlerPostFightScene::ReturnToGame()
{
	/*if (!warper->spawned)
	{
		sess->AddEnemy(warper);
	}
	warper->Activate();*/
	SetPlayerStandDefaultPoint(true);
	sess->cam.EaseOutOfManual(60);
	sess->RemoveEnemy(seqCrawler);
	//sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
	//queen->SeqWait();
}

void CrawlerPostFightScene::AddPoints()
{
	AddStandPoint();
}

void CrawlerPostFightScene::AddGroups()
{
	AddGroup("post_crawler", "W1/w1_crawler_post_fight");
}

void CrawlerPostFightScene::AddFlashes()
{
	AddFlashedImage("crawlercut", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_01b.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("k1", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_02b2.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("c1", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_02c.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("k2", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_03a.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("c2", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_03b.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("k3", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_02d.png"),
		0, 30, 60, 30, Vector2f(960, 540));
}

void CrawlerPostFightScene::AddShots()
{
	AddShot("scenecam");
}

//void CrawlerPostFightScene::StartRunning()
//{
//	//right now only works in gamesession
//	//sess->SetGameSessionState(GameSession::SEQUENCE);
//}

void CrawlerPostFightScene::UpdateState()
{

	switch (seqData.state)
	{
	case FADE:
	{
		if (seqData.frame == 0)
		{
			StartBasicNewMapKillFade();
		}
		break;
	}
	case CRAWLER_SLASHED:
	{
		if (seqData.frame == 0)
		{
			Flash("crawlercut");
		}

		if (IsFlashDone("crawlercut"))
		{
			EndCurrState();
		}
		break;
	}
	/*case KIN_1:
	{
		if (frame == 0)
		{
			Flash("k1");
		}

		if (IsFlashDone("k1"))
		{
			EndCurrState();
		}
		break;
	}
	case CRAWLER_ANGRY:
	{
		if (frame == 0)
		{
			Flash("c1");
		}

		if (IsFlashDone("c1"))
		{
			EndCurrState();
		}
		break;
	}
	case KIN_2:
	{
		if (frame == 0)
		{
			Flash("k2");
		}

		if (IsFlashDone("k2"))
		{
			EndCurrState();
		}
		break;
	}
	case CRAWLER_SCARED:
	{
		if (frame == 0)
		{
			Flash("c2");
		}

		if (IsFlashDone("c2"))
		{
			EndCurrState();
		}
		break;
	}*/
	case FADE_IN:
	{
		if (seqData.frame == 0)
		{
			EndBasicNewMapKillFade();
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("scenecam");

			seqCrawler->Reset();
			sess->AddEnemy(seqCrawler);
			seqCrawler->facingRight = false;
		}
		break;
	}
	case CRAWLER_GIBBERISH:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("post_crawler");
		}
		ConvUpdate();
		break;
	}
	case CRAWLER_RETREAT:
	{
		if (seqData.frame == 0)
		{
			seqCrawler->DigIn();
		}
		
		break;
	}
	case KIN_FINAL_FACE:
	{
		if (seqData.frame == 0)
		{
			Flash("k3");
		}

		if (IsFlashDone("k3"))
		{
			EndCurrState();
		}
		break;
	}
	}
	//if (state == PLAYMOVIE)
	//{
	//	EndCurrState();
	//	/*if (frame == 0)
	//	{
	//		SetCurrMovie("crawler_slash", 60);
	//	}
	//	
	//	UpdateMovie();*/
	//}
}



GetAirdashPowerScene::GetAirdashPowerScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	darkRect.setFillColor(Color::Black);
	darkRect.setSize(Vector2f(1920, 1080));
	darkRect.setPosition(0, 0);
}

void GetAirdashPowerScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[KIN_KNEELING] = 60;
	stateLength[START_MEDITATE] = 60;
	stateLength[FADE_BACKGROUND] = 60;
	stateLength[EXPEL_ENERGY] = 60;
	stateLength[WAITAFTEREXPEL] = 60;
	stateLength[MASKOFF] = 120;
	stateLength[PLAYMOVIE] = -1;
	stateLength[FADE_BACK] = 90;

	//sceneMusic = owner->mainMenu->musicManager->songMap["w1_26_Edge"];
	//sceneMusic->Load();
}

void GetAirdashPowerScene::AddMovies()
{
	AddMovie("kin_meditate_01");
}

void GetAirdashPowerScene::ReturnToGame()
{
	SetPlayerStandDefaultPoint(true);
	sess->Fade(true, 60, Color::Black);
	sess->cam.EaseOutOfManual(60);
	sess->TotalDissolveGates(Gate::BOSS);

	

	sess->mainMenu->musicPlayer->TransitionMusic(sess->originalMusic, 60);
}

void GetAirdashPowerScene::AddPoints()
{
	//AddStandPoint();
}

void GetAirdashPowerScene::StartRunning()
{
	//owner->state = GameSession::SEQUENCE;
}

void GetAirdashPowerScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case KIN_KNEELING:
		if (seqData.frame == 0)
		{
			//owner->mainMenu->musicPlayer->TransitionMusic(sceneMusic, 60);
			sess->cam.Ease(Vector2f(player->position.x, player->position.y - 68), .75, 60, CubicBezier());

			//player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect(0));
			//player->dirtyAuraSprite.setOrigin(player->dirtyAuraSprite.getLocalBounds().width / 2,
			//	player->dirtyAuraSprite.getLocalBounds().height / 2);

			//player->SetDirtyAura(true);

			player->SeqKneel();
		}

		break;
	case START_MEDITATE:
		if (seqData.frame == 0)
		{
			player->SeqMeditateMaskOn();
		}
		break;
	case FADE_BACKGROUND:
		if (seqData.frame == 0)
		{
			sess->Fade(false, 60, Color::Black, true);
		}
		else if (seqData.frame == stateLength[FADE_BACKGROUND] - 1)
		{
			sess->SetGameSessionState(GameSession::SEQUENCE);
		}
		break;
	case EXPEL_ENERGY:
	{
		if (seqData.frame == 0)
		{
			sess->ClearFade();
		}

		int f = 60 - 3 * 10;
		int ff = seqData.frame - f;
		if (ff >= 0 && ff < 10 * 3)
			//if( frame < 10 * 3 )
		{
			//player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect(ff / 3 + 15));
		}
		else
		{
			//player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect((frame % (15 * 3) / 3)));
		}


		break;
	}
	case WAITAFTEREXPEL:
	{
		if (seqData.frame == 0)
		{
			//player->SetDirtyAura(false);
		}
		break;
	}
	case MASKOFF:
		if (seqData.frame == 0)
		{
			player->SeqMaskOffMeditate();
		}
		break;

	case PLAYMOVIE:
	{
		if (seqData.frame == 0)
		{
			SetCurrMovie("kin_meditate_01", 60);
		}

		UpdateMovie();

		if (IsLastFrame())
		{
			sess->SetGameSessionState(GameSession::RUN);
			sess->Fade(true, 60, Color::Black, true);
			sess->hud->Show(60);
			SaveFile *sf = sess->mainMenu->adventureManager->currSaveFile;
			if (sf != NULL)
			{
				sf->UnlockUpgrade(Actor::UpgradeType::UPGRADE_POWER_AIRDASH);
			}
			sess->GetPlayer(0)->SetUpgrade(Actor::UpgradeType::UPGRADE_POWER_AIRDASH, true );
		}
		break;
	}
	case FADE_BACK:
		if (seqData.frame == 0)
		{

		}
		else if (seqData.frame == 60)
		{
			player->SeqGetAirdash();
		}
		else if (seqData.frame == stateLength[FADE_BACK] - 1)
		{
			//cout << "set easting out of manual" << endl;
			sess->cam.EaseOutOfManual(120);
		}
		break;
	}

	if (state != EXPEL_ENERGY)
	{
		//player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect((frame % (15 * 3) / 3)));
	}

	//player->dirtyAuraSprite.setPosition(Vector2f(player->sprite->getPosition().x,
	//	player->sprite->getPosition().y - 32));

	if (sess->GetGameSessionState() == GameSession::SEQUENCE)
	{
		sess->totalGameFrames++;
		player->UpdatePrePhysics();
		player->UpdatePostPhysics();
	}
}

void GetAirdashPowerScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (sess->GetGameSessionState() == GameSession::SEQUENCE)
	{
		target->setView(sess->uiView);

		if (state >= EXPEL_ENERGY)
		{
			//darkRect.setPosition(owner->cam.pos);
			target->draw(darkRect);
		}

		BasicBossScene::Draw(target, layer);


		target->setView(sess->view);


		sess->GetPlayer(0)->Draw(target);
	}
}