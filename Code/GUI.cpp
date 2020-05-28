#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

const int CHECKBOXSIZE = 32;

ToolTip::ToolTip(const std::string &str)
{
	EditSession *edit = EditSession::GetSession();

	toolTipText.setCharacterSize(14);
	toolTipText.setFont(edit->mainMenu->arial);
	toolTipText.setFillColor(Color::Black);
	toolTipText.setString(str);

	SetRectColor( quad, Color::White);
}

void ToolTip::SetFromMousePos(const sf::Vector2i &pos)
{
	Vector2f myPos = Vector2f(pos.x + 20, pos.y + 20);
	float width = toolTipText.getLocalBounds().width;
	float height = toolTipText.getLocalBounds().height;
	float extra = 10;
	SetRectTopLeft(quad, width + extra * 2, height + extra * 2, myPos + Vector2f( -extra, -extra ));
	toolTipText.setPosition(myPos);
}

void ToolTip::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
	target->draw(toolTipText);
}

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

void PanelMember::SetToolTip(const std::string &str)
{
	assert(toolTip == NULL);
	toolTip = new ToolTip(str);
}

PanelMember::~PanelMember()
{
	if (toolTip != NULL)
		delete toolTip;
}

void PanelMember::UpdateToolTip(bool contains)
{
	if (contains)
	{
		panel->SetFocusedMember(this);
	}
	else
	{
		panel->RemoveAsFocusedMember(this);
	}
}

ChooseRect::ChooseRect(ChooseRectIdentity ident, ChooseRectType crType,
	Vertex *v, float size, sf::Vector2f &p_pos, Panel *p)
	:PanelMember(p), quad(v), boxSize(size), pos(p_pos), chooseRectType(crType),
	rectIdentity(ident), circleMode(false)
{
	idleColor = Color::Black;
	idleColor.a = 100;

	mouseOverColor = Color::Green;
	mouseOverColor.a = 100;

	SetShown(false);

	focused = false;

	EditSession *edit = EditSession::GetSession();
	nameText.setFont(edit->mainMenu->arial);
	nameText.setCharacterSize(18);
	nameText.setFillColor(Color::White);
	nameText.setOutlineColor(Color::Black);
	nameText.setOutlineThickness(3);
	nameText.setPosition(Vector2f(pos.x + boxSize / 2, pos.y));
}

void ChooseRect::SetCircleMode(int p_radius)
{
	circleMode = true;
	circleRadius = p_radius;
}

void ChooseRect::SetRectMode()
{
	circleMode = false;
}



void ChooseRect::Init()
{
	SetRectColor(quad, idleColor);
	SetSize(boxSize);
}

void ChooseRect::SetPosition(sf::Vector2f &p_pos)
{
	pos = p_pos;
	UpdateRectDimensions();
	nameText.setPosition(Vector2f(pos.x + boxSize / 2, pos.y));
}

void ChooseRect::SetSize(float s)
{
	boxSize = s;
	UpdateRectDimensions();
}

void ChooseRect::UpdateRectDimensions()
{
	//SetRectCenter(quad, boxSize, boxSize, pos);
	bounds.left = pos.x;// -boxSize / 2.f;
	bounds.top = pos.y;// -boxSize / 2.f;
	bounds.width = boxSize;
	bounds.height = boxSize;
}

void ChooseRect::SetShown(bool s)
{
	if (!s && show)
	{
		SetRectTopLeft(quad, 0, 0, Vector2f(0, 0));
	}
	else if (s && !show)
	{
		if (circleMode)
		{
			SetRectTopLeft(quad, 0, 0, Vector2f(0, 0));
		}
		else
		{
			SetRectTopLeft(quad, boxSize, boxSize, pos);
		}
		SetSize(boxSize);
	}
	show = s;
}

sf::Vector2f ChooseRect::GetGlobalPos()
{
	//return mouseUser->GetFloatPos() + pos;
	return Vector2f(panel->pos) + pos;
}

sf::Vector2f ChooseRect::GetGlobalCenterPos()
{
	return GetGlobalPos() + Vector2f(boxSize / 2, boxSize / 2);
}

void ChooseRect::SetActive(bool a)
{
	active = a;
}

void ChooseRect::Deactivate()
{
	focused = false;
	SetRectColor(quad, idleColor);
}

bool ChooseRect::ContainsPoint(sf::Vector2i &mousePos)
{
	if (circleMode)
	{
		Vector2i center(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
		Vector2f diff(mousePos - center);
		return (length(diff) <= circleRadius);
	}
	else
	{
		return bounds.contains(mousePos);
	}
}

bool ChooseRect::MouseUpdate()
{
	if (!show)
	{
		return false;
	}

	Vector2i mousePos = panel->GetMousePos();

	if (MOUSE.IsMouseLeftClicked())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_LEFTCLICKED);
			focused = true;
		}
	}
	else if (MOUSE.IsMouseLeftReleased())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_LEFTRELEASED);
			focused = true;
		}
	}
	//else if (!MOUSE.IsMouseDownLeft())
	{
		if (ContainsPoint(mousePos))
		{
			SetRectColor(quad, mouseOverColor);
			if (!focused)
			{
				panel->handler->ChooseRectEvent(this, E_FOCUSED);
			}
			focused = true;
		}
		else
		{
			SetRectColor(quad, idleColor);
			if (focused)
			{
				panel->handler->ChooseRectEvent(this, E_UNFOCUSED);
			}
			focused = false;

			//Unfocus();
		}
	}

	if (MOUSE.IsMouseRightClicked())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_RIGHTCLICKED);
			//focused = true;
		}
	}
	else if (MOUSE.IsMouseRightReleased())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_RIGHTRELEASED);
			//focused = true;
		}
	}

	return true;
}

EnemyChooseRect *ChooseRect::GetAsEnemyChooseRect()
{
	if (chooseRectType == ENEMY)
	{
		return (EnemyChooseRect*)this;
	}
	else
	{
		return NULL;
	}
}

ImageChooseRect *ChooseRect::GetAsImageChooseRect()
{
	if (chooseRectType == IMAGE)
	{
		return (ImageChooseRect*)this;
	}
	else
	{
		return NULL;
	}
}

EnemyChooseRect::EnemyChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, ActorType * p_type, int p_level,
	Panel *p)
	:ChooseRect(ident, ChooseRectType::ENEMY, v, 100, p_pos, p), level(p_level)
{
	actorType = NULL;
	SetType(p_type, level);
}

//void EnemyChooseRect::Unfocus()
//{
//	if (enemy != NULL)
//	{
//		enemy->SetActionEditLoop();
//		enemy->UpdateFromEditParams(0);
//	}
//}

void EnemyChooseRect::SetType(ActorType *type, int lev)
{
	if (type == NULL)
	{
		SetShown(false);
		actorType = type;
		level = lev;
		return;
	}

	if (!(type == actorType && lev == level))
	{
		actorType = type;
		level = lev;
		enemyParams = actorType->defaultParamsVec[level - 1];
		enemyParams->MoveTo(Vector2i(0, 0));
		enemy = enemyParams->myEnemy;
		if (enemy != NULL)
		{
			enemy->SetActionEditLoop();
			enemy->UpdateFromEditParams(0);
			nameText.setString(enemy->name);
			nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
				0);
		}

		SetSize(boxSize);

		switch (level)
		{
		case 1:
			idleColor = Color::Blue;
			break;
		case 2:
			idleColor = Color::Cyan;
			break;
		case 3:
			idleColor = Color::Magenta;
			break;
		case 4:
			idleColor = Color::Red;
			break;

		}
		idleColor.a = 100;
		SetRectColor(quad, idleColor);

	}
}

void EnemyChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);
	UpdatePanelPos();
}

void EnemyChooseRect::UpdatePanelPos()
{
	if (actorType != NULL)
	{
		Vector2f truePos = GetGlobalPos() + Vector2f(boxSize / 2.f, boxSize / 2.f);

		float test;
		FloatRect aabb = enemyParams->GetAABB();

		float max = std::max(aabb.height, aabb.width);
		//max *= 1.1f;
		test = max / boxSize;
		view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
		view.setSize(Vector2f(1920 * test, 1080 * test));
	}
}

void EnemyChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		enemyParams->DrawEnemy(target);

		target->setView(oldView);

		target->draw(nameText);
	}
}

void EnemyChooseRect::UpdateSprite(int frameUpdate)
{
	if (!show)
	{
		return;
	}

	if (actorType != NULL)
	{
		if (enemy != NULL)
		{
			if (focused)
			{
				enemy->UpdateFromEditParams(frameUpdate);
			}
			else
			{
				enemy->SetActionEditLoop();
				enemy->UpdateFromEditParams(0);
			}
		}
	}
}

EditorDecorInfo * ImageChooseRect::CreateDecor()
{
	EditorDecorInfo *edi = new EditorDecorInfo(decorName, ts, tileIndex, 0, Vector2f(0, 0),
		0, Vector2f(1, 1));
	return edi;
}

void ImageChooseRect::SetImage(Tileset *p_ts, const sf::IntRect &subRect)
{
	if (p_ts == NULL)
	{
		SetShown(false);
		ts = p_ts;
		tileIndex = -1;
		return;
	}

	ts = p_ts;
	tileIndex = -1;

	ts->SetSpriteTexture(spr);
	spr.setTextureRect(subRect);
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

	SetSize(boxSize);
}

void ImageChooseRect::SetImage(Tileset *p_ts, int p_index)
{
	if (p_ts == NULL)
	{
		SetShown(false);
		ts = p_ts;
		tileIndex = p_index;
		return;
	}

	if (!(p_ts == ts && p_index == tileIndex))
	{
		ts = p_ts;
		tileIndex = p_index;


		ts->SetSpriteTexture(spr);
		ts->SetSubRect(spr, tileIndex);
		spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

		SetSize(boxSize);
	}
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, Tileset *p_ts,
	int p_tileIndex, int bSize, Panel *p)
	:ChooseRect(ident, ChooseRectType::IMAGE, v, bSize, p_pos, p)
{
	ts = NULL;
	SetImage(p_ts, p_tileIndex);
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v,
	sf::Vector2f &p_pos, Tileset *p_ts, const sf::IntRect &subRect, int bSize, Panel *p)
	:ChooseRect(ident, ChooseRectType::IMAGE, v, bSize, p_pos, p)
{
	ts = NULL;
	SetImage(p_ts, subRect);
}

void ImageChooseRect::UpdatePanelPos()
{
	Vector2f truePos = GetGlobalPos() + Vector2f(boxSize / 2.f, boxSize / 2.f);

	float test;
	FloatRect aabb = spr.getGlobalBounds();
	float max = std::max(aabb.height, aabb.width) * 1.2f; //.8 to give the box a little room
	test = max / boxSize;
	view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	view.setSize(Vector2f(1920 * test, 1080 * test));
}

void ImageChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);
	UpdatePanelPos();	
}

void ImageChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		target->draw(spr);

		target->setView(oldView);
	}
}

void ImageChooseRect::UpdateSprite(int frameUpdate)
{
	//animate eventually
}

GridSelector::GridSelector( Vector2i p_pos, int xSizep, int ySizep, int iconX, int iconY, bool p_displaySelected,
						   bool p_displayMouseOver, Panel *p )
	:PanelMember( p ), xSize( xSizep ), ySize( ySizep ), tileSizeX( iconX ), tileSizeY( iconY ), active( true )
{
	displaySelected = p_displaySelected;
	displayMouseOver = p_displayMouseOver;
	icons = new Sprite *[xSize];
	names = new string *[xSize];
	for( int i = 0; i < xSize; ++i )
	{
		icons[i] = new Sprite[ySize];
		names[i] = new string[ySize];
		for( int j = 0; j < ySize; ++j )
		{
			icons[i][j].setTextureRect( sf::IntRect( 0, 0, tileSizeX, tileSizeY ) );
			//icons[i][j].setPosition( i * tileSizeX, j * tileSizeY );
			names[i][j] = "not set";
		}
	}

	pos.x = p_pos.x;
	pos.y = p_pos.y;
	focusX = -1;
	focusY = -1;
	//selectedX = -1;
	//selectedY = -1;
	selectedX = 0;
	selectedY = 0;
	mouseOverX = -1;
	mouseOverY = -1;
}

GridSelector::~GridSelector()
{
	for (int i = 0; i < xSize; ++i)
	{
		delete[] icons[i];
		delete[] names[i];
	}
	delete[] icons;
	delete[] names;
}

void GridSelector::Deactivate()
{
	focusX = -1;
	focusY = -1;
	mouseOverX = -1;
	mouseOverY = -1;
	selectedX = -1;
	selectedY = -1;
}

void GridSelector::Set( int xi, int yi, Sprite s, const std::string &name )
{
	icons[xi][yi] = s;
	icons[xi][yi].setPosition( xi * tileSizeX, yi * tileSizeY );
	names[xi][yi] = name;
}

void GridSelector::Draw( sf::RenderTarget *target )
{
	if( active )
	{
		sf::RectangleShape rs;
		rs.setSize( Vector2f( xSize * tileSizeX, ySize * tileSizeY ) );
		rs.setFillColor( Color::Yellow );
		Vector2i truePos( pos.x, pos.y );
		rs.setPosition( truePos.x, truePos.y );

		target->draw( rs );

		for( int x = 0; x < xSize; ++x )
		{
			for( int y = 0; y < ySize; ++y )
			{
				Sprite &s = icons[x][y];
				Vector2f realPos = s.getPosition();
				s.setPosition( Vector2f( realPos.x + truePos.x, realPos.y + truePos.y ) );

				target->draw( s );
				//s.setColor( Color::White );
				s.setPosition( realPos );
			}
		}

		if( displaySelected )//selectedX >= 0 && selectedY >= 0 )
		{
			Sprite &s = icons[selectedX][selectedY];
			Vector2f rectPos = s.getPosition() + Vector2f( truePos.x, truePos.y );
			//s.setPosition( Vector2f( realPos.x + truePos.x, realPos.y + truePos.y ) );
			sf::RectangleShape re;
			re.setFillColor( Color::Transparent );
			re.setOutlineColor( Color::Green );
			re.setOutlineThickness( 3 );
			re.setPosition( rectPos.x, rectPos.y );
			re.setSize( Vector2f( tileSizeX, tileSizeY ) );
			target->draw( re );
		}

	}
}

//returns true if a selection has been made
bool GridSelector::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	//cout << "update: " << posx << ", " << posy << endl;
	if( !active )
	{
		return false;
		//assert( false && "trying to update inactive grid selector" );
	}
	if( MOUSE.IsMouseLeftClicked() )
	{
		sf::Rect<int> r( pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY );
		if( r.contains(mousePos) )
		{
			focusX = (mousePos.x - pos.x ) / tileSizeX;
			focusY = (mousePos.y - pos.y ) / tileSizeY;
			cout << "contains index: " << focusX << ", " << focusY << endl;
		}
		else
		{
			focusX = -1;
			focusY = -1;
		}
	}
	else
	{
		sf::Rect<int> r( pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY );
		if( r.contains(mousePos) )
		{
			int tempX = (mousePos.x - pos.x ) / tileSizeX;
			int tempY = (mousePos.y - pos.y ) / tileSizeY;
			if( tempX == focusX && tempY == focusY )
			{
				selectedX = tempX;
				selectedY = tempY;
				cout << "tempX: " << tempX << ", tempY: " << tempY << endl;
				panel->SendEvent( this, names[tempX][tempY] );//->GridSelectorCallback( this, names[tempX][tempY] );
				return true;
		//		cout << "success!" << endl;
			}
			else
			{
				focusX = -1;
				focusY = -1;
			}
			//cout << "contains index: " << posx / tileSizeX << ", " << posy / tileSizeY << endl;		
			
		}
		else
		{
		//	cout << "doesn't contain!" << endl;
		//	cout << "pos: " << posx << ", " << posy << endl;
			focusX = -1;
			focusY = -1;
		}
	}

	return false;
}

Slider::Slider(const std::string &n, sf::Vector2i &p_pos, int width, sf::Font &f,
	int p_min, int p_max, int p_defaultNum, Panel *p)
	:PanelMember( p ), pos(p_pos), clickedDown(false), characterHeight(20), name(n),
	myFont(f), minValue( p_min ), maxValue( p_max ), defaultValue(p_defaultNum)
{
	size.y = 20;
	size.x = width;
	circleRad = 13;
	currValue = defaultValue;

	displayText.setFont(f);
	displayText.setCharacterSize(characterHeight);
	displayText.setFillColor(Color::White);

	SetRectColor(displayRect, Color(Color::Black));

	SetRectTopLeft(mainRect, size.x, size.y, Vector2f(pos));
	SetRectColor(mainRect, Color(Color::Black));

	selectCircle.setFillColor(Color::Red);
	selectCircle.setRadius(circleRad);
	selectCircle.setOrigin(selectCircle.getLocalBounds().width / 2,
		selectCircle.getLocalBounds().height / 2);

	float totalDiff = maxValue - minValue;
	float factor = defaultValue / totalDiff;
	SetToFactor(factor);
}

void Slider::Deactivate()
{
	clickedDown = false;
}

void Slider::SetCircle(int x)
{
	selectCircle.setPosition(Vector2f(x, pos.y + size.y / 2));
}

void Slider::SetToFactor(float factor)
{
	int currValue = GetCurrValue(factor);
	displayText.setString(to_string(currValue));
	int currX = GetCurrX(factor);
	auto textBounds = displayText.getLocalBounds();
	displayText.setOrigin(textBounds.left + textBounds.width / 2,
		textBounds.top + textBounds.height / 2);
	displayText.setPosition(Vector2f(currX, pos.y - textBounds.height / 2 - 10));
	SetRectCenter(displayRect, textBounds.width + 10, textBounds.height + 10, Vector2f(displayText.getPosition()));
	SetCircle(currX);
}

bool Slider::IsPointOnRect(sf::Vector2f &point)
{
	return QuadContainsPoint(mainRect, point);
}

float Slider::GetCurrFactor(const sf::Vector2i &mousePos)
{
	Vector2i adjustedPos = mousePos - pos;
	if (adjustedPos.x >= size.x)
	{
		return 1.f;
	}
	else if (adjustedPos.x <= 0)
	{
		return 0.f;
	}
	else
	{
		float len = size.x;
		float curr = adjustedPos.x;

		float factor = curr / len;
		return factor;

		/*float totalDiff = maxValue - minValue;
		float fValue = minValue + (totalDiff * factor);
		int value = fValue;
		return value;*/
	}
}

int Slider::GetCurrValue(float factor)
{
	float totalDiff = maxValue - minValue;
	float fValue = minValue + (totalDiff * factor);
	int value = fValue;
	return value;
}

int Slider::GetCurrX(float factor)
{
	float currXF = factor * size.x + pos.x;
	int currX = currXF;
	return currX;
}


bool Slider::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);
	bool mouseDown = MOUSE.IsMouseDownLeft();
	if ( mouseDown && ( IsPointOnRect( point ) || clickedDown ))
	{
		clickedDown = true;
		float currFactor = GetCurrFactor(mousePos);
		SetToFactor(currFactor);
	}
	else if (!mouseDown)
	{
		clickedDown = false;
	}

	return true;
}

void Slider::Draw(sf::RenderTarget *target)
{
	target->draw(mainRect, 4, sf::Quads);
	target->draw(underRect, 4, sf::Quads);
	target->draw(selectCircle);
	target->draw(displayRect, 4, sf::Quads);
	target->draw(displayText);
}

Dropdown::Dropdown(const std::string &n, sf::Vector2i &p_pos,
	sf::Vector2i &p_size, sf::Font &f,
	const std::vector<std::string> &p_options, int p_defaultIndex, Panel *p)
	:PanelMember( p ), pos(p_pos), clickedDown(false), characterHeight(size.y- 4), size(p_size), name(n),
	myFont( f ), defaultIndex( p_defaultIndex ), expanded(false)
{
	selectedIndex = -1;
	SetOptions(p_options);
}

Dropdown::~Dropdown()
{
	delete[] dropdownRects;
}

void Dropdown::Deactivate()
{
	expanded = false;
	clickedDown = false;
}

void Dropdown::SetOptions(const std::vector<std::string> &p_options)
{
	options = p_options;

	numOptions = options.size();
	optionText.resize(numOptions);

	SetRectTopLeft(mainRect, size.x, size.y, Vector2f(pos));
	SetRectColor(mainRect, Color(Color::Black));

	SetSelectedIndex(defaultIndex);
	selectedText.setFont(myFont);
	selectedText.setFillColor(Color::White);
	selectedText.setCharacterSize(characterHeight);
	auto lb = selectedText.getLocalBounds();
	selectedText.setOrigin(lb.left, lb.top);
	selectedText.setPosition(Vector2f(pos) + Vector2f( 4,4 ));
	
	dropdownRects = new Vertex[4 * numOptions];

	for ( int i = 0; i < numOptions; ++i )
	{
		Vector2f dropPos(pos.x, pos.y + size.y * (i + 1));
		SetRectTopLeft(dropdownRects + i * 4, size.x, size.y, dropPos);
		SetRectColor(dropdownRects + i * 4, Color(Color::Blue));

		Text &t = optionText[i];
		t.setString(options[i]);
		t.setFont(myFont);
		t.setFillColor(Color::White);
		t.setCharacterSize(characterHeight);
		auto tlb = t.getLocalBounds();
		t.setOrigin(tlb.left, tlb.top);
		t.setPosition(dropPos + Vector2f(4, 4));
	}
}

void Dropdown::Draw(sf::RenderTarget *target)
{
	

	target->draw(mainRect, 4, sf::Quads);
	target->draw(selectedText);

	if (expanded)
	{
		target->draw(dropdownRects, numOptions * 4, sf::Quads);
		for (int i = 0; i < numOptions; ++i)
		{
			target->draw(optionText[i]);
		}
	}

	/*sf::RectangleShape rs;
	rs.setSize(size);
	rs.setPosition(pos.x, pos.y);
	if (clickedDown)
		rs.setFillColor(Color::Green);
	else
		rs.setFillColor(Color::Blue);

	target->draw(rs);

	target->draw(text);*/
}

void Dropdown::SetSelectedIndex(int ind)
{
	if (ind != selectedIndex)
	{
		selectedIndex = ind;
		selectedText.setString(options[selectedIndex]);
	}
}

bool Dropdown::IsMouseOnOption(int ind, Vector2f &point )
{
	return QuadContainsPoint(dropdownRects + ind * 4, point );
}

bool Dropdown::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);

	bool onMainQuad = QuadContainsPoint(mainRect, point);

	
	if (!expanded && onMainQuad)
	{
		panel->SetFocusedMember(this);
	}
	else if (expanded || !onMainQuad )//|| (!expanded && !onMainQuad) )
	{
		if (panel->focusedMember == this)
		{
			panel->focusedMember = NULL;
			panel->HideToolTip();
		}
	}

	if (expanded)
	{
		int highlightedIndex = -1;
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				highlightedIndex = i;
				break;
			}
		}

		if (highlightedIndex < 0)
		{
			highlightedIndex = selectedIndex;
		}

		for (int i = 0; i < numOptions; ++i)
		{
			if (i == highlightedIndex)
			{
				SetRectColor(dropdownRects + i * 4, Color(Color::Red));
			}
			else
			{
				SetRectColor(dropdownRects + i * 4, Color(Color::Blue));
			}
		}
	}
	

	

	if (MOUSE.IsMouseLeftReleased() && expanded )
	{
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point ))
			{
				SetSelectedIndex(i);
				expanded = false;
				clickedDown = false;
				panel->SendEvent(this, "selected");
				return true;
			}
		}

		if (!onMainQuad)
		{
			expanded = false;
			clickedDown = false;
		}

		return false;
	}

	if (MOUSE.IsMouseLeftClicked())
	{
		
		if (expanded)
		{
			for (int i = 0; i < numOptions; ++i)
			{
				if (IsMouseOnOption(i, point))
				{
					SetSelectedIndex(i);
					expanded = false;
					clickedDown = true;
					panel->SendEvent(this, "selected");
					return true;
				}
			}

			clickedDown = true;
			expanded = false;
			return false;
		}
		else
		{
			if (onMainQuad)
			{
				clickedDown = true;
				expanded = true;
				return true;
			}
		}
	}
	else
	{
		clickedDown = false;
	}
	return false;
}

MenuDropdown::MenuDropdown(const std::string &n, sf::Vector2i &p_pos,
	sf::Vector2i &p_size, int p_optionWidth, sf::Font &f,
	const std::vector<std::string> &p_options, Panel *p)
	:PanelMember(p), pos(p_pos), clickedDown(false), characterHeight(size.y - 4), size(p_size), name(n),
	myFont(f), expanded(false), optionWidth( p_optionWidth )
{
	selectedIndex = -1;
	SetOptions(p_options);

	idleColor = Color::Black;
	mouseOverColor = Color(100, 100, 100);
	expandedColor = Color(150, 150, 150);
}

MenuDropdown::~MenuDropdown()
{
	delete[] dropdownRects;
}

void MenuDropdown::Deactivate()
{
	SetExpanded(false, false);
	clickedDown = false;
}

void MenuDropdown::SetExpanded(bool exp, bool onMain)
{
	expanded = exp;
	if (expanded)
	{
		SetRectColor(mainRect, expandedColor);
	}
	else
	{
		if (onMain)
		{
			SetRectColor(mainRect, mouseOverColor);
		}
		else
		{
			SetRectColor(mainRect, idleColor);
		}
		
	}
}

void MenuDropdown::SetOptions(const std::vector<std::string> &p_options)
{
	options = p_options;

	numOptions = options.size();
	optionText.resize(numOptions);

	SetRectTopLeft(mainRect, size.x, size.y, Vector2f(pos));
	SetRectColor(mainRect, idleColor);

	menuText.setString(name);
	menuText.setFont(myFont);
	menuText.setFillColor(Color::White);
	menuText.setCharacterSize(characterHeight);
	auto lb = menuText.getLocalBounds();
	menuText.setOrigin(lb.left, lb.top);
	menuText.setPosition(Vector2f(pos) + Vector2f(4, 4));

	dropdownRects = new Vertex[4 * numOptions];

	for (int i = 0; i < numOptions; ++i)
	{
		Vector2f dropPos(pos.x, pos.y + size.y * (i + 1));
		SetRectTopLeft(dropdownRects + i * 4, optionWidth, size.y, dropPos);
		SetRectColor(dropdownRects + i * 4, Color(Color::Blue));

		Text &t = optionText[i];
		t.setString(options[i]);
		t.setFont(myFont);
		t.setFillColor(Color::White);
		t.setCharacterSize(characterHeight);
		auto tlb = t.getLocalBounds();
		t.setOrigin(tlb.left, tlb.top);
		t.setPosition(dropPos + Vector2f(4, 4));
	}
}

void MenuDropdown::Draw(sf::RenderTarget *target)
{
	target->draw(mainRect, 4, sf::Quads);
	target->draw(menuText);

	if (expanded)
	{
		target->draw(dropdownRects, numOptions * 4, sf::Quads);
		for (int i = 0; i < numOptions; ++i)
		{
			target->draw(optionText[i]);
		}
	}
}

void MenuDropdown::SetSelectedIndex(int ind)
{
	if (ind != selectedIndex)
	{
		selectedIndex = ind;
	}
}

bool MenuDropdown::IsMouseOnOption(int ind, Vector2f &point)
{
	return QuadContainsPoint(dropdownRects + ind * 4, point);
}

bool MenuDropdown::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);

	bool onMainQuad = QuadContainsPoint(mainRect, point);
	if (onMainQuad)
	{
		panel->SetFocusedMember(this);
	}
	//UpdateToolTip(onMainQuad);

	if (expanded)
	{
		int highlightedIndex = -1;
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				highlightedIndex = i;
				break;
			}
		}

		for (int i = 0; i < numOptions; ++i)
		{
			if (i == highlightedIndex)
			{
				SetRectColor(dropdownRects + i * 4, Color(Color::Red));
			}
			else
			{
				SetRectColor(dropdownRects + i * 4, Color(Color::Blue));
			}
		}
	}

	

	if (!expanded)
	{
		if (onMainQuad)
		{
			SetRectColor(mainRect, mouseOverColor);
		}
		else
		{
			SetRectColor(mainRect, idleColor);
		}
	}


	if (MOUSE.IsMouseLeftReleased() && expanded)
	{
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				SetSelectedIndex(i);
				SetExpanded(false, onMainQuad);
				clickedDown = false;
				panel->SendEvent(this, "selected");
				return true;
			}
		}

		if (!onMainQuad)
		{
			SetExpanded(false, onMainQuad);
			clickedDown = false;
		}

		return false;
	}

	if (MOUSE.IsMouseLeftClicked())
	{
		if (expanded)
		{
			for (int i = 0; i < numOptions; ++i)
			{
				if (IsMouseOnOption(i, point))
				{
					SetSelectedIndex(i);
					SetExpanded(false, onMainQuad);
					clickedDown = true;
					panel->SendEvent(this, "selected");
					return true;
				}
			}

			clickedDown = true;
			SetExpanded(false, onMainQuad);
			return false;
		}
		else
		{
			if (onMainQuad)
			{
				clickedDown = true;
				SetExpanded(true, onMainQuad);
				return true;
			}
		}
	}
	else
	{
		clickedDown = false;
	}
	return false;
}


Panel::Panel( const string &n, int width, int height, GUIHandler *h, bool pop )
	:handler( h ), size( width, height ), name( n ), popup( pop )
	//:t( 0, 0, 200, 10, f, "hello" ), t2( 0, 100, 100, 10, f, "blah" ), b( 0, 50, 100, 50, f, "button!" )
{
	imageChooseRectQuads = NULL;
	enemyChooseRectQuads = NULL;
	reservedEnemyRectCount = 0;
	reservedImageRectCount = 0;
	arial.loadFromFile("Resources/Fonts/Breakneck_Font_01.ttf");
	SetPosition(Vector2i(0, 0));
	extraUpdater = NULL;

	Color defaultColor(83, 102, 188);
	SetRectColor(quad, defaultColor);
	SetRectTopLeft(quad, size.x, size.y, Vector2f( 0, 0 ));

	slideFrame = 0;
	slideDuration = -1;
	
	currToolTip = NULL;
	toolTipThresh = 30;
	toolTipCounter = 0;

	focusedMember = NULL;
	confirmButton = NULL;

	autoSpace.x = false;
	autoSpace.y = false;
}

Panel::~Panel()
{
	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = sliders.begin(); it != sliders.end(); ++it)
	{
		delete (*it).second;
	}
	for (auto it = textBoxes.begin(); it != textBoxes.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = buttons.begin(); it != buttons.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = labels.begin(); it != labels.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = checkBoxes.begin(); it != checkBoxes.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = gridSelectors.begin(); it != gridSelectors.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = enemyChooseRects.begin(); it != enemyChooseRects.end(); ++it)
	{
		delete (*it);
	}
	if (enemyChooseRectQuads != NULL)
	{
		delete[] enemyChooseRectQuads;
	}
	

	for (auto it = imageChooseRects.begin(); it != imageChooseRects.end(); ++it)
	{
		delete (*it);
	}
	if (imageChooseRectQuads != NULL)
	{
		delete[] imageChooseRectQuads;
	}
	

	/*for (auto it = enemyChoosers.begin(); it != enemyChoosers.end(); ++it)
	{
		delete (*it).second;
	}*/
}

void Panel::SetAutoSpacing(bool aSpaceX, bool aSpaceY, Vector2i start )
{
	autoSpace.x = aSpaceX;
	autoSpace.y = aSpaceY;
	autoStart = start;
}

void Panel::SetConfirmButton(Button *b)
{
	confirmButton = b;
}

void Panel::SetFocusedMember(PanelMember*pm)
{
	if (pm != focusedMember)
	{
		HideToolTip();
		if( focusedMember != NULL )
			focusedMember->Deactivate();
		focusedMember = pm;
	}
}
void Panel::RemoveAsFocusedMember(PanelMember *pm)
{
	if (pm == focusedMember)
	{
		focusedMember->Deactivate();
		focusedMember = NULL;
		HideToolTip();
	}
}

bool Panel::ToolTipCanBeTurnedOn()
{
	return currToolTip == NULL && toolTipCounter == toolTipThresh;
}

void Panel::UpdateToolTip( int frames )
{
	bool down = MOUSE.IsMouseDownLeft() || MOUSE.IsMouseDownRight();
	if (down)
	{
		HideToolTip();
	}
	else
	{
		if (toolTipCounter == 0)
		{
			lastMouse = GetMousePos();
			toolTipCounter += frames;
		}
		else
		{
			if (toolTipCounter < toolTipThresh)
			{
				toolTipCounter += frames;
				if (toolTipCounter > toolTipThresh)
					toolTipCounter = toolTipThresh;
			}
			
			if (toolTipCounter == toolTipThresh && focusedMember != NULL )
			{
				ShowToolTip(focusedMember->toolTip);
			}
		}
	}
}

void Panel::ShowToolTip(ToolTip *tt)
{
	if (tt == NULL)
		return;

	currToolTip = tt;
	currToolTip->SetFromMousePos(mousePos);
}

void Panel::HideToolTip()
{
	currToolTip = NULL;
	toolTipCounter = 0;
}

void Panel::Slide(sf::Vector2i &dest, CubicBezier &bez,
	int duration)
{
	assert(duration > 0);
	slideDuration = duration;
	slideStart = pos;
	slideEnd = dest;
	slideBez = bez;
	slideFrame = 0;
}

bool Panel::IsPopup()
{
	return popup;
}

void Panel::SetColor(sf::Color c)
{
	SetRectColor(quad, c);
}

void Panel::SetPosition(const sf::Vector2i &p_pos)
{
	pos = p_pos;

	for (auto it = enemyChooseRects.begin(); it != enemyChooseRects.end(); ++it)
	{
		if ((*it)->show)
		{
			(*it)->UpdatePanelPos();
		}
	}

	for (auto it = imageChooseRects.begin(); it != imageChooseRects.end(); ++it)
	{
		if ((*it)->show)
		{
			(*it)->UpdatePanelPos();
		}
	}

	
	//SetRectTopLeft(quad, size.x, size.y, Vector2f(pos));
}

void Panel::SetCenterPos(const sf::Vector2i &p_pos)
{
	SetPosition(Vector2i(p_pos.x - size.x / 2, p_pos.y - size.y / 2));
}

bool Panel::IsMenuDropExpanded()
{
	bool menuDropExpanded = false;
	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		if ((*it).second->expanded)
		{
			menuDropExpanded = true;
			break;
		}
	}

	return menuDropExpanded;
}

const sf::Vector2i &Panel::GetMousePos()
{
	return mousePos;
}

//returns true if consumed
//checkcontained is mostly for debug, have to redo panels better soon
bool Panel::MouseUpdate()
{
	Vector2i mPos = MOUSE.GetPos();

	if (!(mPos.x >= pos.x && mPos.x <= pos.x + size.x &&
		mPos.y >= pos.y && mPos.y <= pos.y + size.y))
	{
		if (!popup)
		{
			if (!IsMenuDropExpanded())
			{
				return false;
			}
		}
		else
		{
			if (MOUSE.IsMouseLeftClicked())
			{
				handler->PanelCallback(this, "leftclickoffpopup");
			}
			
		}
	}

	mousePos = mPos - pos;

	/*if (IsSliding())
		return true;*/

	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		bool temp = (*it).second->MouseUpdate();
		if (temp)
		{
			return true;
		}
	}

	//cout << "pos: " << posx << ", " << posy << endl;
	bool dropdownUsedMouse = false;
	for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
	{
		bool temp = (*it).second->MouseUpdate();
		if (temp)
		{
			dropdownUsedMouse = true;
			//return true;
		}
	}

	if (dropdownUsedMouse)
		return true;

	for (auto it = sliders.begin(); it != sliders.end(); ++it)
	{
		bool temp = (*it).second->MouseUpdate();
		/*if (temp)
		{
			return withinPanel;
		}*/
	}

	bool textFocused = false;
	for (auto it = textBoxes.begin(); it != textBoxes.end(); ++it)
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->MouseUpdate();
		if (temp)
		{
			for (std::map<string, TextBox*>::iterator it2 = textBoxes.begin(); it2 != textBoxes.end(); ++it2)
			{
				if ((*it2).second != (*it).second)
				{
					(*it2).second->focused = false;
				}
			}

			(*it).second->focused = true;
			textFocused = true;
		}
	}
	hasFocusedTextbox = textFocused;

	for (auto it = buttons.begin(); it != buttons.end(); ++it)
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->MouseUpdate();

	}

	for (auto it = checkBoxes.begin(); it != checkBoxes.end(); ++it)
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->MouseUpdate();
	}

	for (auto it = gridSelectors.begin(); it != gridSelectors.end(); ++it)
	{
		//cout << "sending pos: " << posx << ", " << posy << endl;
		bool temp = (*it).second->MouseUpdate();
	}
	
	for (auto it = enemyChooseRects.begin(); it != enemyChooseRects.end(); ++it)
	{
		bool temp = (*it)->MouseUpdate();
	}

	for (auto it = imageChooseRects.begin(); it != imageChooseRects.end(); ++it)
	{
		bool temp = (*it)->MouseUpdate();
	}

	if (extraUpdater != NULL)
		extraUpdater->MouseUpdate();

	return true;
}

void Panel::UpdateSlide(int numUpdateFrames)
{
	if (slideDuration > 0)
	{
		slideFrame += numUpdateFrames;
		if (slideFrame < slideDuration)
		{
			double a = ((double)slideFrame) / slideDuration;
			double f = slideBez.GetValue(a);
			V2d currPos = V2d(slideStart) * (1.0 - f) + V2d(slideEnd) * f;
			pos = Vector2i(currPos);
		}
		else
		{
			slideDuration = -1;
			pos = slideEnd;
		}
	}
}

void Panel::UpdateSprites(int numUpdateFrames)
{
	for (auto it = enemyChooseRects.begin(); it != enemyChooseRects.end(); ++it)
	{
		(*it)->UpdateSprite(numUpdateFrames);
	}

	UpdateToolTip( numUpdateFrames );
}

void Panel::SendEvent( Button *b, const std::string & e )
{
	handler->ButtonCallback( b, e );
}

void Panel::SendEvent( GridSelector *gs, const std::string & e )
{
	handler->GridSelectorCallback( gs, e );
}

void Panel::SendEvent( TextBox *tb, const std::string & e )
{
	handler->TextBoxCallback( tb, e );
}

void Panel::SendEvent( CheckBox *cb, const std::string & e )
{
	handler->CheckBoxCallback( cb, e );
}

void Panel::SendEvent(Dropdown *drop, const std::string & e)
{
	handler->DropdownCallback(drop, e);
}

void Panel::SendEvent(Slider *slide, const std::string & e)
{
	handler->SliderCallback(slide, e);
}

void Panel::SendEvent(MenuDropdown *menuDrop, const std::string & e)
{
	handler->MenuDropdownCallback(menuDrop, e);
}

bool Panel::HandleEvent(sf::Event ev)
{
	switch (ev.type)
	{
	case Event::KeyPressed:
		return SendKey(ev.key.code, ev.key.shift);
		break;
	case Event::KeyReleased:
		break;
	}

	return false;
}

void Panel::ReserveEnemyRects(int num)
{
	reservedEnemyRectCount = num;
	enemyChooseRects.reserve(num);
	enemyChooseRectQuads = new Vertex[num * 4];
}

void Panel::ReserveImageRects(int num)
{
	reservedImageRectCount = num;
	imageChooseRects.reserve(num);
	imageChooseRectQuads = new Vertex[num * 4];
}

EnemyChooseRect * Panel::AddEnemyRect(
	ChooseRect::ChooseRectIdentity ident,
	sf::Vector2f &position,
	ActorType * type,
	int level)
{
	assert(enemyChooseRects.size() < reservedEnemyRectCount);
	EnemyChooseRect *ecRect = new EnemyChooseRect(ident, 
		enemyChooseRectQuads + enemyChooseRects.size() * 4,
		Vector2f( autoStart ) + position, type, level, this);
	enemyChooseRects.push_back(ecRect);

	if (autoSpace.x)
	{
		autoStart.x += ecRect->bounds.width + position.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += ecRect->bounds.height + position.y;
	}

	return ecRect;
}

ImageChooseRect * Panel::AddImageRect(ChooseRect::ChooseRectIdentity ident, 
	sf::Vector2f &position, Tileset *ts, int tileIndex, int bSize)
{
	assert(imageChooseRects.size() < reservedImageRectCount);
	ImageChooseRect *icRect = new ImageChooseRect(ident,
		imageChooseRectQuads + imageChooseRects.size() * 4,
		Vector2f(autoStart) + position, ts, tileIndex, bSize, this);
	imageChooseRects.push_back(icRect);

	if (autoSpace.x)
	{
		autoStart.x += icRect->bounds.width + position.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += icRect->bounds.height + position.y;
	}

	return icRect;
}

ImageChooseRect * Panel::AddImageRect(ChooseRect::ChooseRectIdentity ident,
	sf::Vector2f &position, Tileset *ts, const sf::IntRect &subRect, int bSize )
{
	assert(imageChooseRects.size() < reservedImageRectCount);
	ImageChooseRect *icRect = new ImageChooseRect(ident,
		imageChooseRectQuads + imageChooseRects.size() * 4,
		Vector2f(autoStart) + position, ts, subRect, bSize, this);
	imageChooseRects.push_back(icRect);

	if (autoSpace.x)
	{
		autoStart.x += icRect->bounds.width + position.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += icRect->bounds.height + position.y;
	}

	return icRect;
}

Slider * Panel::AddSlider(const std::string &name, sf::Vector2i &pos,
	int width, int minValue, int maxValue, int defaultValue)
{
	assert(sliders.count(name) == 0);
	Slider *slider = new Slider(name, autoStart + pos, width, arial, minValue, maxValue, defaultValue, this);
	sliders[name] = slider;

	if (autoSpace.x)
	{
		autoStart.x += slider->size.x + pos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += slider->size.y + pos.y;
	}

	return slider;
}

Dropdown * Panel::AddDropdown(const std::string &name, sf::Vector2i &pos,
	sf::Vector2i &size, const std::vector<std::string> &p_options, int defaultIndex )
{
	assert(dropdowns.count(name) == 0);
	Dropdown *drop = new Dropdown(name, autoStart + pos, size, arial, p_options, defaultIndex, this);
	dropdowns[name] = drop;

	if (autoSpace.x)
	{
		autoStart.x += drop->size.x + pos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += drop->size.y + pos.y;
	}

	return drop;
}

MenuDropdown * Panel::AddMenuDropdown(const std::string &name, sf::Vector2i &pos,
	sf::Vector2i &size, int optionWidth, const std::vector<std::string> &p_options)
{
	assert(dropdowns.count(name) == 0);
	MenuDropdown *menuDrop = new MenuDropdown(name, autoStart + pos, size, optionWidth, arial, p_options, this);
	menuDropdowns[name] = menuDrop;

	if (autoSpace.x)
	{
		autoStart.x += menuDrop->size.x + pos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += menuDrop->size.y + pos.y;
	}

	return menuDrop;
}

Button *Panel::AddButton( const string &name, sf::Vector2i pos, sf::Vector2f size, const std::string &text )
{
	assert( buttons.count( name ) == 0 );
	Button *button = new Button(name, autoStart.x + pos.x, autoStart.y + pos.y, size.x, size.y, arial, text, this);
	buttons[name] = button;

	if (autoSpace.x)
	{
		autoStart.x += button->size.x + pos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += button->size.y + pos.y;
	}

	return button;
}

CheckBox * Panel::AddCheckBox( const string &name, sf::Vector2i pos, bool startChecked )
{
	assert( checkBoxes.count( name ) == 0 );
	CheckBox *cb = new CheckBox(name, autoStart.x + pos.x, autoStart.y + pos.y, this);
	cb->checked = startChecked;
	checkBoxes[name] = cb;

	if (autoSpace.x)
	{
		autoStart.x += cb->size.x + pos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += cb->size.y + pos.y;
	}

	return cb;
}

TextBox * Panel::AddTextBox( const std::string &name, sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText )
{
	//Button *b = new Button( pos.x, pos.y, size.x, size.y, arial, handler );
	assert( textBoxes.count( name ) == 0 );
	TextBox *tb = new TextBox(name, autoStart.x + pos.x, autoStart.y + pos.y, width, lengthLimit, arial, this, initialText);
	textBoxes[name] = tb;

	if (autoSpace.x)
	{
		autoStart.x += tb->size.x + pos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += tb->size.y + pos.y;
	}

	return tb;
	//textBoxes.push_back(  );
}

void Panel::AddLabel( const std::string &name, sf::Vector2i labelPos, int characterHeight, const std::string &text )
{
	assert( labels.count( name ) == 0 );
	sf::Text *t = new sf::Text( text, arial, characterHeight );
	t->setPosition( labelPos.x, labelPos.y );
	t->setFillColor( Color::Black );
	auto lb = t->getLocalBounds();
	t->setOrigin(lb.left, lb.top);

	auto bounds = t->getLocalBounds();

	if (autoSpace.x)
	{
		autoStart.x += bounds.width + labelPos.x;
	}
	if (autoSpace.y)
	{
		autoStart.y += bounds.height + labelPos.y;
	}

	labels[name] = t;
}

GridSelector * Panel::AddGridSelector( const std::string &name, sf::Vector2i pos, 
	int sizex, int sizey, int tilesizex, int tilesizey, 
	bool displaySelected, bool displayMouseOver )
{
	assert( gridSelectors.count( name ) == 0 );
	GridSelector *gs = new GridSelector( pos, sizex, sizey, tilesizex, tilesizey, displaySelected, displayMouseOver, this );
	gridSelectors[name] = gs;
	return gs;
}

bool Panel::ContainsPoint(sf::Vector2i &point)
{
	IntRect r(pos.x, pos.y, size.x, size.y);
	bool rectContains = r.contains(point);
	bool menuDropExpanded = IsMenuDropExpanded();
	return rectContains || menuDropExpanded;
}

void Panel::DrawQuad(RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
}

void Panel::Deactivate()
{
	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = sliders.begin(); it != sliders.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = textBoxes.begin(); it != textBoxes.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = buttons.begin(); it != buttons.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = checkBoxes.begin(); it != checkBoxes.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = gridSelectors.begin(); it != gridSelectors.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	for (auto it = enemyChooseRects.begin(); it != enemyChooseRects.end(); ++it)
	{
		(*it)->Deactivate();
	}

	for (auto it = imageChooseRects.begin(); it != imageChooseRects.end(); ++it)
	{
		(*it)->Deactivate();
	}

	if (extraUpdater != NULL)
		extraUpdater->Deactivate();

	HideToolTip();
}

void Panel::Draw( RenderTarget *target )
{
	sf::View oldView = target->getView();

	sf::View myView = oldView;
	myView.setCenter(myView.getCenter() - Vector2f(pos));

	target->setView(myView);
	
	DrawQuad(target);

	if (extraUpdater != NULL )
	{
		extraUpdater->Draw(target);
	}



	/*sf::RectangleShape rs;
	rs.setSize( size );
	rs.setFillColor( Color( 83, 102, 188) );
	rs.setPosition( 0, 0 );
	target->draw( rs );*/

	for(auto it = labels.begin(); it != labels.end(); ++it )
	{
		//Vector2f labelPos = (*it).second->getPosition();

		//(*it).second->setPosition( pos.x + labelPos.x, pos.y + labelPos.y );
		target->draw( *(*it).second );

		//(*it).second->setPosition( labelPos.x, labelPos.y );
	}

	for(auto it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		(*it).second->Draw( target );
	}
	
	for(auto it = buttons.begin(); it != buttons.end(); ++it )
	{
		(*it).second->Draw( target );
	}

	for( auto it = checkBoxes.begin(); it != checkBoxes.end(); ++it )
	{
		(*it).second->Draw( target );
	}
	
	for( auto it = gridSelectors.begin(); it != gridSelectors.end(); ++it )
	{
		(*it).second->Draw( target );
	}

	for (auto it = sliders.begin(); it != sliders.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	target->draw(enemyChooseRectQuads, 4 * enemyChooseRects.size(), sf::Quads);
	for (auto it = enemyChooseRects.begin(); it != enemyChooseRects.end(); ++it)
	{
		(*it)->Draw(target);
	}

	target->draw(imageChooseRectQuads, 4 * imageChooseRects.size(), sf::Quads);
	for (auto it = imageChooseRects.begin(); it != imageChooseRects.end(); ++it)
	{
		(*it)->Draw(target);
	}

	if (currToolTip != NULL)
		currToolTip->Draw(target);

	target->setView(oldView);
}

bool Panel::IsSliding()
{
	return slideDuration > 0;
}

bool Panel::SendKey( sf::Keyboard::Key k, bool shift )
{	
	if( k == Keyboard::Return)
	{
		if (confirmButton != NULL)
		{
			SendEvent(confirmButton, "pressed");
		}
		return true;
	}

	for( map<string,TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		if( (*it).second->focused )
		{
			(*it).second->SendKey( k, shift );
			SendEvent( (*it).second, "modified" );
			return true;
		}
	}

	return false;
}

TextBox::TextBox( const string &n, int posx, int posy, int width_p, int lengthLimit, sf::Font &f, Panel *p,const std::string & initialText = "")
	:PanelMember( p ), pos( posx, posy ), width( width_p ), maxLength( lengthLimit ), clickedDown( false ), name( n )
{
	focused = false;
	leftBorder = 3;
	verticalBorder = 10;
	characterHeight = 20;
	
	cursor.setString("|");
	cursor.setFont(f);
	cursor.setFillColor(Color::Red);
	cursor.setCharacterSize(characterHeight);

	text.setFont( f );
	text.setFillColor( Color::Black );
	text.setCharacterSize( characterHeight );
	text.setPosition(pos.x + leftBorder, pos.y);

	SetString(initialText);
	SetCursorIndex(initialText.length());

	size = Vector2i(width, characterHeight + verticalBorder);

	
}

void TextBox::SetString(const std::string &str)
{
	text.setString(str);
}

std::string TextBox::GetString()
{
	return text.getString();
}

void TextBox::Deactivate()
{
	focused = false;
	clickedDown = false;
}

void TextBox::SetCursorIndex( int index )
{
	cursorIndex = index;



	cursor.setPosition( text.getLocalBounds().width + leftBorder, pos.y );
}

void TextBox::SetCursorIndex( Vector2i &mousePos )
{
	//do this later

	/*int stringLength = text.getString().getSize();
	Text textCopy( text );
	Text temp( text );
	for( int i = 0; i < stringLength; ++i )
	{
		temp.setString( text.getString().substring( 
		
	}*/
}

void TextBox::SendKey( Keyboard::Key k, bool shift )
{
	char c = 0;
	switch( k )
	{
		case Keyboard::A:
			c = 'a';
			break;
		case Keyboard::B:
			c = 'b';
			break;
		case Keyboard::C:
			c = 'c';
			break;
		case Keyboard::D:
			c = 'd';
			break;
		case Keyboard::E:
			c = 'e';
			break;
		case Keyboard::F:
			c = 'f';
			break;
		case Keyboard::G:
			c = 'g';
			break;
		case Keyboard::H:
			c = 'h';
			break;
		case Keyboard::I:
			c = 'i';
			break;
		case Keyboard::J:
			c = 'j';
			break;
		case Keyboard::K:
			c = 'k';
			break;
		case Keyboard::L:
			c = 'l';
			break;
		case Keyboard::M:
			c = 'm';
			break;
		case Keyboard::N:
			c = 'n';
			break;
		case Keyboard::O:
			c = 'o';
			break;
		case Keyboard::P:
			c = 'p';
			break;
		case Keyboard::Q:
			c = 'q';
			break;
		case Keyboard::R:
			c = 'r';
			break;
		case Keyboard::S:
			c = 's';
			break;
		case Keyboard::T:
			c = 't';
			break;
		case Keyboard::U:
			c = 'u';
			break;
		case Keyboard::V:
			c = 'v';
			break;
		case Keyboard::W:
			c = 'w';
			break;
		case Keyboard::X:
			c = 'x';
			break;
		case Keyboard::Y:
			c = 'y';
			break;
		case Keyboard::Z:
			c = 'z';
			break;
		case Keyboard::Space:
			c = ' ';
			break;
		case Keyboard::Num0:
		case Keyboard::Numpad0:
			c = '0';
			break;
		case Keyboard::Num1:
		case Keyboard::Numpad1:
			c = '1';
			break;
		case Keyboard::Num2:
		case Keyboard::Numpad2:
			c = '2';
			break;
		case Keyboard::Num3:
		case Keyboard::Numpad3:
			c = '3';
			break;
		case Keyboard::Num4:
		case Keyboard::Numpad4:
			c = '4';
			break;
		case Keyboard::Num5:
		case Keyboard::Numpad5:
			c = '5';
			break;
		case Keyboard::Num6:
		case Keyboard::Numpad6:
			c = '6';
			break;
		case Keyboard::Num7:
		case Keyboard::Numpad7:
			c = '7';
			break;
		case Keyboard::Num8:
		case Keyboard::Numpad8:
			c = '8';
			break;
		case Keyboard::Num9:
		case Keyboard::Numpad9:
			c = '9';
			break;
		case Keyboard::Dash:
			c = '-';
			break;
		case Keyboard::Period:
			c = '.';
			break;
		case Keyboard::BackSpace:
			{
			//text.setString( text.getString().substring( 0, cursorIndex ) + text.getString().substring( cursorIndex + 1 ) );
			cursorIndex -= 1;

			if( cursorIndex < 0 )
				cursorIndex = 0;
			else
			{
				
				sf::String s = text.getString();
				if( s.getSize() > 0 )
				{
					s.erase( cursorIndex );
					text.setString( s );
				}
			}

			break;
			}
		case Keyboard::Left:
			cursorIndex -= 1;
			if( cursorIndex < 0 )
				cursorIndex = 0;
			break;
		case Keyboard::Right:
			if( cursorIndex < text.getString().getSize() )
				cursorIndex += 1;
			break;
		
	}

	if( c != 0 && text.getString().getSize() < maxLength )
	{
		if( shift && c >= 'a' && c <= 'z' )
		{
			c -= 32;
		}
		else if( shift && c == '-' )
		{
			c = '_';
		}
		sf::String s = text.getString();
		
		s.insert( cursorIndex, sf::String( c ) );
		text.setString( s );
		cursorIndex++;
	}

	sf::Text test;
	test = text;
	test.setString( test.getString().substring( 0, cursorIndex) );
	cursor.setPosition( pos.x + test.getLocalBounds().width, pos.y);
}

bool TextBox::MouseUpdate()
{
	sf::Vector2i mousePos = panel->GetMousePos();
	sf::Rect<int> r( pos.x, pos.y, width, characterHeight + verticalBorder );

	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if( MOUSE.IsMouseDownLeft() )
	{	
		if(containsMouse)
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if(containsMouse && clickedDown )
		{
			clickedDown = false;
			
			//need to make it so that if you click a letter the cursor goes to the left of it. too lazy for now.

			/*int textLength = text.getString().getSize();

			sf::Text tempText;
			tempText = text;
			tempText.setString( text.getString().substring( 0, 1 ) );

			sf::Rect<int> first( pos.x, pos.y, tempText.getLocalBounds().width / 2 , characterHeight + verticalBorder );
			if( first.contains( sf::Vector2i( posx, posy ) ) )
			{
				cursorIndex = 0;
				cursor.setPosition( pos.x, pos.y);
			}*/
			
			//if( textLength > 1 )
			//{
			//	int startX = 0;
			//	for( int i = 1; i <= textLength; ++i )
			//	{
			//		tempText.setString( text.getString().substring( 0, i );
			//		 //= tempText.getLocalBounds().left + tempText.getLocalBounds().width;

			//		//tempText.setString( text.getString().substring( i-1, 2 ) );
			//		sf::Rect<int> temp( pos.x + startX, pos.y, tempText.getLocalBounds().width / 2 , characterHeight + verticalBorder );
			//		if( temp.contains( sf::Vector2i( posx, posy ) ) )
			//		{

			//		}
			//	}
			//}

			//SetCursorIndex( pos );
			cursor.setPosition( pos.x + text.getLocalBounds().width + leftBorder, pos.y );
			cursorIndex = text.getString().getSize();
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void TextBox::Draw( sf::RenderTarget *target )
{
	sf::RectangleShape rs;
	//rs.setSize( Vector2f( 300, characterHeight + verticalBorder) );
	rs.setSize( Vector2f( size ) );
	rs.setFillColor( Color::White );
	rs.setPosition( pos.x, pos.y );

	target->draw( rs );

	if( focused )
	{
		target->draw( cursor );
	}
	target->draw( text );
}

Button::Button( const string &n, int posx, int posy, int width, int height, sf::Font &f, const std::string & t, Panel *p )
	:PanelMember( p ), pos( posx, posy ), clickedDown( false ), characterHeight( 20 ), size( width, height ), name( n )
{	
	text.setString( t );
	text.setFont( f );
	text.setFillColor( Color::White );
	text.setCharacterSize( characterHeight );
}

void Button::Deactivate()
{
	clickedDown = false;
}

bool Button::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	sf::Rect<int> r( pos.x, pos.y, size.x, size.y );

	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if( MOUSE.IsMouseLeftClicked() )
	{	
		if(containsMouse)
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if(containsMouse && clickedDown )
		{
			clickedDown = false;
			panel->SendEvent( this, "pressed" );
			return true;
		}
		else 
		{
			clickedDown = false;
		}
	}

	return false;
}

void Button::Draw( RenderTarget *target )
{
	text.setPosition( pos.x + size.x / 2 - text.getLocalBounds().width / 2, pos.y + size.y / 2 - text.getLocalBounds().height / 2);

	sf::RectangleShape rs;
	rs.setSize( size );
	rs.setPosition( pos.x, pos.y );
	if( clickedDown )
		rs.setFillColor( Color::Green );
	else
		rs.setFillColor( Color::Blue );

	target->draw( rs );

	target->draw( text );
}

CheckBox::CheckBox( const std::string &n, int posx, int posy, Panel *p )
	:PanelMember( p ), pos( posx, posy ), clickedDown( false ), name( n ), checked( false )
{
	size = Vector2i(CHECKBOXSIZE, CHECKBOXSIZE);
}

void CheckBox::Deactivate()
{
	clickedDown = false;
}

void CheckBox::SetLockedStatus(bool check, bool lock)
{
	checked = check;
	locked = lock;
}

bool CheckBox::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	sf::Rect<int> r( pos.x, pos.y, CHECKBOXSIZE, CHECKBOXSIZE);
	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if (locked)
		return false;

	if( MOUSE.IsMouseLeftClicked() )
	{	
		if( containsMouse )
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if( containsMouse && clickedDown )
		{
			clickedDown = false;
			checked = !checked;
			if( checked )
			{
				panel->SendEvent( this, "checked" );
			}
			else
			{
				panel->SendEvent( this, "unchecked" );
			}
			
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}


//void CheckBox::SetPanelPos(const sf::Vector2i &p_pos)
//{
//
//}

void CheckBox::Draw( RenderTarget *target )
{
	sf::RectangleShape rs;
	rs.setSize( sf::Vector2f(CHECKBOXSIZE, CHECKBOXSIZE) );
	rs.setPosition( pos.x, pos.y );

	if (locked)
	{
		if (checked)
		{
			rs.setFillColor(Color( 255, 100, 100 ));
		}
		else
		{
			rs.setFillColor(Color( 100,100,100));
		}
	}
	else
	{
		if (clickedDown)
		{
			rs.setFillColor(Color::Cyan);
		}
		else
		{
			if (checked)
			{
				rs.setFillColor(Color::Magenta);
			}
			else
			{
				rs.setFillColor(Color::Black);
			}
		}
	}

	target->draw( rs );
}

ErrorBar::ErrorBar( sf::Font &f )
	:myFont(&f)
{
	CreateErrorTable();
	int rectHeight = 30;
	rect.setSize(Vector2f(1920, rectHeight));
	rect.setPosition(Vector2f(0, 1080 - rectHeight));
	Color rectColor = Color::Red;
	//rectColor.a = 25;
	rect.setFillColor(rectColor);
	show = false;

	errorText.setCharacterSize(20);
	errorText.setFillColor(Color::Black);
	errorText.setFont(f);
	errorText.setPosition(5, (1080 - rectHeight) + 5);

	//SetText("BLAH BLAH BLAH");

	SetShown(false);
}

void ErrorBar::SetShown(bool s)
{
	show = s;
}

void ErrorBar::SetText(const std::string &msg)
{
	errorText.setString(msg);
}

void ErrorBar::ShowText(const std::string &msg)
{
	SetShown(true);
	SetText(msg);
}

void ErrorBar::ShowError( ErrorType er )
{
	ShowText(errorStringVec[er]);
}

void ErrorBar::Draw(sf::RenderTarget *target)
{
	if (show)
	{
		target->draw(rect);
		target->draw(errorText);
	}
}

void ErrorBar::CreateErrorTable()
{
	errorStringVec.resize(ERR_Count);
	errorStringVec[ERR_PLAYER_INTERSECTS_POLY] = "The player illegally intersects one or more polygons";
	errorStringVec[ERR_POLY_INTERSECTS_POLY] = "A polygon illegally intersects one or more polygons";
	errorStringVec[ERR_POLY_CONTAINS_POLY] = "A polygon illegally contains one or more polygons";
	errorStringVec[ERR_POLY_INTERSECTS_ENEMY] = "A polygon illegally intersects one or more enemies";
	errorStringVec[ERR_POLY_INTERSECTS_GATE] = "A polygon illegally intersects one or more gates";
	

	errorStringVec[ERR_POINTS_MOVE_TOO_CLOSE] = "Attempted to move points too close to one another";
	errorStringVec[ERR_POLY_HAS_SLIVER] = "A polygon has created an illegal sliver shape";
	errorStringVec[ERR_POLY_INTERSECTS_ITSELF] = "A polygon has illegally intersected its own lines";
	errorStringVec[ERR_ENEMY_NEEDS_GROUND] = "An enemy needed to be attached to a polygon and couldn't find one";
	errorStringVec[ERR_ENEMY_NEEDS_RAIL] = "An enemy neeed to be attached to a rail and couldn't find one";
	errorStringVec[ERR_ENEMY_NEEDS_GROUND_OR_RAIL] = "An enemy needed to be attached to a polygon or rail and couldn't either";

	//these might nevRer be relevant?
	errorStringVec[ERR_POLY_INCORRECT_WINDING_INVERSE] = "An inverse polygon has the wrong winding";
	errorStringVec[ERR_POLY_INCORRECT_WINDING] = "A polygon has the wrong winding";

	errorStringVec[ERR_GATE_INSTERSECTS_ENEMY] = "A gate illegally intersects one or more polygons";
	errorStringVec[ERR_GATE_SAME_POINT] = "A gate cannot have the same start and end point";
	errorStringVec[ERR_GATE_CREATES_SLIVER] = "A gate created an illegal sliver shape";
	errorStringVec[ERR_GATE_INTERSECTS_GATE] = "A gate illegally intersected another gate";
	errorStringVec[ERR_GATE_POINT_ALREADY_OCCUPIED] = "A gate cannot use the same point that another gate is already using";
	errorStringVec[ERR_GATE_INSTERSECTS_ENEMY] = "A gate illegally intersects an enemy";
	errorStringVec[ERR_GATE_NEEDS_BOTH_POINTS] = "A gate needs 2 points to be valid";

	errorStringVec[ERR_POLY_NEEDS_THREE_OR_MORE_POINTS] = "A polygon needs at least 3 points to be valid";
	errorStringVec[ERR_POINTS_TOO_CLOSE_TO_SEG_IN_PROGRESS] = "Points on polygon too close to the draw segment. You can adjust minimum draw separation in settings";
	errorStringVec[ERR_POINT_TO_CLOSE_TO_OTHERS] = "Points on polygon in progress too close together. You can adjust minimum draw separation in settings";
	errorStringVec[ERR_LINES_INTERSECT_IN_PROGRESS] = "Polygon in progress intersects itself";

	errorStringVec[ERR_SELECTED_TERRAIN_MULTIPLE_LAYERS] = "Cannot modify material type on multiple layers at once";
	
}

UIMouse::UIMouse()
{
	ResetMouse();
}

void UIMouse::Update( sf::Vector2i &mPos)
{
	bool mouseDownL = Mouse::isButtonPressed(Mouse::Left);
	bool mouseDownR = Mouse::isButtonPressed(Mouse::Right);

	lastMouseDownLeft = isMouseDownLeft;
	isMouseDownLeft = mouseDownL;

	lastMouseDownRight = isMouseDownRight;
	isMouseDownRight = mouseDownR;

	mousePos = mPos;

	consumed = false;
}

bool UIMouse::IsMouseDownLeft()
{
	return isMouseDownLeft;
}

bool UIMouse::IsMouseDownRight()
{
	return isMouseDownRight;
}

bool UIMouse::IsMouseLeftClicked()
{
	return isMouseDownLeft && !lastMouseDownLeft;
}

bool UIMouse::IsMouseLeftReleased()
{
	return !isMouseDownLeft && lastMouseDownLeft;
}

bool UIMouse::IsMouseRightClicked()
{
	return isMouseDownRight && !lastMouseDownRight;
}

bool UIMouse::IsMouseRightReleased()
{
	return !isMouseDownRight && lastMouseDownRight;
}

void UIMouse::ResetMouse()
{
	isMouseDownLeft = false;
	lastMouseDownLeft = false;
	isMouseDownRight = false;
	lastMouseDownRight = false;
}