#include "KinBoostScreen.h"

using namespace std;
using namespace sf;

KinBoostScreen::KinBoostScreen()
{
	rect.setFillColor(Color::Red);
	rect.setPosition(200, 200);
	rect.setSize(Vector2f(2000, 2000));
}

void KinBoostScreen::Draw(RenderTarget *target)
{
	target->draw(rect);
}