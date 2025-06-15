#include "KinExperienceBar.h"
#include "VectorMath.h"
#include "MainMenu.h"
#include <string>
#include "nlohmann\json.hpp"
#include <fstream>

using namespace sf;
using namespace std;

using json = nlohmann::json;

ExpBarParams::ExpBarParams()
{
	maxLevel = -1;
	levelUpAmtBase = -1;
	maxLevelUpIncreaseLevel = -1;
	levelUpIncrease = -1;
	gold = -1;
	silver = -1;
	bronze = -1;
}

void ExpBarParams::Load()
{
	string path = "Resources/Rush/kinexp.json";

	ifstream is;
	is.open(path);

	json j;
	is >> j;

	auto &info = j["Info"];

	maxLevel = info["Max Level"];
	levelUpAmtBase = info["Level-up Amount Base"];
	maxLevelUpIncreaseLevel = info["Max Level-up Increase Level"];
	levelUpIncrease = info["Level-up Increase"];
	gold = info["Gold"];
	silver = info["Silver"];
	bronze = info["Bronze"];
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

	params.Load();
	
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

void KinExperienceBar::AddMedal(int medal)
{
	gainedLevels = 0;

	int toAdd = 0;
	if (medal == 0)
	{
		toAdd = params.bronze;
	}
	else if (medal == 1)
	{
		toAdd = params.silver;
	}
	else if (medal == 2)
	{
		toAdd = params.gold;
	}
	AddExp(toAdd);
}

int KinExperienceBar::GetExpToLevelUp()
{
	return params.levelUpAmtBase + min( currLevel, params.maxLevelUpIncreaseLevel ) * params.levelUpIncrease;
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
		int expToNextLevel = GetExpToLevelUp();

		assert(currExp <= expToNextLevel);
		if (currExp == expToNextLevel)
		{
			cout << "leveled up" << "\n";

			currLevel += 1;
			currExp = 0;
			gainedLevels += 1;

			//action = A_LEVEL_UP;
			//frame = 0;
		}
		else if (expToAdd == 0)
		{
			cout << "finished adding exp" << "\n";
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
		if (frame % 30 == 0)
		{
			cout << "added 1 exp current to add is: " << expToAdd - 1 << "\n";
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
	cout << "About to add " << exp << " exp\n";

	assert(exp > 0);
}

void KinExperienceBar::Draw( sf::RenderTarget *target )
{
	target->draw(barQuad, 4, sf::Quads, ts_bar->texture);
	target->draw(expText);
}

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