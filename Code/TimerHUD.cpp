#include "TimerHUD.h"
#include "ImageText.h"
#include "Session.h"

using namespace std;
using namespace sf;

TimerHUD::TimerHUD()
{
	sess = Session::GetSession();
	ts_text = sess->GetSizedTileset("HUD/number_score_80x80.png");
	timer = new TimerText(ts_text);

	Vector2f center = Vector2f(1920 / 2, 100);

	timer->SetNumber(0);
	timer->SetCenter( center );

	float centiScale = .5;

	Vector2f centiTopLeft = center + Vector2f(ts_text->tileWidth * 2.5, ts_text->tileHeight * (.5 - centiScale ));

	centiSecondTimer = new ImageText( 2, ts_text);
	centiSecondTimer->SetScale(centiScale);
	centiSecondTimer->SetTopLeft(centiTopLeft);
	centiSecondTimer->SetNumber(0);
	centiSecondTimer->ShowZeroes(2);
}

TimerHUD::~TimerHUD()
{
	delete timer;
	delete centiSecondTimer;
}

void TimerHUD::SetNumFrames(int frames)
{
	int currSeconds = frames / 60;

	timer->SetNumber(currSeconds);

	int frameTest = frames % 60;
	float frameProp = frameTest / 60.f;
	frameProp *= 100.f;
	int centiNumber = frameProp;

	centiSecondTimer->SetNumber(centiNumber);
}

void TimerHUD::UpdateSprite()
{
	timer->UpdateSprite();
	centiSecondTimer->UpdateSprite();
}

void TimerHUD::Drain(int frameDamage)
{
	//draw some kind of drain thing here
}

void TimerHUD::Update()
{
	UpdateSprite();
}

void TimerHUD::Draw(RenderTarget * target)
{
	timer->Draw(target);
	centiSecondTimer->Draw(target);
}