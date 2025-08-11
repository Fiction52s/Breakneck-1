#include "TimerHUD.h"
#include "ImageText.h"
#include "Session.h"
#include "PaletteShader.h"
#include "Actor.h"

using namespace std;
using namespace sf;

TimerHUD::TimerHUD(TilesetManager *tm, bool p_modifier, bool p_showMinutes )
	:growBez( 0, 0, 1, 1 ), shrinkBez( 0, 0, 1, 1 )
{
	modifier = p_modifier;
	showMinutes = p_showMinutes;
	sess = NULL;//Session::GetSession();
	ts_text = tm->GetSizedTileset("HUD/timer_numbers_54x64.png");//tm->GetSizedTileset("HUD/timer_numbers_48x64.png");

	timer = NULL;
	secondOnlyTimer = NULL;

	if (showMinutes)
	{
		timer = new TimerText(ts_text);
	}
	else
	{
		secondOnlyTimer = new ImageText(2, ts_text);
	}

	countingUp = false;

	tracking = false;

	//center = Vector2f(1920 / 2, 50);

	textShader = new PaletteShader("timer", "Resources/HUD/timer_numbers_palette_2x1.png");

	textShader->SetPaletteIndex(0);
	textShader->SetTileset(ts_text);

	if (modifier)
	{
		baseScale = .6f;
		grownScale = .6f;//1.5f;
		//center.x += 250;
		//timer->SetSymbol(ImageText::SYMBOL_PLUS);
	}
	else
	{
		baseScale = 1.f;
		grownScale = 1.5f;
	}
	
	currScale = baseScale;

	actionLength[NORMAL] = 1;
	actionLength[GROW] = 5;
	actionLength[LARGE] = 1;
	actionLength[SHRINK] = 5;
	actionLength[HIDDEN] = 1;
	actionLength[SHOW_MODIFIER] = 60;

	if (timer != NULL)
	{
		timer->SetNumber(0);
		//timer->SetShader(&textShader->pShader);
	}
	else if (secondOnlyTimer != NULL)
	{
		secondOnlyTimer->SetNumber(0);
		secondOnlyTimer->SetSpacingFactor(1.f);
		//secondOnlyTimer->SetShader(&textShader->pShader);
		secondOnlyTimer->ShowZeroes(2);
	}
	

	centiSecondTimer = new ImageText( 2, ts_text);
	centiSecondTimer->SetSpacingFactor(1.f);
	centiSecondTimer->SetNumber(0);
	centiSecondTimer->ShowZeroes(2);
	//centiSecondTimer->SetShader(&textShader->pShader);

	currNumFrames = 0;
	centiScale = .5;

	Reset();
}

TimerHUD::~TimerHUD()
{
	if (timer != NULL)
	{
		delete timer;
	}

	if (secondOnlyTimer != NULL)
	{
		delete secondOnlyTimer;
	}
	
	delete centiSecondTimer;
	delete textShader;
}

void TimerHUD::SetSession(Session *p_sess)
{
	sess = p_sess;
}

void TimerHUD::Reset()
{
	if( modifier )
	{
		action = HIDDEN;
	}
	else
	{
		action = NORMAL;
	}
	
	frame = 0;
	SetScale(baseScale);
	SetColor(Color::Cyan);
}

void TimerHUD::UpdatePixelPos(sf::RenderTarget *target)
{
	pixelPos = Vector2f(target->mapCoordsToPixel(trackingPos));
}

void TimerHUD::SetTrackingPos(sf::Vector2f tPos)
{
	trackingPos = tPos;
	tracking = true;
}

void TimerHUD::SetHoverOffset(sf::Vector2f hOffset)
{
	hoverOffset = hOffset;
}

void TimerHUD::SetNumFrames(int frames)
{
	currNumFrames = frames;
	int currSeconds = frames / 60;

	int remain = frames % 60;
	int centiSecond = floor((double)remain * (1.0 / 60.0 * 100.0) + .5);


	if (showMinutes)
	{
		timer->SetNumber(currSeconds);
	}
	else
	{
		secondOnlyTimer->SetNumber(currSeconds);
	}

	/*int frameTest = frames % 60;
	float frameProp = frameTest / 60.f;
	frameProp *= 100.f;
	int centiNumber = frameProp;*/

	centiSecondTimer->SetNumber(centiSecond);

	if (!modifier && !countingUp)
	{
		float growThresh = 60 * 10;
		if (action == NORMAL && frames <= growThresh)
		{
			Grow();
		}
		else if (action == LARGE && frames > growThresh)
		{
			Shrink();
		}

		if (frames <= sess->GetPlayer(0)->GetNumMaxSurvivalFrames()) //survival
		{
			SetColor(sess->GetPlayer(0)->currentDespColor);
		}
		else
		{
			SetColor(Color::Cyan);
		}
	}
}

void TimerHUD::SetColor(sf::Color c)
{
	textShader->SetColor(1, c);
}

void TimerHUD::SetModifiedPlus(int frames)
{
	assert(showMinutes);

	if (action == SHOW_MODIFIER && timer->symbolType == ImageText::SYMBOL_PLUS)
	{
		frames += currNumFrames;
	}

	assert(modifier);
	timer->SetSymbol(ImageText::SYMBOL_PLUS);
	SetColor(Color::Green);
	action = SHOW_MODIFIER;
	frame = 0;
	SetNumFrames(frames);
	SetScale(baseScale);
	UpdateSprite();
}

void TimerHUD::SetModifiedMinus(int frames)
{
	assert(showMinutes);

	if (action == SHOW_MODIFIER && timer->symbolType == ImageText::SYMBOL_MINUS)
	{
		frames += currNumFrames;
	}

	assert(modifier);
	timer->SetSymbol(ImageText::SYMBOL_MINUS);
	SetColor(Color::Red);
	action = SHOW_MODIFIER;
	frame = 0;
	SetNumFrames(frames);
	SetScale(baseScale);
	UpdateSprite();
}

void TimerHUD::SetCenter(sf::Vector2f &p_center)
{
	center = p_center;
	SetScale(currScale);
}

sf::Vector2f TimerHUD::GetAdjustedCenter()
{
	Vector2f adjustedCenter = center;

	if (currScale > baseScale)
	{
		adjustedCenter.y += (ts_text->tileHeight / 2) * (currScale - baseScale);
	}

	return adjustedCenter;
}

void TimerHUD::SetScale(float f)
{
	currScale = f;

	Vector2f adjustedCenter = GetAdjustedCenter();
	Vector2f centiTopLeft;
	if (showMinutes)
	{
		timer->SetCenter(adjustedCenter);
		timer->SetScale(f);
		centiTopLeft = adjustedCenter + Vector2f(ts_text->tileWidth * f * 2.5, ts_text->tileHeight * f * (.5 - centiScale));
	}
	else
	{
		secondOnlyTimer->SetCenter(adjustedCenter);
		secondOnlyTimer->SetScale(f);
		centiTopLeft = adjustedCenter + Vector2f(ts_text->tileWidth * f * 1.0, ts_text->tileHeight * f * (.5 - centiScale));
	}
	

	if (modifier && timer->symbolType != ImageText::SYMBOL_NONE)
	{
		centiTopLeft.x += ts_text->tileWidth * f;
	}

	centiSecondTimer->SetScale(centiScale * f);
	centiSecondTimer->SetTopLeft(centiTopLeft);
}

sf::Vector2f TimerHUD::GetRightCenter()
{
	float rightWidth = (ts_text->tileWidth * 2.5 * currScale)
		+ (ts_text->tileWidth * 2 * currScale * centiScale);

	
	return GetAdjustedCenter() + Vector2f(rightWidth, 0);//Vector2f(center.x + rightWidth, center.y);
	//return center + 
}

void TimerHUD::UpdateSprite()
{
	if (showMinutes)
	{
		timer->UpdateSprite();
	}
	else
	{
		secondOnlyTimer->UpdateSprite();
	}
	
	centiSecondTimer->UpdateSprite();
}

void TimerHUD::Drain(int frameDamage)
{
	//draw some kind of drain thing here
}

void TimerHUD::Grow()
{
	if (action == SHRINK)
	{
		if (frame == 0)
		{
			action = LARGE;
			frame = 0;
		}
		else
		{
			float amountDone = frame / (float)actionLength[action];
			float amountShrunken = 1.f - amountDone;
			action = GROW;
			frame = actionLength[GROW] * amountShrunken;
		}
	}
	else if (action == NORMAL)
	{
		action = GROW;
		frame = 0;
	}
}

void TimerHUD::Shrink()
{
	if (action == GROW)
	{
		if (frame == 0)
		{
			action = NORMAL;
			frame = 0;
		}
		else
		{
			float amountDone = frame / (float)actionLength[action];
			float amountShrunken = 1.f - amountDone;
			action = SHRINK;
			frame = actionLength[SHRINK] * amountShrunken;
		}
	}
	else if (action == LARGE)
	{
		action = SHRINK;
		frame = 0;
	}
}


void TimerHUD::Update()
{
	if (frame == actionLength[action])
	{
		frame = 0;

		switch (action)
		{
		case GROW:
			action = LARGE;
			SetScale(grownScale);
			break;
		case SHRINK:
			action = NORMAL;
			SetScale(baseScale);
			break;
		case SHOW_MODIFIER:
			action = HIDDEN;
			frame = 0;
			break;
		}
	}

	float factor = frame / (float)actionLength[action];
	

	switch (action)
	{
	case NORMAL:
		break;
	case GROW:
	{
		float growFactor = growBez.GetValue(factor);
		SetScale(baseScale *  (1.f - growFactor) + grownScale * growFactor);
		break;
	}
	case LARGE:
		break;
	case SHRINK:
	{
		float shrinkFactor = shrinkBez.GetValue(factor);
		SetScale(baseScale * shrinkFactor + grownScale * (1.f - shrinkFactor));
		break;
	}
		
	}

	UpdateSprite();

	++frame;
}

void TimerHUD::Draw(RenderTarget * target)
{
	if (action != HIDDEN)
	{
		if (tracking)
		{
			SetCenter(pixelPos + hoverOffset);
			//UpdateSprite();
		}
		
		if (showMinutes)
		{
			timer->Draw(target);
		}
		else
		{
			secondOnlyTimer->Draw(target);
		}
		centiSecondTimer->Draw(target);
	}
}

//TimerModifierHUD::TimerModifierHUD( TimerHUD *p_tHUD )
//	:tHUD( p_tHUD )
//{
//	modifierTimer = new ImageText( 2, ts_text);
//	modifierTimer->SetSymbol(ImageText::SYMBOL_PLUS);
//	modifierTextShader = new PaletteShader("timer", "Resources/HUD/numbers_palette_2x1.png");
//
//	modifierCentiSecondTimer = new ImageText(2, ts_text);
//	modifierCentiSecondTimer->SetNumber(0);
//	modifierCentiSecondTimer->ShowZeroes(2);
//
//	modifierTimer->SetShader(&modifierTextShader->pShader);
//	modifierCentiSecondTimer->SetShader(&modifierTextShader->pShader);
//}
//
//TimerModifierHUD::~TimerModifierHUD()
//{
//	delete modifierTimer;
//	delete modifierTextShader;
//}
//
//void TimerModifierHUD::Update()
//{
//	if (frame == actionLength[action])
//	{
//		frame = 0;
//	}
//
//	//float factor = frame / (float)actionLength[action];
//
//	UpdateSprite();
//
//	++frame;
//}
//
//void TimerModifierHUD::UpdateSprite()
//{
//	modifierTimer->UpdateSprite();
//	modifierCentiSecondTimer->UpdateSprite();
//}
//
//void TimerModifierHUD::SetNumFrames(int frames)
//{
//
//}
//
//void TimerModifierHUD::Draw(sf::RenderTarget *target)
//{
//	modifierTimer->Draw(target);
//	modifierCentiSecondTimer->Draw(target);
//}