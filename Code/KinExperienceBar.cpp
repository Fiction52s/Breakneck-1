#include "KinExperienceBar.h"
#include "VectorMath.h"

using namespace sf;

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

	SetRectCenter(barQuad, 729, 173, Vector2f(960, 540));
}

void KinExperienceBar::Draw( sf::RenderTarget *target )
{
	target->draw(barQuad, 4, sf::Quads, ts_bar->texture);
}