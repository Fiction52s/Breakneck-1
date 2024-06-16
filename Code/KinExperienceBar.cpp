#include "KinExperienceBar.h"
#include "VectorMath.h"
#include "MainMenu.h"
#include <string>

using namespace sf;
using namespace std;

ExperienceAdder::ExperienceAdder()
{

}

void ExperienceAdder::SetTopLeft(sf::Vector2f pos)
{

}

void ExperienceAdder::Draw(sf::RenderTarget *target)
{
	target->draw(fullText);
}


KinExperienceBar::KinExperienceBar(TilesetManager *tm)
{
	ts_bar = tm->GetSizedTileset("Menu/AdventureScoreDisplay/expbartest_729x173.png");
	ts_bar->SetQuadSubRect(barQuad, 0);

	Vector2f center(960, 540);

	SetRectCenter(barQuad, 729, 173, center);

	MainMenu *mm = MainMenu::GetInstance();
	expText.setFont(mm->arial);
	expText.setCharacterSize(30);
	expText.setPosition(center + Vector2f( 0, - 200 ));
	
	Reset();
}

void KinExperienceBar::Reset()
{
	currLevel = -1;
	expToLevelUp = -1;
	currExp = -1;
	expToAdd = 0;

	action = A_IDLE;
	frame = 0;
}

void KinExperienceBar::Setup(int p_currLevel, int p_currExp)
{
	Reset();

	currLevel = p_currLevel;
	currExp = p_currExp;

	//needs parameters to set current level etc
}


int KinExperienceBar::GetExpToLevelUp()
{
	return 50;
}

bool KinExperienceBar::IsLeveledUp()
{
	return action == A_LEVEL_UP;
}

void KinExperienceBar::Update()
{
	switch (action)
	{
	case A_ADDING:
	{
		if (currExp >= GetExpToLevelUp())
		{
			currExp -= GetExpToLevelUp();
			currLevel += 1;

			action = A_LEVEL_UP;
			frame = 0;
		}
		else if (expToAdd == 0)
		{
			action = A_IDLE;
			frame = 0;
		}
		break;
	}
	}

	switch (action)
	{
	case A_ADDING:
	{
		if (frame % 3 == 0)
		{
			assert(expToAdd > 0);
			expToAdd -= 1;
			currExp += 1;

			expText.setString(to_string(currExp) + " / " + to_string(GetExpToLevelUp()));
		}
		break;
	}
		
	}


	++frame;
}

void KinExperienceBar::AddExp(int exp)
{
	action = A_ADDING;
	frame = 0;

	expToAdd = exp;
}

void KinExperienceBar::Draw( sf::RenderTarget *target )
{
	target->draw(barQuad, 4, sf::Quads, ts_bar->texture);
	target->draw(expText);
}