#include "SequenceW1.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "Enemy_CrawlerQueen.h"

using namespace std;
using namespace sf;

TextTestSeq::TextTestSeq(GameSession *p_owner)
	:owner(p_owner)
{
	stateLength[TALK] = 10000;
	stateLength[END] = 30;


	sceneLabel.setFont(owner->font);
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

	AddGroup("W4/w4_bird_tiger");
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
	ConversationGroup *cg = new ConversationGroup(owner);
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
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != END)
	{
		int s = state;
		s++;
		state = (State)s;
		frame = 0;

		if (state == END)
		{
		}
	}

	if (state == END)
	{
		player->SetAction(Actor::STAND);
		player->frame = 0;
		owner->adventureHUD->Show(60);
		owner->cam.EaseOutOfManual(60);
		//player->set
		return false;
	}

	ConversationGroup *cg = groups[gIndex];
	Conversation *conv = cg->GetConv(cIndex);
	switch (state)
	{
	case TALK:
		if (frame == 0)
		{
			owner->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30, CubicBezier());
			conv->Show();
			owner->adventureHUD->Hide(60);
		}

		if (owner->GetCurrInput(0).A && !owner->GetPrevInput(0).A)
		{
			conv->NextSection();
		}
		if (owner->GetCurrInput(0).B)
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
					owner->CrossFade(10, 0, 10, Color::Black);

					ConversationGroup *newcg = groups[gIndex];
					Conversation *newconv = newcg->GetConv(cIndex);

					newconv->Show();

				}
				else
				{
					frame = stateLength[TALK] - 1;
				}
			}
		}
		break;
	}

	++frame;

	return true;
}
void TextTestSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	View v = target->getView();
	target->setView(owner->uiView);

	ConversationGroup *cg = groups[gIndex];
	Conversation *conv = cg->GetConv(cIndex);

	target->draw(sceneLabel);

	conv->Draw(target);
	target->setView(v);
}
void TextTestSeq::Reset()
{
	state = TALK;
	frame = 0;
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

CrawlerAttackSeq::CrawlerAttackSeq(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
{
}

void CrawlerAttackSeq::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[KINSTOP] = 30;
	stateLength[ROCKSFALL] = 30;
	stateLength[CRAWLERSWOOP] = 9 * 3;
	stateLength[DIGGINGAROUND] = 180;
	stateLength[THROWOUT] = 60;
	stateLength[CRAWLERFACE] = 10000;
	stateLength[CRAWLERTALK] = 10000;
	stateLength[KINTALK] = 10000;
}

void CrawlerAttackSeq::AddShots()
{
	AddShot("scenecam");
	AddShot("cavecam");
	AddShot("fightcam");
}

void CrawlerAttackSeq::AddPoints()
{
	AddStartAndStopPoints();
	AddPoint("crawlerdig1");
	AddPoint("crawlerdig2");
	AddPoint("crawlersurface");
	AddPoint("crawlerthrowkin");
}

void CrawlerAttackSeq::AddGroups()
{
	AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");
	SetConvGroup("pre_crawler");
}

void CrawlerAttackSeq::AddEnemies()
{
	PoiInfo *surface = points["crawlersurface"];
	queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	AddEnemy("queen", queen);
}

void CrawlerAttackSeq::AddFlashes()
{
	AddFlashedImage("detailedgrab", owner->GetTileset("Story/Crawler_Dig_01_860x830.png", 860, 830),
		0, 30, 60, 30, Vector2f(1160, 540));

	AddFlashedImage("crawlerface", owner->GetTileset("Story/Crawler_Dig_02_828x875.png", 828, 875),
		0, 30, 60, 30, Vector2f(1350, 325));

	AddFlashedImage("kinface", owner->GetTileset("Story/Crawler_Dig_03_510x565.png", 510, 565),
		0, 30, 60, 30, Vector2f(625, 325));
}

void CrawlerAttackSeq::SpecialInit()
{
	ts_queenGrab = owner->GetTileset("Bosses/Crawler/crawler_queen_grab_320x320.png", 320, 320);
	queenGrabSprite.setTexture(*ts_queenGrab->texture);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(0));
}

void CrawlerAttackSeq::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();
	queen->StartFight();

	EaseShot("fightcam", 60);
}

void CrawlerAttackSeq::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		EntranceUpdate();
		break;
	case KINSTOP:
		if (frame == 0)
		{
			owner->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30);
			player->desperationMode = false;
			player->StartAction(Actor::SEQ_LOOKUP);
		}
		break;
	case ROCKSFALL:
		RumbleDuringState(3, 3);
		break;
	case CRAWLERSWOOP:
	{
		if (frame == 15)
		{
			player->StartAction(Actor::SEQ_LOOKUPDISAPPEAR);
		}

		UpdateCrawlerSwoop();
		break;
	}
	case DIGGINGAROUND:
		RumbleDuringState(10, 10);
		if (frame == 0)
		{
			EasePoint("crawlerdig1", 1, 60);
		}
		else if (frame == 60)
		{
			EasePoint("crawlerdig2", 1, 60);
		}
		else if (frame == 120)
		{
			EaseShot("cavecam", 60);
		}

		if (frame == 20)
		{
			Flash("detailedgrab");
		}
		break;
	case THROWOUT:
		if (frame == 0)
		{
			//owner->currentZone->ReformAllGates();

			player->StartSeqKinThrown(points["crawlersurface"]->pos, V2d(-10, -10));
		}
		else if (frame == 30)
		{
			owner->AddEnemy(queen);
			queen->StartInitialUnburrow();
			owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		break;
	case CRAWLERFACE:
	{
		if (frame == 0)
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

void CrawlerAttackSeq::UpdateCrawlerSwoop()
{
	Actor *player = owner->GetPlayer(0);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(frame / 3));
	queenGrabSprite.setOrigin(queenGrabSprite.getLocalBounds().width / 2,
		queenGrabSprite.getLocalBounds().height);

	Edge *ground = player->ground;
	V2d gPoint = ground->GetPoint(player->edgeQuantity);
	V2d gNorm = ground->Normal();

	queenGrabSprite.setPosition(Vector2f(gPoint - gNorm * 30.0));
}

void CrawlerAttackSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state == CRAWLERSWOOP)
	{
		target->draw(queenGrabSprite);
	}

	BasicBossScene::Draw(target, layer);
}

AfterCrawlerFightSeq::AfterCrawlerFightSeq(GameSession *p_owner)
	:BasicBossScene( p_owner, BasicBossScene::APPEAR )
{

}

void AfterCrawlerFightSeq::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 10;
	stateLength[PLAYMOVIE] = 1000000;
}

void AfterCrawlerFightSeq::ReturnToGame()
{
	SetPlayerStandDefaultPoint(true);
	owner->Fade(true, 60, Color::Black);
	owner->cam.EaseOutOfManual(60);
	owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
}

void AfterCrawlerFightSeq::AddPoints()
{
	AddStandPoint();
}

void AfterCrawlerFightSeq::StartRunning()
{
	owner->state = GameSession::SEQUENCE;
}

void AfterCrawlerFightSeq::UpdateState()
{
	if (state == FADE)
	{
		if (frame == 0)
		{
			MainMenu *mm = owner->mainMenu;

			owner->CrossFade(10, 0, 60, Color::White);
			mm->musicPlayer->FadeOutCurrentMusic(60);
		}
	}
	if (state == PLAYMOVIE)
	{
		if (frame == 0)
		{
			SetCurrMovie("crawler_slash", 60);
		}
		
		UpdateMovie();
	}
}

void AfterCrawlerFightSeq::AddMovies()
{
	AddMovie("crawler_slash");
}