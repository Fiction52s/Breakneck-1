#include "ScoreDisplay.h"
#include "Session.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "GameSession.h"
#include "SaveFile.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

ScoreDisplay::ScoreDisplay(Vector2f &position,
	sf::Font &testFont)
	:font(testFont)
{
	sess = Session::GetSession();
	basePos = position;

	ts_score = sess->GetSizedTileset("HUD/score_384x96.png");

	for (int i = 0; i < NUM_BARS; ++i)
	{
		bars[i] = new ScoreBar(i, this);
	}

	for (int i = 0; i < NUM_SELECT_BARS; ++i)
	{
		selectBars[i] = new SelectBar(i, this);
	}

	//bars[0]->SetText("", Color::White);
	//bars[1]->SetText("", Color::White);
	//bars[2]->SetText("", Color::White);


	selectOffset = NUM_BARS * 100 + 20;

	active = false;
	waiting = false;
}

ScoreDisplay::~ScoreDisplay()
{
	for (int i = 0; i < NUM_BARS; ++i)
	{
		delete bars[i];
	}

	for (int i = 0; i < NUM_SELECT_BARS; ++i)
	{
		delete selectBars[i];
	}
}

void ScoreDisplay::Draw(RenderTarget *target)
{
	if (active)
	{
		for (int i = 0; i < NUM_BARS; ++i)
		{
			bars[i]->Draw(target);
		}

		for (int i = 0; i < NUM_SELECT_BARS; ++i)
		{
			selectBars[i]->Draw(target);
		}

		if (bars[0]->state == ScoreBar::SHEET_DISPLAY || bars[0]->state ==
			ScoreBar::SYMBOL_DISPLAY)
		{
			
			
		}
		if (bars[1]->state == ScoreBar::SHEET_DISPLAY || bars[1]->state ==
			ScoreBar::SYMBOL_DISPLAY)
		{
			//target->draw(timeSprite);
			//target->draw(timeText);
			//target->draw(keys);
		}
		if (bars[2]->state == ScoreBar::SHEET_DISPLAY || bars[2]->state ==
			ScoreBar::SYMBOL_DISPLAY)
		{
			//target->draw(shardSprite);
			//target->draw(keys);
		}
	}
}

void ScoreDisplay::PopOutBar(int row)
{
	bars[row]->PopOut();
}

void ScoreDisplay::PopOutSelectBars()
{
	for (int i = 0; i < NUM_SELECT_BARS; ++i)
	{
		selectBars[i]->PopOut();
	}
}

void ScoreDisplay::Update()
{
	if (!active)
		return;

	bool allNone = true;
	bool allDisplay = true;
	for (int i = 0; i < NUM_BARS; ++i)
	{
		bars[i]->Update();
		if (bars[i]->state != ScoreBar::NONE)
		{
			allNone = false;
		}
		if (bars[i]->state != ScoreBar::SHEET_DISPLAY)
		{
			allDisplay = false;
		}

		
	}

	for (int i = 0; i < NUM_SELECT_BARS; ++i)
	{
		selectBars[i]->Update();
		if (selectBars[i]->state != SelectBar::NONE)
		{
			allNone = false;
		}
		if (selectBars[i]->state != SelectBar::DISPLAY)
		{
			allDisplay = false;
		}
	}
	

	if (allNone)
	{
		active = false;
	}
	if (allDisplay)
	{
		waiting = true;
	}

}

void ScoreDisplay::Reset()
{
	waiting = false;
	active = false;
	for (int i = 0; i < NUM_BARS; ++i)
	{
		bars[i]->Reset();
	}

	for (int i = 0; i < NUM_SELECT_BARS; ++i)
	{
		selectBars[i]->Reset();
	}
}

void ScoreDisplay::Activate()
{
	active = true;
	waiting = false;
	PopOutBar(0);
	//bars[0]->state = ScoreBar::POP_OUT;
	//bars[0]->frame = 0;

	/*timeText.setFont(font);
	timeText.setCharacterSize(40);
	timeText.setFillColor(Color::Black);

	
	keys.setFont(font);
	keys.setCharacterSize(40);
	keys.setFillColor(Color::Black);*/

	stringstream ss;

	//timeText.setString(GetTimeStr(sess->totalFramesBeforeGoal));
	
	ss.str("");
	//ss << sess->numKeysCollected << " / " << sess->numTotalKeys;
	//keys.setString(ss.str());

	//time.setString("HERE I AM BUDDY");

	//time.setString


	//for( int i = 0; i < NUM_BARS; ++i )
	//{
	//	bars[i]->state = ScoreBar::POP_OUT;
	//	bars[i]->frame = 0;
	//	//bars[i]->xDiffPos = 0;
	//}
	//state = POP_OUT;
	//frame = 0;
}

void ScoreDisplay::Deactivate()
{
	for (int i = 0; i < NUM_BARS; ++i)
	{
		bars[i]->Retract();
	}

	for (int i = 0; i < NUM_SELECT_BARS; ++i)
	{
		selectBars[i]->Retract();
	}

	waiting = false;
}

ScoreBar::ScoreBar(int p_row, ScoreDisplay *p_parent)
	:row(p_row), parent( p_parent )
{
	barSprite.setTexture(*parent->ts_score->texture);
	symbolSprite.setTexture(*parent->ts_score->texture);

	symbolSprite.setTextureRect(parent->ts_score->GetSubRect(12 + row));

	stateLength[NONE] = 1;
	stateLength[POP_OUT] = 30;
	stateLength[SHEET_APPEAR] = 12;
	stateLength[SYMBOL_DISPLAY] = 30;
	stateLength[SHEET_DISPLAY] = 1;
	stateLength[RETRACT] = 30;

	textColor = Color::Black;

	text.setFont(parent->font);
	text.setCharacterSize(40);
	text.setFillColor(textColor);

	Reset();
}

void ScoreBar::Reset()
{
	SetBarPos(0);
	state = NONE;
	frame = 0;
	symbolSprite.setColor(Color::Transparent);
}

void ScoreBar::SetBarPos(float xDiff)
{
	int rowHeight = 100;
	xDiffPos = xDiff;

	Vector2f newPos(parent->basePos.x + xDiffPos, parent->basePos.y + row * rowHeight);
	barSprite.setPosition(newPos);
	symbolSprite.setPosition(newPos);
	text.setPosition(newPos + Vector2f(150, 20));
}

void ScoreBar::Update()
{
	if (frame == stateLength[state])
	{
		frame = 0;
		switch (state)
		{
		case NONE:
			break;
		case POP_OUT:
			if (row < parent->NUM_BARS - 1)
			{
				parent->PopOutBar(row + 1);
			}
			else
			{
				parent->PopOutSelectBars();
			}
			state = SHEET_APPEAR;
			break;
		case SHEET_APPEAR:
			state = SYMBOL_DISPLAY;
			break;
		case SYMBOL_DISPLAY:
			state = SHEET_DISPLAY;
			break;
		case SHEET_DISPLAY:
			break;
		case RETRACT:
			state = NONE;
			break;
		}
	}

	switch (state)
	{
	case NONE:
	{
		break;
	}
	case POP_OUT:
	{
		CubicBezier bez(0, 0, 1, 1);
		float z = bez.GetValue((double)frame / (stateLength[POP_OUT] - 1 ));

		SetBarPos(-384.f * z);

		barSprite.setTextureRect(parent->ts_score->GetSubRect(0));

		break;
	}
	case SHEET_APPEAR:
	{
		barSprite.setTextureRect(parent->ts_score->GetSubRect(frame));
		break;
	}
	case SYMBOL_DISPLAY:
	{
		//if (!contBar)
		{
			int dispFrames = 30;
			CubicBezier bez(0, 0, 1, 1);
			float z = bez.GetValue((double)frame / dispFrames);
			symbolSprite.setColor(Color(255, 255, 255, z * 255.f));
			
			Color c = textColor;
			c.a = z * 255.f;
			text.setFillColor(c);
		}
		break;
	}
	case SHEET_DISPLAY:
	{
		break;
	}
	case RETRACT:
	{
		int retractFrames = 30;
		CubicBezier bez(0, 0, 1, 1);
		float z = bez.GetValue((double)frame / retractFrames);

		SetBarPos(-384 * (1 - z));
		break;
	}
	}

	++frame;
}

void ScoreBar::Draw(sf::RenderTarget *target)
{
	target->draw(barSprite);

	if (state == SYMBOL_DISPLAY || state == SHEET_DISPLAY || state == RETRACT)
	{
		target->draw(symbolSprite);
		target->draw(text);
	}
	
}

void ScoreBar::SetText(const std::string &str,
	sf::Color c)
{
	textColor = c;
	text.setString(str);
	text.setFillColor(textColor);
}

void ScoreBar::PopOut()
{
	Reset();
	state = POP_OUT;
	frame = 0;

	GameSession *game = GameSession::GetSession();

	int recordScore =
		game->saveFile->GetBestFramesLevel(game->level->index);

	if (row == 0)
	{
		if (game != NULL && game->saveFile != NULL)
		{
			if (recordScore > 0)
			{
				if (game->totalFramesBeforeGoal < recordScore)
				{
					SetText(GetTimeStr(game->totalFramesBeforeGoal),
						Color::Red);
				}
				else
				{
					SetText(GetTimeStr(recordScore),
						Color::White);
				}
			}
			else
			{
				SetText(GetTimeStr(game->totalFramesBeforeGoal),
					Color::Red);
			}
		}
		else
		{
			SetText("-----", Color::White);
		}
	}
	else if (row == 1)
	{
		if( game != NULL )
		{
			if (recordScore > 0)
			{
				if (game->totalFramesBeforeGoal < recordScore)
				{
					SetText(GetTimeStr(game->totalFramesBeforeGoal),
						Color::Red);
				}
				else
				{
					SetText(GetTimeStr(recordScore),
						Color::White);
				}
			}
			else
			{
				SetText(GetTimeStr(game->totalFramesBeforeGoal),
					Color::Red);
			}
		}
	}
	else if( row == 2 )
	{
		Session *sess = parent->sess;

		int total = sess->mapHeader->numShards;
		int currCaptured = 0;
		for (auto it = sess->mapHeader->shardInfoVec.begin();
			it != sess->mapHeader->shardInfoVec.end(); ++it)
		{
			if (sess->shardsCapturedField->GetBit((*it).GetTrueIndex()))
			{
				currCaptured++;
			}
		}

		stringstream ss;
		ss << currCaptured << "/" << total;
		SetText(ss.str(), Color::White );
		
	}
}

void ScoreBar::Retract()
{
	state = RETRACT;
	frame = 0;
}





SelectBar::SelectBar(int p_row, ScoreDisplay *p_parent)
	:row(p_row), parent(p_parent)
{
	barSprite.setTexture(*parent->ts_score->texture);
	barSprite.setTextureRect(parent->ts_score->GetSubRect(15 + row));
	buttonIconSprite.setTexture(*parent->sess->mainMenu->ts_buttonIcons->texture);
	

	stateLength[NONE] = 1;
	stateLength[POP_OUT] = 30;
	stateLength[DISPLAY] = 1;
	stateLength[RETRACT] = 30;

	Reset();
}

void SelectBar::Reset()
{
	SetBarPos(0);
	state = NONE;
	frame = 0;

	Session *sess = parent->sess;

	int tileOffset = 0;

	ControllerType ct = sess->GetController(0).GetCType();
	switch (ct)
	{
	case ControllerType::CTYPE_XBOX:
		tileOffset = 0;
		break;
	case ControllerType::CTYPE_GAMECUBE:
		tileOffset = 16 * 2;
		break;
	}
	
	int buttonIndex = 0;

	if (row == 0) //continue
	{
		buttonIndex = 0;
	}
	else if (row == 1) //exit
	{
		buttonIndex = 2;
	}
	else if (row == 2) //retry
	{
		buttonIndex = 1;
	}

	buttonIconSprite.setTextureRect(
		parent->sess->mainMenu->ts_buttonIcons->GetSubRect(tileOffset + buttonIndex));
	buttonIconSprite.setScale(.5, .5);
}

void SelectBar::SetBarPos(float xDiff)
{
	int rowHeight = 100;
	xDiffPos = xDiff;

	Vector2f newPos(parent->basePos.x + xDiffPos, 
		parent->basePos.y + row * rowHeight + parent->selectOffset);
	barSprite.setPosition(newPos);
	buttonIconSprite.setPosition(newPos + Vector2f( 320, 2 ));
}

void SelectBar::Update()
{
	if (frame == stateLength[state])
	{
		frame = 0;
		switch (state)
		{
		case NONE:
			break;
		case POP_OUT:
			state = DISPLAY;
			break;
		case DISPLAY:
			break;
		case RETRACT:
			state = NONE;
			break;
		}
	}

	switch (state)
	{
	case NONE:
	{
		break;
	}
	case POP_OUT:
	{
		CubicBezier bez(0, 0, 1, 1);
		float z = bez.GetValue((double)frame / (stateLength[POP_OUT] - 1));

		SetBarPos(-384.f * z);
		break;
	}
	case DISPLAY:
	{
		break;
	}
	case RETRACT:
	{
		int retractFrames = 30;
		CubicBezier bez(0, 0, 1, 1);
		float z = bez.GetValue((double)frame / retractFrames);

		SetBarPos(-384 * (1 - z));
		break;
	}
	}

	++frame;
}

void SelectBar::Draw(sf::RenderTarget *target)
{
	target->draw(barSprite);
	target->draw(buttonIconSprite);
}

void SelectBar::PopOut()
{
	Reset();
	state = POP_OUT;
	frame = 0;

}

void SelectBar::Retract()
{
	state = RETRACT;
	frame = 0;
}