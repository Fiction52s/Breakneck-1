#include "BackpackCounter.h"
#include "Tileset.h"
#include "VectorMath.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"
#include "Session.h"
#include "RushManager.h"

using namespace std;
using namespace sf;

BackpackCounter::BackpackCounter(TilesetManager *tm)
{
	//backpack1_575x518_960, 491
	//backpackcircle_236x236_960, 524
	MainMenu *mm = MainMenu::GetInstance();

	levelText.setFont(mm->arial);
	levelText.setCharacterSize(20);
	levelText.setFillColor(Color::White);

	ts_ring = tm->GetSizedTileset("HUD/backpackcircle_245x245.png"); 
	ts_cage = tm->GetSizedTileset( "HUD/backpacklens_258x258.png" );
	ts_backpack = tm->GetSizedTileset( "HUD/backpack1_575x518.png");

	//scale = .33f;//1.f;

	scale = 1.f;

	if (!ringShader.loadFromFile("Resources/Shader/ring.frag", sf::Shader::Fragment))
	{
		assert(0);
	}

	ringShader.setUniform("u_texture", *ts_ring->texture);//sf::Shader::CurrentTexture);
	ringShader.setUniform("u_startAngle", 0.f);
	
	ringShader.setUniform("u_activeColor", sf::Glsl::Vec4(Color::Green));
	ringShader.setUniform("u_emptyColor", sf::Glsl::Vec4(Color::Red));

	SetRectSubRect(backpackQuad, ts_backpack->GetSubRect(0));
	SetRectSubRect(cageQuad, ts_cage->GetSubRect(0));
	SetRectSubRectGL(ringQuad, ts_ring->GetSubRect(0), Vector2f(ts_ring->texture->getSize()));

	numDivisions = 6;
	numParticlesPerDivision = 3;

	Reset();
}

void BackpackCounter::Reset()
{
	RushManager *rm = MainMenu::GetInstance()->rushManager;
	if (rm != NULL)
	{
		SetLevel(rm->storePoints);
		currParticlePoints = rm->currBackpackSectionsFilled * numParticlesPerDivision;
		double div = (PI * 2.0) / numDivisions;
		currFactor = div * rm->currBackpackSectionsFilled;
	}
	else
	{
		currFactor = 0;
		currParticlePoints = 0;
	}
}

void BackpackCounter::SetScale(float f)
{
	scale = f;
}

void BackpackCounter::SetSession(Session *p_sess)
{
	/*sess = p_sess;
	RushManager *rm = MainMenu::GetInstance()->rushManager;
	if ( rm != NULL)
	{
		SetLevel(rm->storePoints);
	}*/
}

void BackpackCounter::SetCenter(sf::Vector2f pos)
{
	//SetRectCenter(backpackQuad, ts_backpack->tileWidth * scale, ts_backpack->tileHeight * scale, pos);
	SetRectCenter(ringQuad, ts_ring->tileWidth * scale, ts_ring->tileHeight * scale, pos);
	SetRectCenter(cageQuad, ts_cage->tileWidth * scale, ts_cage->tileHeight * scale, pos);

	levelText.setPosition(pos);
	levelText.setOrigin(levelText.getLocalBounds().left + levelText.getLocalBounds().width / 2, levelText.getLocalBounds().top + levelText.getLocalBounds().height / 2);

}

int BackpackCounter::GetTotalPotentialPoints()
{
	return 0;//totalDivisions * pointsPerDivision;
}

void BackpackCounter::SetLevel(int lev)
{
	currLevel = lev;
	levelText.setString(to_string(currLevel));
	levelText.setOrigin(levelText.getLocalBounds().left + levelText.getLocalBounds().width / 2, levelText.getLocalBounds().top + levelText.getLocalBounds().height / 2);
	RushManager *rm = MainMenu::GetInstance()->rushManager;
	if (rm != NULL)
	{
		//SetLevel(rm->storePoints);
		rm->storePoints = currLevel;
	}
}

void BackpackCounter::AddParticle()
{
	double div = (PI * 2.0) / numDivisions;
	currFactor += div / numParticlesPerDivision;
	cout << "adding particle" << endl;
	currParticlePoints++;

	if (currParticlePoints % numParticlesPerDivision == 0)
	{
		RushManager *rm = MainMenu::GetInstance()->rushManager;
		if (rm != NULL)
		{
			//SetLevel(rm->storePoints);
			rm->currBackpackSectionsFilled = currParticlePoints / numParticlesPerDivision;
		}
	}

	if (currParticlePoints == numDivisions * numParticlesPerDivision)
	{
		SetLevel(currLevel + 1);
		currParticlePoints = 0;
		currFactor = 0;
	}
}

void BackpackCounter::Update()
{
	double div = (PI * 2.0) / numDivisions;
	//double f = div * currD
	if (currFactor < div * 4)
	{

	}

	ringShader.setUniform("u_filledRange", (float)currFactor);
}

void BackpackCounter::Draw(sf::RenderTarget *target)
{
	//target->draw(backpackQuad, 4, sf::Quads, ts_backpack->texture);
	target->draw(ringQuad, 4, sf::Quads, &ringShader);
	target->draw(cageQuad, 4, sf::Quads, ts_cage->texture);
	target->draw(levelText);
}