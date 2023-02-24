#include "TutorialSequence.h"
#include "Session.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "TutorialBox.h"

using namespace sf;
using namespace std;

TutorialSequence::TutorialSequence()
{
	sess = Session::GetSession();

	SetRectColor(overlayRect, Color(100, 100, 100, 100));
	SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));

	tutBox = new TutorialBox;
	tutBox->SetCenterPos(Vector2f(960, 200));
}

TutorialSequence::~TutorialSequence()
{
	delete tutBox;
}


void TutorialSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[TELL] = 1000000;
}

void TutorialSequence::AddGroups()
{
//	AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");
	
}

void TutorialSequence::SetText(const std::string &str)
{
	tutBox->SetText(str);
}

void TutorialSequence::SetBoxPos(sf::Vector2f &pos)
{
	tutBox->SetCenterPos(pos);
}

void TutorialSequence::ReturnToGame()
{
	//sess->cam.EaseOutOfManual(60);
	//sess->cam.StopRumble();
}

void TutorialSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (state)
	{
	case TELL:
	{
		if (frame == 0)
		{
		//	SetConvGroup("pre_crawler");
		}
		break;
	}
	}

	tutBox->UpdateButtonIconsWhenControllerIsChanged();
	//++frame;

	//emitter->Update();
}

void TutorialSequence::Draw(RenderTarget *target, EffectLayer layer)
{
	if (layer == UI_FRONT)
	{
		tutBox->Draw(target);
	}
	//Sequence::Draw(target, layer);
}

void TutorialSequence::Reset()
{
	Sequence::Reset();
	state = TELL;
	frame = 0;
}