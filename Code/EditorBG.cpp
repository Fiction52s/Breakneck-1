#include "EditorBG.h"
#include "Background.h"
#include "Parallax.h"

using namespace sf;
using namespace std;

EditorBG::EditorBG()
{
	currBackground = NULL;
	showBG = false;

	bgView.setSize(1920, 1080);
	bgView.setCenter(960, 0);
}

void EditorBG::Show(bool s)
{
	showBG = s;
}

void EditorBG::FlipShown()
{
	showBG = !showBG;
}

EditorBG::~EditorBG()
{
	if (currBackground != NULL)
	{
		delete currBackground;
	}

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		delete (*it);
	}
}

void EditorBG::Draw(sf::RenderTarget *target)
{
	if (!showBG)
		return;

	if (currBackground != NULL)
	{
		currBackground->Draw(target);
	}

	if( scrollingBackgrounds.size() > 0 )
	{
		sf::View oldView = target->getView();

		target->setView(bgView);
		for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
			it != scrollingBackgrounds.end(); ++it)
		{
			(*it)->Draw(target);
		}
		target->setView(oldView);
	}
}