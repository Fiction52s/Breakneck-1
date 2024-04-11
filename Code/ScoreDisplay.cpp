#include "ScoreDisplay.h"
#include "Session.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "GameSession.h"
#include "EditSession.h"
#include "SaveFile.h"
#include "MapHeader.h"
#include "AdventureManager.h"
#include "FeedbackInputBox.h"
#include "Medal.h"
#include "MedalSequence.h"

using namespace std;
using namespace sf;

ScoreDisplay::ScoreDisplay(Vector2f &position,
	sf::Font &testFont)
	:font(testFont)
{
	feedbackInputBox = new FeedbackInputBox;

	feedbackInputBox->SetCenter(Vector2f(960, 100));

	sess = Session::GetSession();
	game = GameSession::GetSession();
	basePos = position;

	basePos += Vector2f(0, 80); //for name box

	if (sess->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && sess->mainMenu->adventureManager != NULL)
	{
		ts_score = sess->mainMenu->adventureManager->GetSizedTileset("HUD/score_384x96.png");
		ts_scoreIcons = sess->mainMenu->adventureManager->GetSizedTileset("HUD/score_icons_128x96.png");
	}
	else
	{
		ts_score = sess->GetSizedTileset("HUD/score_384x96.png");
		ts_scoreIcons = sess->GetSizedTileset("HUD/score_icons_128x96.png");
	}
	
	medalSeq = new MedalSequence;
	medalSeq->Init();

	for (int i = 0; i < NUM_BARS; ++i)
	{
		bars[i] = new ScoreBar(i, this);
	}

	numShownSelectBars = NUM_SELECT_BARS;
	if (!MainMenu::GetInstance()->steamOn)
	{
		numShownSelectBars = NUM_SELECT_BARS - 1;
	}

	for (int i = 0; i < numShownSelectBars; ++i)
	{
		selectBars[i] = new SelectBar(i, this);
	}

	levelNameText.setFont(sess->mainMenu->arial);
	levelNameText.setFillColor(Color::White);
	levelNameText.setCharacterSize(40);

	SetRectColor(nameBGQuad, Color::Black);

	selectOffset = NUM_BARS * 100 + 20;

	includeExtraSelectBars = false;

	Reset();
}

ScoreDisplay::~ScoreDisplay()
{
	delete feedbackInputBox;

	delete medalSeq;

	for (int i = 0; i < NUM_BARS; ++i)
	{
		delete bars[i];
	}

	for (int i = 0; i < numShownSelectBars; ++i)
	{
		delete selectBars[i];
	}
}

void ScoreDisplay::Draw(RenderTarget *target)
{
	if (IsActive())
	{
		target->draw(nameBGQuad, 4, sf::Quads);
		target->draw(levelNameText);


		if (action == A_WAIT && MainMenu::GetInstance()->gameRunType == MainMenu::GameRunType::GRT_ADVENTURE )
		{
			feedbackInputBox->Draw(target);
		}

		for (int i = 0; i < NUM_BARS; ++i)
		{
			bars[i]->Draw(target);
		}
		
		int activeSelectBars = GetNumSelectBars();
		for (int i = 0; i < numShownSelectBars; ++i)
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

		if (action == A_GIVE_GOLD || action == A_GIVE_SILVER || action == A_GIVE_BRONZE )
		{
			medalSeq->Draw(target, EffectLayer::UI_FRONT );
		}
	}
}

void ScoreDisplay::PopOutBar(int row)
{
	bars[row]->PopOut();
}

void ScoreDisplay::PopOutSelectBars()
{
	int numSelectBars = GetNumSelectBars();
	for (int i = 0; i < numSelectBars; ++i)
	{
		selectBars[i]->PopOut();
	}
}

int ScoreDisplay::GetNumSelectBars()
{
	int selectBarNum = numShownSelectBars;
	if (!includeExtraSelectBars)
	{
		selectBarNum = 3;
	}

	return selectBarNum;
}

bool ScoreDisplay::IsActive()
{
	return action != A_IDLE;
}

bool ScoreDisplay::IsWaiting()
{
	return action == A_WAIT;
}

void ScoreDisplay::Update()
{
	if (!IsActive())
		return;

	if (action == A_ENTER )//|| action == A_WAIT )
	{
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

		int activeSelectBars = GetNumSelectBars();
		for (int i = 0; i < activeSelectBars; ++i)
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
			action = A_IDLE;
			frame = 0;
		}

		if (allDisplay)
		{
			if (gotGold)
			{
				action = A_GIVE_GOLD;
				frame = 0;
				medalSeq->Reset();

				medalSeq->StartGold(sess->mapHeader->goldRewardShardInfo.world, sess->mapHeader->goldRewardShardInfo.localIndex);
			}
			else if (gotSilver)
			{
				action = A_GIVE_SILVER;
				frame = 0;
				medalSeq->Reset();
				medalSeq->StartSilver();
			}
			else if (gotBronze)
			{
				action = A_GIVE_BRONZE;
				frame = 0;
				medalSeq->Reset();
				medalSeq->StartBronze();
			}
			else
			{
				action = A_WAIT;
				frame = 0;
			}
		}
	}
	else if (action == A_GIVE_GOLD)
	{
		if (medalSeq->Update())
		{

		}
		else
		{
			action = A_WAIT;
			frame = 0;
		}
		/*if (medalSeq->IsLastFrame())
		{
			action = A_WAIT;
			frame = 0;
		}
		else
		{
			medalSeq->Update();
		}*/
	}
	else if (action == A_GIVE_SILVER)
	{
		if (medalSeq->Update())
		{

		}
		else
		{
			action = A_WAIT;
			frame = 0;
		}
		/*if (medalSeq->IsLastFrame())
		{
			action = A_WAIT;
			frame = 0;
		}
		else
		{
			medalSeq->Update();
		}*/
	}
	else if (action == A_GIVE_BRONZE)
	{
		if (medalSeq->Update())
		{

		}
		else
		{
			action = A_WAIT;
			frame = 0;
		}
		/*if (medalSeq->IsLastFrame())
		{
			action = A_WAIT;
			frame = 0;
		}
		else
		{
			medalSeq->Update();
		}*/
	}

	++frame;
}

void ScoreDisplay::Reset()
{
	action = A_IDLE;
	frame = 0;
	madeRecord = false;
	gotGold = false;
	gotSilver = false;
	gotBronze = false;
	for (int i = 0; i < NUM_BARS; ++i)
	{
		bars[i]->Reset();
	}

	for (int i = 0; i < numShownSelectBars; ++i)
	{
		selectBars[i]->Reset();
	}
}

void ScoreDisplay::Activate()
{
	action = A_ENTER;
	frame = 0;

	GameSession *game = GameSession::GetSession();
	EditSession *edit = EditSession::GetSession();

	if (game != NULL)
	{
		levelNameText.setString(game->mapHeader->fullName);
	}
	else if (edit != NULL)
	{
		levelNameText.setString(edit->mapHeader->fullName);
	}
	

	auto lb = levelNameText.getLocalBounds();
	//if( levelNameText.getGlobalBounds().)
	levelNameText.setOrigin(lb.left + lb.width, lb.top + lb.height / 2);
	//levelNameText.setOrigin(levelNameText.getLocalBounds().left + levelNameText.getLocalBounds().width / 2, 
	//	levelNameText.getLocalBounds().top + levelNameText.getLocalBounds().height / 2 );
	levelNameText.setPosition(basePos + Vector2f( -20, -40 ));

	auto gb = levelNameText.getGlobalBounds();
	SetRectTopLeft(nameBGQuad, gb.width + 20, gb.height + 20, Vector2f( gb.left - 10, gb.top - 10 ));


	MainMenu *mm = MainMenu::GetInstance();

	includeExtraSelectBars = false;

	if (mm->gameRunType == MainMenu::GameRunType::GRT_ADVENTURE)
	{
		if (game != NULL)
		{
			int score = game->saveFile->GetBestFramesLevel(game->level->index);
			if (score > 0)
			{
				includeExtraSelectBars = true;
			}
		}
	}

	PopOutBar(0);

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

	for (int i = 0; i < numShownSelectBars; ++i)
	{
		selectBars[i]->Retract();
	}

	action = A_IDLE;
	frame = 0;
}

ScoreBar::ScoreBar(int p_row, ScoreDisplay *p_parent)
	:row(p_row), parent( p_parent )
{
	barSprite.setTexture(*parent->ts_score->texture);
	symbolSprite.setTexture(*parent->ts_scoreIcons->texture);
	extraSymbolSprite.setTexture(*parent->ts_scoreIcons->texture);

	symbolSprite.setTextureRect(parent->ts_scoreIcons->GetSubRect(row));

	if (row == 2)
	{
		extraSymbolSprite.setTextureRect(parent->ts_scoreIcons->GetSubRect(row + 1));
	}
	

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

	extraText.setFont(parent->font);
	extraText.setCharacterSize(40);
	extraText.setFillColor(textColor);

	Reset();
}

void ScoreBar::Reset()
{
	SetBarPos(0);
	state = NONE;
	frame = 0;
	symbolSprite.setColor(Color::Transparent);
	extraSymbolSprite.setColor(Color::Transparent);
}

void ScoreBar::SetBarPos(float xDiff)
{
	int rowHeight = 100;
	xDiffPos = xDiff;

	Vector2f newPos(parent->basePos.x + xDiffPos, parent->basePos.y + row * rowHeight);
	barSprite.setPosition(newPos);
	symbolSprite.setPosition(newPos + Vector2f(25, 0));

	if (row == 0 || row == 1)
	{
		text.setPosition(newPos + Vector2f(150, 20));
	}
	else
	{
		text.setPosition(newPos + Vector2f(130, 20));
	}
	

	extraSymbolSprite.setPosition(symbolSprite.getPosition() + Vector2f(160, 0));
	extraText.setPosition(text.getPosition() + Vector2f(160, 0));
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

			extraSymbolSprite.setColor(symbolSprite.getColor());
			
			Color c = textColor;
			c.a = z * 255.f;
			text.setFillColor(c);

			extraText.setFillColor(text.getFillColor());
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

		if (row == 2)
		{
			target->draw(extraSymbolSprite);
			target->draw(extraText);
		}
	}
	
}

void ScoreBar::SetText(const std::string &str,
	sf::Color c)
{
	textColor = c;
	text.setString(str);
	text.setFillColor(textColor);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(2);
}

void ScoreBar::PopOut()
{
	Reset();
	state = POP_OUT;
	frame = 0;

	GameSession *game = GameSession::GetSession();

	int recordScore = 0;
	if (game != NULL)
	{
		GameSession *topParent = game->GetTopParentGame();
		
		if (game->saveFile != NULL)
		{
			recordScore = game->saveFile->GetBestFramesLevel(game->level->index);//topParent->level->index);
		}
	}
	

	if (row == 0)
	{
		if (game != NULL && game->saveFile != NULL )
		{
			if (parent->madeRecord)
			{
				SetText(GetTimeStr(recordScore),
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
			SetText("-----", Color::White);
		}
	}
	else if (row == 1)
	{
		if( game != NULL && !game->usedWarp)
		{
			if (parent->madeRecord)
			{
				SetText(GetTimeStr(recordScore),
					Color::Red);
			}
			else
			{
				SetText(GetTimeStr(game->totalFramesBeforeGoal),
					Color::White);
			}
		}
		else
		{
			Session *sess = Session::GetSession();
			if (sess->usedWarp)
			{
				SetText("-----", Color::White);
			}
			else
			{
				SetText(GetTimeStr(sess->totalFramesBeforeGoal),
					Color::White);
			}
			
		}
	}
	else if( row == 2 )
	{
		Session *sess = parent->sess;

		
		int totalShards = 0;//sess->mapHeader->numShards;
		int currShardsCaptured = 0;
		/*for (auto it = sess->mapHeader->shardInfoVec.begin();
			it != sess->mapHeader->shardInfoVec.end(); ++it)
		{
			if (sess->IsShardCaptured( (*it).GetTrueIndex() ) )
			{
				currShardsCaptured++;
			}
		}*/

		stringstream ss;
		ss << currShardsCaptured << "/" << totalShards;
		SetText(ss.str(), Color::White);


		int totalLogs = 0;// sess->mapHeader->numLogs;
		int currLogsCaptured = 0;
		/*
		for (auto it = sess->mapHeader->logInfoVec.begin();
			it != sess->mapHeader->logInfoVec.end(); ++it)
		{
			if (sess->HasLog((*it).GetTrueIndex()))
			{
				currLogsCaptured++;
			}
		}*/

		ss.str("");
		ss.clear();

		ss << currLogsCaptured << "/" << totalLogs;

		extraText.setString(ss.str());
		extraText.setFillColor(textColor);
		extraText.setOutlineColor(Color::Black);
		extraText.setOutlineThickness(2);
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
	extraText.setFont(MainMenu::GetInstance()->arial);
	extraText.setCharacterSize(20);
	extraText.setFillColor(Color::White);

	barSprite.setTexture(*parent->ts_score->texture);
	if (row <= 2)
	{
		int barStartTile = 12;
		barSprite.setTextureRect(parent->ts_score->GetSubRect(barStartTile + row * 2));
	}
	else
	{
		int barStartTile = 20; //after PLAY
		int r = row - 3;
		barSprite.setTextureRect(parent->ts_score->GetSubRect(barStartTile + r * 2));
	}
	
	
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
	buttonIconSprite.setTexture(*sess->GetButtonIconTileset(0)->texture);
	
	auto button = XBOX_BLANK;

	if (row == 0) //continue
	{
		button = XBOX_A;//ControllerSettings::BUTTONTYPE_JUMP;//0
	}
	else if (row == 1) //exit
	{
		button = XBOX_B;//ControllerSettings::BUTTONTYPE_DASH;//2;
	}
	else if (row == 2) //retry
	{
		button = XBOX_X;//ControllerSettings::BUTTONTYPE_SPECIAL;//1;
	}
	else if (row == 3)//watch
	{
		button = XBOX_R1;//ControllerSettings::BUTTONTYPE_ATTACK;
	}
	else if (row == 4)//race ghost
	{
		button = XBOX_Y;//ControllerSettings::BUTTONTYPE_SHIELD;
	}
	else if (row == 5)
	{
		button = XBOX_START;
	}

	IntRect ir = sess->mainMenu->GetButtonIconTileForMenu(sess->controllerStates[0]->GetControllerType(), button);//sess->mainMenu->adventureManager->controllerInput, button);
	buttonIconSprite.setTextureRect(ir);

	if (ir.width == 128)
	{
		buttonIconSprite.setScale(.5, .5);
	}
	else
	{
		buttonIconSprite.setScale(1.0, 1.0);
	}
}

void SelectBar::SetBarPos(float xDiff)
{
	int rowHeight = 100;
	xDiffPos = xDiff;

	Vector2f newPos(parent->basePos.x + xDiffPos, 
		parent->basePos.y + row * rowHeight + parent->selectOffset);
	barSprite.setPosition(newPos);
	buttonIconSprite.setPosition(newPos + Vector2f( 320, 2 ));

	extraText.setPosition(newPos + Vector2f(278 + 20, 35));
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

	if (row == 4)//race ghost
	{
		GameSession *game = GameSession::GetSession();
		if (game != NULL)
		{
			int numGhosts = game->GetNumPotentialGhosts();

			if (numGhosts > 1)
			{
				extraText.setString("(" + to_string(numGhosts) + ")");
				auto lb = extraText.getLocalBounds();
				extraText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
			}
			else
			{
				extraText.setString("");
			}
			
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
	target->draw(extraText);
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