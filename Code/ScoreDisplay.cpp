#include "ScoreDisplay.h"
#include "GameSession.h"
#include "Tileset.h"

using namespace std;
using namespace sf;

ScoreDisplay::ScoreDisplay(GameSession *p_owner, Vector2f &position,
	sf::Font &testFont)
	:scoreBarVA(sf::Quads, 4 * (NUM_BARS+1)), scoreSymbolsVA(sf::Quads, 4 * NUM_BARS), scoreSheetVA(sf::Quads, 4 * NUM_BARS), font(testFont)
{
	basePos = position;
	owner = p_owner;
	ts_scoreBar = owner->GetTileset("score_bar_384x80.png", 384, 80);
	ts_scoreContinue = owner->GetTileset("score_continue_384x80.png", 384, 80);
	ts_scoreSheet = owner->GetTileset("score_sheet_384x80.png", 384, 80);
	ts_scoreSymbols = owner->GetTileset("score_symbol_384x80.png", 384, 80);

	//scoreContinue.setTexture(*ts_scoreContinue->texture);
	//scoreContinue.setTextureRect(ts_scoreContinue->GetSubRect(0));
	scoreContinue.setOrigin(scoreContinue.getLocalBounds().width, 0);
	scoreContinue.setPosition(1920, 400);

	IntRect ir = ts_scoreBar->GetSubRect(0);
	IntRect irCont = ts_scoreBar->GetSubRect(1);

	for (int i = 0; i < NUM_BARS; ++i)
	{
		//SetRectSubRect(scoreBarVA + i * 4, ir);
		scoreBarVA[i * 4 + 0].texCoords = Vector2f(ir.left, ir.top);
		scoreBarVA[i * 4 + 1].texCoords = Vector2f(ir.left + ir.width, ir.top);
		scoreBarVA[i * 4 + 2].texCoords = Vector2f(ir.left + ir.width, ir.top + ir.height);
		scoreBarVA[i * 4 + 3].texCoords = Vector2f(ir.left, ir.top + ir.height);

		IntRect ir1 = ts_scoreSymbols->GetSubRect(0);
		scoreSymbolsVA[i * 4 + 0].texCoords = Vector2f(ir1.left, ir1.top);
		scoreSymbolsVA[i * 4 + 1].texCoords = Vector2f(ir1.left + ir1.width, ir1.top);
		scoreSymbolsVA[i * 4 + 2].texCoords = Vector2f(ir1.left + ir1.width, ir1.top + ir1.height);
		scoreSymbolsVA[i * 4 + 3].texCoords = Vector2f(ir1.left, ir1.top + ir1.height);

		bars[i] = new ScoreBar(i, this);
		bars[i]->SetSymbolTransparency(0);
	}

	scoreBarVA[NUM_BARS * 4 + 0].texCoords = Vector2f(irCont.left, irCont.top);
	scoreBarVA[NUM_BARS * 4 + 1].texCoords = Vector2f(irCont.left + irCont.width, irCont.top);
	scoreBarVA[NUM_BARS * 4 + 2].texCoords = Vector2f(irCont.left + irCont.width, irCont.top + irCont.height);
	scoreBarVA[NUM_BARS * 4 + 3].texCoords = Vector2f(irCont.left, irCont.top + irCont.height);
	bars[NUM_BARS] = new ScoreBar(NUM_BARS, this, true );

	active = false;
	waiting = false;
	//SetScoreBarPos( 0, 0 );
}

ScoreDisplay::~ScoreDisplay()
{
	for (int i = 0; i <= NUM_BARS; ++i)
	{
		delete bars[i];
	}
}

void ScoreDisplay::Draw(RenderTarget *target)
{
	if (active)
	{
		target->draw(scoreBarVA, ts_scoreBar->texture);
		target->draw(scoreSheetVA, ts_scoreSheet->texture);
		target->draw(scoreSymbolsVA, ts_scoreSymbols->texture);
		//target->draw(scoreContinue);

		if (bars[0]->state == ScoreBar::SHEET_DISPLAY || bars[0]->state ==
			ScoreBar::SYMBOL_DISPLAY)
		{
			target->draw(time);
		}
		if (bars[1]->state == ScoreBar::SHEET_DISPLAY || bars[1]->state ==
			ScoreBar::SYMBOL_DISPLAY)
		{
			target->draw(keys);
		}
	}
}

void ScoreDisplay::Update()
{
	if (!active)
		return;

	bool allNone = true;
	bool allDisplay = true;
	for (int i = 0; i < NUM_BARS+1; ++i)
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
	active = false;
	for (int i = 0; i < NUM_BARS+1; ++i)
	{
		bars[i]->state = ScoreBar::NONE;
		bars[i]->frame = 0;
		bars[i]->xDiffPos = 0;
		if( !bars[i]->contBar )
			bars[i]->SetSymbolTransparency(0);
	}
}

void ScoreDisplay::Activate()
{
	active = true;
	waiting = false;
	bars[0]->state = ScoreBar::POP_OUT;
	bars[0]->frame = 0;

	time.setFont(font);
	time.setCharacterSize(40);
	time.setFillColor(Color::Black);

	
	keys.setFont(font);
	keys.setCharacterSize(40);
	keys.setFillColor(Color::Black);

	stringstream ss;

	int seconds = owner->totalGameFrames / 60;
	int remain = owner->totalGameFrames % 60;
	int centiSecond = floor((double)remain * (1.0 / 60.0 * 100.0) + .5);

	if (seconds < 10)
	{
		ss << "0";
	}
	ss << seconds << " : ";

	if (centiSecond < 10)
	{
		ss << "0";
	}
	ss << centiSecond << endl;
	time.setString(ss.str());
	
	ss.str("");
	ss << owner->numKeysCollected << " / " << owner->numTotalKeys;
	keys.setString(ss.str());

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
	for (int i = 0; i < NUM_BARS+1; ++i)
	{
		bars[i]->state = ScoreBar::RETRACT;
		bars[i]->frame = 0;
		//bars[i]->xDiffPos = 0;
	}
	waiting = false;

	//active = false;
	//waiting = false;
	//Reset();
}

ScoreBar::ScoreBar(int p_row, ScoreDisplay *p_parent, bool p_contBar )
	:parent(p_parent), frame(0), state(NONE), row(p_row), xDiffPos(0), contBar( p_contBar )
{

}

void ScoreBar::SetBarPos(float xDiff)
{
	int f = 0;
	if (contBar)
		f = 1;
	IntRect ir = parent->ts_scoreBar->GetSubRect(f);
	int rowHeight = 100;
	xDiffPos = xDiff;

	VertexArray &scoreBarVA = parent->scoreBarVA;
	VertexArray &scoreSheetVA = parent->scoreSheetVA;
	VertexArray &scoreSymbolsVA = parent->scoreSymbolsVA;
	Vector2f &basePos = parent->basePos;

	scoreBarVA[row * 4 + 0].position = Vector2f(basePos.x + xDiff, basePos.y + row * rowHeight);
	scoreBarVA[row * 4 + 1].position = Vector2f(basePos.x + xDiff + ir.width, basePos.y + row * rowHeight);
	scoreBarVA[row * 4 + 2].position = Vector2f(basePos.x + xDiff + ir.width, basePos.y + row * rowHeight + ir.height);
	scoreBarVA[row * 4 + 3].position = Vector2f(basePos.x + xDiff, basePos.y + row * rowHeight + ir.height);

	if (row < ScoreDisplay::NUM_BARS)
	{
		scoreSheetVA[row * 4 + 0].position = Vector2f(basePos.x + xDiff, basePos.y + row * rowHeight);
		scoreSheetVA[row * 4 + 1].position = Vector2f(basePos.x + xDiff + ir.width, basePos.y + row * rowHeight);
		scoreSheetVA[row * 4 + 2].position = Vector2f(basePos.x + xDiff + ir.width, basePos.y + row * rowHeight + ir.height);
		scoreSheetVA[row * 4 + 3].position = Vector2f(basePos.x + xDiff, basePos.y + row * rowHeight + ir.height);

		scoreSymbolsVA[row * 4 + 0].position = Vector2f(basePos.x + xDiff, basePos.y + row * rowHeight);
		scoreSymbolsVA[row * 4 + 1].position = Vector2f(basePos.x + xDiff + ir.width, basePos.y + row * rowHeight);
		scoreSymbolsVA[row * 4 + 2].position = Vector2f(basePos.x + xDiff + ir.width, basePos.y + row * rowHeight + ir.height);
		scoreSymbolsVA[row * 4 + 3].position = Vector2f(basePos.x + xDiff, basePos.y + row * rowHeight + ir.height);
	}
}

void ScoreBar::ClearSheet()
{
	/*scoreSheetVA[ row * 4 + 0 ].texCoords = Vector2f( ir.left, ir.top );
	scoreSheetVA[ row * 4 + 1 ].texCoords = Vector2f( ir.left + ir.width, ir.top );
	scoreSheetVA[ row * 4 + 2 ].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	scoreSheetVA[ row * 4 + 3 ].texCoords = Vector2f( ir.left, ir.top + ir.height );*/
}

void ScoreBar::SetSheetFrame(int frame)
{
	VertexArray &scoreSheetVA = parent->scoreSheetVA;
	Tileset *ts_scoreSheet = parent->ts_scoreSheet;
	IntRect ir = ts_scoreSheet->GetSubRect(frame);
	scoreSheetVA[row * 4 + 0].texCoords = Vector2f(ir.left, ir.top);
	scoreSheetVA[row * 4 + 1].texCoords = Vector2f(ir.left + ir.width, ir.top);
	scoreSheetVA[row * 4 + 2].texCoords = Vector2f(ir.left + ir.width, ir.top + ir.height);
	scoreSheetVA[row * 4 + 3].texCoords = Vector2f(ir.left, ir.top + ir.height);
}

void ScoreBar::SetSymbolTransparency(float f)
{
	int n = floor(f * 255.0 + .5);
	VertexArray &scoreSymbolsVA = parent->scoreSymbolsVA;
	scoreSymbolsVA[row * 4 + 0].color = Color(255, 255, 255, n);
	scoreSymbolsVA[row * 4 + 1].color = Color(255, 255, 255, n);
	scoreSymbolsVA[row * 4 + 2].color = Color(255, 255, 255, n);
	scoreSymbolsVA[row * 4 + 3].color = Color(255, 255, 255, n);

}

void ScoreBar::Update()
{
	switch (state)
	{
	case NONE:
	{
		break;
	}
	case POP_OUT:
	{
		int popoutFrames = 30;
		if (frame == popoutFrames + 1)
		{
			if (contBar)
			{
				state = SHEET_DISPLAY;
			}
			else
			{
				state = SHEET_APPEAR;
			}
			
			frame = 0;
			break;
		}
		else if (frame == popoutFrames)
		{
			if (row < parent->NUM_BARS)
			{
				parent->bars[row + 1]->state = ScoreBar::POP_OUT;
				parent->bars[row + 1]->frame = 0;
			}
		}
		++frame;

		break;
	}
	case SHEET_APPEAR:
	{
		if (frame == 10)
		{
			state = SYMBOL_DISPLAY;
			frame = 0;

			if (row == 0)
			{
				int rowHeight = 100;
				Vector2f basePos = parent->basePos + Vector2f(0, rowHeight * row)
					+ Vector2f(32, 20) + Vector2f(-parent->ts_scoreBar->tileWidth + 80, 0);
				parent->time.setPosition(basePos);
			}
			else if (row == 1)
			{
				int rowHeight = 100;
				Vector2f basePos = parent->basePos + Vector2f(0, rowHeight * row)
					+ Vector2f(32, 20) + Vector2f(-parent->ts_scoreBar->tileWidth + 80, 0);
				parent->keys.setPosition(basePos);
				
			}
			break;
		}

		++frame;
		break;
	}
	case SYMBOL_DISPLAY:
	{
		if (frame == 31)
		{
			state = SHEET_DISPLAY;
			frame = 0;
			break;
		}
		++frame;
		break;
	}
	case SHEET_DISPLAY:
	{
		break;
	}
	case RETRACT:
	{
		if (frame == 30)
		{
			state = NONE;
			frame = 0;
			//active = false;
		}

		++frame;
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
		int popoutFrames = 30;
		CubicBezier bez(0, 0, 1, 1);
		float z = bez.GetValue((double)frame / popoutFrames);

		SetBarPos(-384.f * z);

		break;
	}
	case SHEET_APPEAR:
	{
		if( !contBar )
			SetSheetFrame(frame);
		break;
	}
	case SYMBOL_DISPLAY:
	{
		if (!contBar)
		{


			int dispFrames = 30;
			CubicBezier bez(0, 0, 1, 1);
			float z = bez.GetValue((double)frame / dispFrames);
			SetSymbolTransparency(z);

			if (row == 0)
			{

			}
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
}