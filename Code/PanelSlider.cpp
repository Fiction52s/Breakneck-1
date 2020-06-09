#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

PanelSlider::PanelSlider(Panel *p, sf::Vector2i &p_origPos, sf::Vector2i &p_destPos)
	:panel(p), slid(false)
{
	origPos = p_origPos;//Vector2i(-270, 0);
	destPos = p_destPos;
	panel->SetPosition(origPos);
	normalDuration = 10;
	bez = CubicBezier(0, 0, 1, 1);
}


bool PanelSlider::MouseUpdate()
{
	if (!slid)
	{
		slid = true;
		int duration = normalDuration;// - outFrame;
		int skip = 0;
		if (panel->IsSliding())
		{
			skip = normalDuration - panel->slideFrame;
		}
		panel->SetPosition(origPos);
		panel->Slide(destPos, bez, duration);
		panel->slideFrame = skip;
	}

	return true;
}

void PanelSlider::Deactivate()
{
	int duration = normalDuration;
	int skip = 0;
	if (panel->IsSliding())
	{
		skip = normalDuration - panel->slideFrame;
	}
	panel->SetPosition(destPos);
	panel->Slide(origPos, bez, duration);
	panel->slideFrame = skip;
	slid = false;
}