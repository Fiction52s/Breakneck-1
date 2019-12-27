#include "SequenceW1.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"

using namespace std;
using namespace sf;

CrawlerAfterFightSeq::CrawlerAfterFightSeq(GameSession *p_owner)
	:owner(p_owner)
{
	frame = 0;
	frameCount = 6000;

	cfightCenter = owner->poiMap["cfightcenter"]->pos;
}

void CrawlerAfterFightSeq::Reset()
{
	frame = 0;
}

bool CrawlerAfterFightSeq::Update()
{
	Actor *player = owner->GetPlayer(0);
	switch (frame)
	{
	case 0:
	{
		owner->Fade(false, 60, Color::White);
		owner->Pause(60);
		owner->cam.SetManual(true);
	}
	break;
	case 1:
	{
		owner->ClearFX();
		owner->cam.Set(Vector2f(cfightCenter.x, cfightCenter.y), 1, 0);
		owner->Fade(true, 60, Color::White);
		owner->Pause(60);
		player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
		player->frame = 0;
		player->groundSpeed = 0;
		PoiInfo *pi = owner->poiMap["cfightjumpback"];
		player->ground = pi->edge;
		player->edgeQuantity = pi->edgeQuantity;
		player->offsetX = player->b.rw;
		//player->offsetX = 0;
		player->facingRight = true;
	}
	break;
	case 60:
	{
	}
	break;
	}

	if (frame == 80)
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->frame = 0;
	}
	else if (frame == 120)
	{
		player->action = Actor::GETPOWER_AIRDASH_MEDITATE;
		player->frame = 0;
		player->groundSpeed = 0;
	}


	//int blah = 30 + 60 + 120 + 60 + 60;
	//if( frame == blah )//if( owner->cam.currMove == NULL )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
	//}
	//else if( frame == blah + 30 )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED;
	//	player->groundSpeed = 0;
	//	owner->cam.SetRumble( 3, 3, 40 );
	//	owner->b_crawler->action = Boss_Crawler::EMERGE;
	//	owner->b_crawler->frame = 0;
	//}
	//else if( frame == blah + 50 )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_DODGEBACK;
	//	player->frame = 0;

	//}
	//else if( frame == blah + 80 )
	//{
	//	//in reality this would be calling a function within the crawler
	//	owner->b_crawler->StartMeetPlayerSeq();

	//	//owner->b_crawler->portrait.SetSprite( ts_birdFace, 1 );
	//}

	//if( startFightMsgFrame >= 0 && frame == startFightMsgFrame + 30 )
	//{
	//	player->action = Actor::STAND;
	//	player->frame = 0;
	//}

	++frame;

	if (frame == frameCount)
		return false;
	else
		return true;
}

void CrawlerAfterFightSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{

}

CrawlerDefeatedSeq::CrawlerDefeatedSeq(GameSession *p_owner)
	:owner(p_owner)
{
	Reset();

	stateLength[PLAYMOVIE] = 1000000;

	assert(mov.openFromFile("Resources/Movie/crawler_slash.ogv"));
	mov.fit(sf::FloatRect(0, 0, 1920, 1080));
}

void CrawlerDefeatedSeq::Reset()
{
	state = PLAYMOVIE;
	frame = 0;
}

bool CrawlerDefeatedSeq::Update()
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
		//owner->Fade(true, 60, sf::Color::White);
		owner->state = GameSession::RUN;
		owner->Fade(true, 60, Color::Black);

		Actor *player = owner->GetPlayer(0);
		player->SeqAfterCrawlerFight();
		//player->SetAction(Actor::GOALKILLWAIT);
		//player->frame = 0;
		//owner->scoreDisplay->Activate();
		return false;
	}

	if (state == PLAYMOVIE)
	{
		sfe::Status movStatus = mov.getStatus();
		if (frame == 0)
		{
			mov.setVolume(owner->mainMenu->config->GetData().musicVolume);
			mov.setPlayingOffset(sf::Time::Zero);
			mov.play();
		}
		else
		{
			mov.update();

			//cout << "mov: " << mov.getPlayingOffset().asSeconds() << endl;
			if (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped)
			{
				frame = stateLength[PLAYMOVIE] - 1;
				if (owner->originalMusic != NULL)
				{
					MainMenu *mm = owner->mainMenu;
					mm->musicPlayer->TransitionMusic(owner->originalMusic, 60);
				}

				//owner->state = GameSession::RUN;
				//owner->Fade(true, 60, Color::Black, true);
				/*if (frame == stateLength[MASKOFF] - 1)
				{

				}*/
			}
		}
	}
	++frame;

	return true;
}


void CrawlerDefeatedSeq::Draw(sf::RenderTarget *target,
	EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	target->draw(mov);
}

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