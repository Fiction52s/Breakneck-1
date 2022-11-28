#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"
#include "MusicSelector.h"
#include "LobbyBrowser.h"
#include "UIMouse.h"

using namespace sf;
using namespace std;

Panel::Panel(const string &n, int width, int height, GUIHandler *h, bool pop)
	:handler(h), size(width, height), name(n), popup(pop)
	//:t( 0, 0, 200, 10, f, "hello" ), t2( 0, 100, 100, 10, f, "blah" ), b( 0, 50, 100, 50, f, "button!" )
{
	imageChooseRectQuads = NULL;
	enemyChooseRectQuads = NULL;
	textChooseRectQuads = NULL;
	lobbyChooseRectQuads = NULL;
	reservedEnemyRectCount = 0;
	reservedImageRectCount = 0;
	reservedTextRectCount = 0;
	reservedLobbyRectCount = 0;
	arial.loadFromFile("Resources/Fonts/Breakneck_Font_01.ttf"); //each panel loads the font separately, this is bad
	SetPosition(Vector2i(0, 0));
	extraUpdater = NULL;

	Color defaultColor(83, 102, 188);
	SetRectColor(quad, defaultColor);
	SetRectTopLeft(quad, size.x, size.y, Vector2f(0, 0));

	slideFrame = 0;
	slideDuration = -1;

	currToolTip = NULL;
	toolTipThresh = 30;
	toolTipCounter = 0;

	focusedMember = NULL;
	confirmButton = NULL;
	cancelButton = NULL;

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

	for (auto it = tabGroups.begin(); it != tabGroups.end(); ++it)
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

	for (auto it = hyperLinks.begin(); it != hyperLinks.end(); ++it)
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

	for (auto it = scrollBars.begin(); it != scrollBars.end(); ++it)
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

	for (auto it = textChooseRects.begin(); it != textChooseRects.end(); ++it)
	{
		delete (*it);
	}
	if (textChooseRectQuads != NULL)
	{
		delete[] textChooseRectQuads;
	}

	for (auto it = lobbyChooseRects.begin(); it != lobbyChooseRects.end(); ++it)
	{
		delete (*it);
	}
	if (lobbyChooseRectQuads != NULL)
	{
		delete[] lobbyChooseRectQuads;
	}
	


	/*for (auto it = enemyChoosers.begin(); it != enemyChoosers.end(); ++it)
	{
	delete (*it).second;
	}*/
}

void Panel::SetAutoSpacing(bool aSpaceX, bool aSpaceY, Vector2i start, Vector2i p_defaultExtra)
{
	autoSpace.x = aSpaceX;
	autoSpace.y = aSpaceY;
	autoStart = start;
	defaultExtra = p_defaultExtra;
	autoSpacePaused = false;
}

void Panel::StopAutoSpacing()
{
	autoSpace.x = false;
	autoSpace.y = false;
	autoStart = Vector2i( 0, 0 );
	autoSpacePaused = true;
}

void Panel::PauseAutoSpacing()
{
	autoSpacePaused = true;
}

void Panel::UnpauseAutoSpacing()
{
	autoSpacePaused = false;
}

void Panel::SetConfirmButton(Button *b)
{
	confirmButton = b;
}

void Panel::SetCancelButton(Button *b)
{
	cancelButton = b;
}

void Panel::SetFocusedMember(PanelMember*pm)
{
	if (pm != focusedMember)
	{
		HideToolTip();
		if (focusedMember != NULL)
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

void Panel::UpdateToolTip(int frames)
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

			if (toolTipCounter == toolTipThresh && focusedMember != NULL)
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

bool Panel::IsDropActive()
{
	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		if ((*it).second->expanded)
		{
			return true;
		}
	}

	for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
	{
		if ((*it).second->ContainsMouse())
		{
			return true;
		}
	}

	return false;
}

bool Panel::IsSlideActive()
{
	for (auto it = sliders.begin(); it != sliders.end(); ++it)
	{
		if ((*it).second->clickedDown)
		{
			return true;
		}
	}

	return false;
}

const sf::Vector2i &Panel::GetMousePos()
{
	return mousePos;
}

//returns true if consumed
//checkcontained is mostly for debug, have to redo panels better soon
bool Panel::MouseUpdate()
{
	MainMenu *mm = MainMenu::GetInstance();

	ControllerUpdate();

	Vector2i mPos = MOUSE.GetPos();
	if (!(mPos.x >= pos.x && mPos.x <= pos.x + size.x &&
		mPos.y >= pos.y && mPos.y <= pos.y + size.y))
	{
		if (!popup)
		{
			if (!IsDropActive() && !IsSlideActive())
			{
				return false;
			}
		}
		else
		{
			if (MOUSE.IsMouseLeftClicked())
			{
				if (!IsDropActive() && !IsSlideActive())
				{
					if (handler != NULL)
					{
						handler->PanelCallback(this, "leftclickoffpopup");
					}
				}
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
			break;
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

	for (auto it = tabGroups.begin(); it != tabGroups.end(); ++it)
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

	for (auto it = hyperLinks.begin(); it != hyperLinks.end(); ++it)
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

	for (auto it = textChooseRects.begin(); it != textChooseRects.end(); ++it)
	{
		bool temp = (*it)->MouseUpdate();
	}

	for (auto it = lobbyChooseRects.begin(); it != lobbyChooseRects.end(); ++it)
	{
		bool temp = (*it)->MouseUpdate();
	}

	for (auto it = scrollBars.begin(); it != scrollBars.end(); ++it)
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->MouseUpdate();
	}

	if (extraUpdater != NULL)
		extraUpdater->MouseUpdate();

	return true;
}

void Panel::ControllerUpdate()
{
	if (CONTROLLERS.ButtonPressed_Start())
	{
		Confirm();
	}
	else if (CONTROLLERS.ButtonPressed_B())
	{
		Cancel();
	}
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

	UpdateToolTip(numUpdateFrames);

	UpdateFrame(numUpdateFrames);
}

void Panel::UpdateFrame(int numUpdateFrames)
{
	if (extraUpdater != NULL)
		extraUpdater->UpdateFrame(numUpdateFrames);
}

void Panel::SendEvent(Button *b, const std::string & e)
{
	handler->ButtonCallback(b, e);
}

void Panel::SendEvent(TabGroup *tg, const std::string & e)
{
	handler->TabGroupCallback(tg, e);
}

void Panel::SendEvent(HyperLink *link, const std::string & e)
{
	handler->HyperLinkCallback(link, e);
}

void Panel::SendEvent(GridSelector *gs, const std::string & e)
{
	handler->GridSelectorCallback(gs, e);
}

void Panel::SendEvent(TextBox *tb, const std::string & e)
{
	handler->TextBoxCallback(tb, e);
}

void Panel::SendEvent(CheckBox *cb, const std::string & e)
{
	handler->CheckBoxCallback(cb, e);
}

void Panel::SendEvent(ScrollBar *sb, const std::string &e)
{
	handler->ScrollBarCallback(sb, e);
}

void Panel::SendEvent(Dropdown *drop, const std::string & e)
{
	handler->DropdownCallback(drop, e);
}

void Panel::SendEvent(Slider *slide)
{
	handler->SliderCallback(slide);
}

void Panel::SendEvent(MenuDropdown *menuDrop, const std::string & e)
{
	handler->MenuDropdownCallback(menuDrop, e);
}

void Panel::Confirm()
{
	/*if (confirmButton != NULL && !confirmButton->hidden)
	{
		SendEvent(confirmButton, "pressed");
	}*/
	handler->ConfirmCallback(this);
}

void Panel::Cancel()
{
	/*if (cancelButton != NULL && !cancelButton->hidden)
	{
		SendEvent(cancelButton, "pressed");
	}*/
	handler->CancelCallback(this);
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
	case Event::MouseWheelMoved:
		if (ev.mouseWheel.delta != 0)
		{
			if (extraUpdater != NULL)
			{
				extraUpdater->MouseScroll(ev.mouseWheel.delta);
			}
			else
			{
				for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
				{
					if ((*it).second->expanded)
					{
						(*it).second->MouseScroll(ev.mouseWheel.delta);
						break;
					}
				}
			}
		}
		return true;
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

void Panel::ReserveTextRects(int num)
{
	reservedTextRectCount = num;
	textChooseRects.reserve(num);
	textChooseRectQuads = new Vertex[num * 4];
}

void Panel::ReserveLobbyRects(int num)
{
	reservedLobbyRectCount = num;
	lobbyChooseRects.reserve(num);
	lobbyChooseRectQuads = new Vertex[num * 4];
}

void Panel::AddAutoSpaceX(int x)
{
	if (autoSpace.x && !autoSpacePaused)
	{
		autoStart.x += x + defaultExtra.x;
	}
}
void Panel::AddAutoSpaceY(int y)
{
	if (autoSpace.y && !autoSpacePaused)
	{
		autoStart.y += y + defaultExtra.y;
	}
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
		Vector2f(autoStart) + position, type, level, this);
	enemyChooseRects.push_back(ecRect);

	AddAutoSpaceX(100 + position.x);
	AddAutoSpaceY(100 + position.y);

	return ecRect;
}

ImageChooseRect * Panel::AddImageRect(ChooseRect::ChooseRectIdentity ident,
	sf::Vector2f &position, Tileset *ts, int tileIndex, int bSize)
{
	assert(imageChooseRects.size() < reservedImageRectCount);
	ImageChooseRect *icRect = new ImageChooseRect(ident,
		imageChooseRectQuads + imageChooseRects.size() * 4,
		Vector2f(autoStart) + position, ts, tileIndex, Vector2f( bSize, bSize ), this);
	imageChooseRects.push_back(icRect);

	AddAutoSpaceX(bSize + position.x);
	AddAutoSpaceY(bSize + position.y);

	return icRect;
}

ImageChooseRect * Panel::AddImageRect(ChooseRect::ChooseRectIdentity ident,
	sf::Vector2f &position, Tileset *ts, const sf::IntRect &subRect, int bSize)
{
	assert(imageChooseRects.size() < reservedImageRectCount);
	ImageChooseRect *icRect = new ImageChooseRect(ident,
		imageChooseRectQuads + imageChooseRects.size() * 4,
		Vector2f(autoStart) + position, ts, subRect, Vector2f(bSize, bSize), this);
	imageChooseRects.push_back(icRect);

	AddAutoSpaceX(bSize + position.x);
	AddAutoSpaceY(bSize + position.y);

	return icRect;
}

TextChooseRect * Panel::AddTextRect(ChooseRect::ChooseRectIdentity ident,
	sf::Vector2f &position, sf::Vector2f &bSize, const std::string &text)
{
	assert(textChooseRects.size() < reservedTextRectCount);
	TextChooseRect *icRect = new TextChooseRect(ident,
		textChooseRectQuads + textChooseRects.size() * 4,
		Vector2f(autoStart) + position, text, bSize, this);
	textChooseRects.push_back(icRect);

	AddAutoSpaceX(bSize.x + position.x);
	AddAutoSpaceY(bSize.y + position.y);

	return icRect;
}

LobbyChooseRect * Panel::AddLobbyRect(sf::Vector2f &position, sf::Vector2f &bSize)
{
	assert(lobbyChooseRects.size() < reservedLobbyRectCount);
	LobbyChooseRect *lcRect = new LobbyChooseRect(ChooseRect::ChooseRectIdentity::I_LOBBY,
		lobbyChooseRectQuads + lobbyChooseRects.size() * 4,
		Vector2f(autoStart) + position, "testtt", bSize, this);
	lobbyChooseRects.push_back(lcRect);

	AddAutoSpaceX(bSize.x + position.x);
	AddAutoSpaceY(bSize.y + position.y);

	return lcRect;
}

TabGroup *Panel::AddTabGroup(const std::string &name, sf::Vector2i &pos, std::vector<std::string> &tabStrings, int memberWidth, int height)
{
	assert(tabGroups.count(name) == 0);
	TabGroup *tabGroup = new TabGroup(name, autoStart.x + pos.x, autoStart.y + pos.y, tabStrings, memberWidth, height, arial, this);
	tabGroups[name] = tabGroup;

	AddAutoSpaceX(tabGroup->totalSize.x + pos.x);
	AddAutoSpaceY(tabGroup->totalSize.y + pos.y);

	return tabGroup;
}

Slider * Panel::AddSlider(const std::string &name, sf::Vector2i &pos,
	int width, int minValue, int maxValue, int defaultValue)
{
	assert(sliders.count(name) == 0);
	Slider *slider = new Slider(name, autoStart + pos, width, arial, minValue, maxValue, defaultValue, this);
	sliders[name] = slider;

	AddAutoSpaceX(slider->size.x + pos.x);
	AddAutoSpaceY(slider->size.y + pos.y);

	return slider;
}

Slider * Panel::AddFloatSlider(const std::string &name, sf::Vector2i &pos,
	int width, float minValue, float maxValue, float defaultValue, float step)
{
	float iDefault = ((defaultValue - minValue) / step);
	int numSpaces = (maxValue - minValue) / step;

	assert(sliders.count(name) == 0);
	Slider *slider = new Slider(name, autoStart + pos, width, arial, 0, numSpaces, iDefault, this);
	slider->SetFloatMode(minValue, step);

	sliders[name] = slider;

	AddAutoSpaceX(slider->size.x + pos.x);
	AddAutoSpaceY(slider->size.y + pos.y);

	return slider;
}

Dropdown * Panel::AddDropdown(const std::string &name, sf::Vector2i &pos,
	sf::Vector2i &size, const std::vector<std::string> &p_options, int defaultIndex)
{
	assert(dropdowns.count(name) == 0);
	Dropdown *drop = new Dropdown(name, autoStart + pos, size, arial, p_options, defaultIndex, this);
	dropdowns[name] = drop;

	AddAutoSpaceX(drop->size.x + pos.x);
	AddAutoSpaceY(drop->size.y + pos.y);

	return drop;
}

MenuDropdown * Panel::AddMenuDropdown(const std::string &name, sf::Vector2i &pos,
	sf::Vector2i &size, int optionWidth, const std::vector<std::string> &p_options)
{
	assert(dropdowns.count(name) == 0);
	MenuDropdown *menuDrop = new MenuDropdown(name, autoStart + pos, size, optionWidth, arial, p_options, this);
	menuDropdowns[name] = menuDrop;

	AddAutoSpaceX(menuDrop->size.x + pos.x);
	AddAutoSpaceY(menuDrop->size.y + pos.y);

	return menuDrop;
}

Button *Panel::AddButton(const string &name, sf::Vector2i pos, sf::Vector2f size, const std::string &text)
{
	assert(buttons.count(name) == 0);
	Button *button = new Button(name, autoStart.x + pos.x, autoStart.y + pos.y, size.x, size.y, arial, text, this);
	buttons[name] = button;

	AddAutoSpaceX(button->size.x + pos.x);
	AddAutoSpaceY(button->size.y + pos.y);

	return button;
}

HyperLink * Panel::AddHyperLink(const std::string &name, sf::Vector2i pos, int characterHeight, const std::string &text,
	const std::string &link)
{
	assert(hyperLinks.count(name) == 0);
	HyperLink *hyperLink = new HyperLink(name, autoStart.x + pos.x, autoStart.y + pos.y, characterHeight, arial, text, link, this);
	hyperLinks[name] = hyperLink;

	auto bounds = hyperLink->text.getGlobalBounds();//getLocalBounds();
	bounds.height = hyperLink->text.getFont()->getLineSpacing( characterHeight );

	AddAutoSpaceX(bounds.width + pos.x);
	AddAutoSpaceY(bounds.height + pos.y);


	return hyperLink;
}

CheckBox * Panel::AddCheckBox(const string &name, sf::Vector2i pos, bool startChecked)
{
	assert(checkBoxes.count(name) == 0);
	CheckBox *cb = new CheckBox(name, autoStart.x + pos.x, autoStart.y + pos.y, this);
	cb->checked = startChecked;
	checkBoxes[name] = cb;

	AddAutoSpaceX(cb->size.x + pos.x);
	AddAutoSpaceY(cb->size.y + pos.y);

	return cb;
}

CheckBox * Panel::AddLabeledCheckBox(
	const std::string &name, sf::Vector2i pos, 
	const std::string &labelText, bool startChecked )
{
	int extraSpacing = 8;
	Vector2i oldAutoStart = autoStart;
	Vector2i labelStart = pos;
	labelStart.y += 6;
	Label *lab = AddLabel(name + "label", labelStart, 24, labelText);
	pos.x += lab->text.getLocalBounds().left + lab->text.getLocalBounds().width + extraSpacing;
	autoStart = oldAutoStart;
	return AddCheckBox(name, pos, startChecked);
}

Slider * Panel::AddLabeledSlider(const std::string &name, sf::Vector2i pos,
	const std::string &labelText, int width, int minValue, int maxValue, int defaultValue )
{
	int extraSpacing = 8;

	Vector2i oldAutoStart = autoStart;
	Vector2i labelStart = pos;

	Label *lab = AddLabel(name + "label", labelStart, 24, labelText);

	pos.x += lab->text.getLocalBounds().left + lab->text.getLocalBounds().width + extraSpacing;
	autoStart = oldAutoStart;

	labelStart.y += 6;

	return AddSlider(name, pos, width, minValue, maxValue, defaultValue);
}

ScrollBar *Panel::AddScrollBar(const std::string &name, sf::Vector2i &pos, sf::Vector2i &size, int p_numRows, int p_numDisplayedRows)
{
	assert(scrollBars.count(name) == 0);
	ScrollBar *sb = new ScrollBar(name, Vector2i( autoStart.x + pos.x, autoStart.y + pos.y), size, p_numRows, p_numDisplayedRows, this);
	scrollBars[name] = sb;

	AddAutoSpaceX(sb->size.x + pos.x);
	AddAutoSpaceY(sb->size.y + pos.y);

	return sb;
}

TextBox * Panel::AddLabeledTextBox(const std::string &name, sf::Vector2i pos, bool labelToleft, int rows, int cols, int charHeight, int lengthLimit,
	const std::string &initialText, const std::string &labelText)
{
	int extraSpacing = 8;
	Vector2i oldAutoStart = autoStart;
	Vector2i labelStart = pos;
	//labelStart.y += 6;
	Label *lab = AddLabel(name + "label", labelStart, 24, labelText);

	if (labelToleft)
	{
		pos.x += lab->text.getLocalBounds().left + lab->text.getLocalBounds().width + extraSpacing;
	}
	else
	{
		pos.y += lab->text.getLocalBounds().top + lab->text.getLocalBounds().height + extraSpacing;
	}

	autoStart = oldAutoStart;
	//needs to be fixed soon
	return AddTextBox(name, pos, rows, cols, charHeight, lengthLimit, initialText);
}

HyperLink * Panel::AddLabeledHyperLink(const std::string &name, sf::Vector2i pos, int characterHeight, const std::string &text,
	const std::string &link, const std::string &labelText)
{
	int extraSpacing = 8;
	Vector2i oldAutoStart = autoStart;
	Vector2i labelStart = pos;
	labelStart.y += 6;
	Label *lab = AddLabel(name + "label", labelStart, characterHeight, labelText);
	pos.x += lab->text.getLocalBounds().left + lab->text.getLocalBounds().width + extraSpacing;
	autoStart = oldAutoStart;
	return AddHyperLink(name, pos, characterHeight, text, link);
}

TextBox * Panel::AddTextBox(const std::string &name, sf::Vector2i pos, int rows, int cols, int charHeight, int lengthLimit, const std::string &initialText)
{
	//Button *b = new Button( pos.x, pos.y, size.x, size.y, arial, handler );
	assert(textBoxes.count(name) == 0);
	TextBox *tb = new TextBox(name, autoStart.x + pos.x, autoStart.y + pos.y, rows, cols, charHeight, lengthLimit, arial, this, initialText);
	textBoxes[name] = tb;

	AddAutoSpaceX(tb->size.x + pos.x);
	AddAutoSpaceY(tb->size.y + pos.y);

	return tb;
	//textBoxes.push_back(  );
}

TextBox * Panel::AddTextBox(const std::string &name, sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText)
{
	int charHeight = 20;
	//Button *b = new Button( pos.x, pos.y, size.x, size.y, arial, handler );
	assert(textBoxes.count(name) == 0);
	TextBox *tb = new TextBox(name, autoStart.x + pos.x, autoStart.y + pos.y, 1, lengthLimit, charHeight, lengthLimit, arial, this, initialText);
	textBoxes[name] = tb;

	AddAutoSpaceX(tb->size.x + pos.x);
	AddAutoSpaceY(tb->size.y + pos.y);

	return tb;
	//textBoxes.push_back(  );
}

Label * Panel::AddLabel(const std::string &name, sf::Vector2i labelPos, int characterHeight, const std::string &str)
{
	assert(labels.count(name) == 0);

	Label *lab = new Label( this );

	lab->text.setFont(arial);
	lab->text.setCharacterSize(characterHeight);
	lab->text.setString(str);
	lab->text.setFillColor(Color::Black);
	lab->text.setPosition(autoStart.x + labelPos.x, autoStart.y + labelPos.y);
	
	auto lb = lab->text.getLocalBounds();
	lab->text.setOrigin(lb.left, lb.top);

	auto bounds = lab->text.getLocalBounds();

	AddAutoSpaceX(bounds.width + labelPos.x);
	AddAutoSpaceY(bounds.height + labelPos.y);

	labels[name] = lab;

	return lab;
}

GridSelector * Panel::AddGridSelector(const std::string &name, sf::Vector2i pos,
	int sizex, int sizey, int tilesizex, int tilesizey,
	bool displaySelected, bool displayMouseOver)
{
	assert(gridSelectors.count(name) == 0);
	GridSelector *gs = new GridSelector(pos, sizex, sizey, tilesizex, tilesizey, displaySelected, displayMouseOver, this);
	gridSelectors[name] = gs;
	return gs;
}

bool Panel::ContainsPoint(sf::Vector2i &point)
{
	IntRect r(pos.x, pos.y, size.x, size.y);
	bool rectContains = r.contains(point);
	bool extraActive = IsDropActive() || IsSlideActive();
	return rectContains || extraActive;
}

void Panel::DrawQuad(RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
}

void Panel::SetSize(sf::Vector2f &p_size)
{
	size = p_size;
	SetRectTopLeft(quad, size.x, size.y, Vector2f(0, 0));
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

	for (auto it = tabGroups.begin(); it != tabGroups.end(); ++it)
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

	for (auto it = hyperLinks.begin(); it != hyperLinks.end(); ++it)
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

	for (auto it = scrollBars.begin(); it != scrollBars.end(); ++it)
	{
		(*it).second->Deactivate();
	}

	if (extraUpdater != NULL)
		extraUpdater->Deactivate();

	HideToolTip();
}

void Panel::Draw(RenderTarget *target)
{
	sf::View oldView = target->getView();

	sf::View myView = oldView;
	myView.setCenter(myView.getCenter() - Vector2f(pos));

	target->setView(myView);

	DrawQuad(target);

	if (extraUpdater != NULL)
	{
		extraUpdater->Draw(target);
	}



	/*sf::RectangleShape rs;
	rs.setSize( size );
	rs.setFillColor( Color( 83, 102, 188) );
	rs.setPosition( 0, 0 );
	target->draw( rs );*/

	for (auto it = scrollBars.begin(); it != scrollBars.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = labels.begin(); it != labels.end(); ++it)
	{
		//Vector2f labelPos = (*it).second->getPosition();

		//(*it).second->setPosition( pos.x + labelPos.x, pos.y + labelPos.y );
		(*it).second->Draw(target);

		//(*it).second->setPosition( labelPos.x, labelPos.y );
	}

	for (auto it = textBoxes.begin(); it != textBoxes.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = buttons.begin(); it != buttons.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = hyperLinks.begin(); it != hyperLinks.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = checkBoxes.begin(); it != checkBoxes.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = gridSelectors.begin(); it != gridSelectors.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = sliders.begin(); it != sliders.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	for (auto it = tabGroups.begin(); it != tabGroups.end(); ++it)
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

	target->draw(textChooseRectQuads, 4 * textChooseRects.size(), sf::Quads);
	for (auto it = textChooseRects.begin(); it != textChooseRects.end(); ++it)
	{
		(*it)->Draw(target);
	}

	target->draw(lobbyChooseRectQuads, 4 * lobbyChooseRects.size(), sf::Quads);
	for (auto it = lobbyChooseRects.begin(); it != lobbyChooseRects.end(); ++it)
	{
		(*it)->Draw(target);
	}

	Dropdown *expandedDrop = NULL;
	for (auto it = dropdowns.begin(); it != dropdowns.end(); ++it)
	{
		if (!(*it).second->expanded)
		{
			(*it).second->Draw(target);
		}
		else
		{
			expandedDrop = (*it).second;
		}
	}
	if (expandedDrop != NULL)
	{
		expandedDrop->Draw(target);
	}

	for (auto it = menuDropdowns.begin(); it != menuDropdowns.end(); ++it)
	{
		(*it).second->Draw(target);
	}

	if (currToolTip != NULL)
		currToolTip->Draw(target);

	if (extraUpdater != NULL)
	{
		extraUpdater->LateDraw(target);
	}

	target->setView(oldView);
}

bool Panel::IsSliding()
{
	return slideDuration > 0;
}

bool Panel::SendKey(sf::Keyboard::Key k, bool shift)
{
	if (k == Keyboard::Return)
	{
		bool isMultilineTextBoxFocused = false;
		for (map<string, TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it)
		{
			if ((*it).second->focused && (*it).second->maxRows > 1 )
			{
				isMultilineTextBoxFocused = true;
				break;
			}
		}

		if (!isMultilineTextBoxFocused)
		{
			Confirm();
		}
		else
		{
			for (map<string, TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it)
			{
				if ((*it).second->focused)
				{
					(*it).second->SendKey(k, shift);
					SendEvent((*it).second, "modified");
					return true;
				}
			}
		}
		return true;
	}
	else if (k == Keyboard::Escape)
	{
		Cancel();
		return true;
	}

	for (map<string, TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it)
	{
		if ((*it).second->focused)
		{
			(*it).second->SendKey(k, shift);
			SendEvent((*it).second, "modified");
			return true;
		}
	}

	return false;
}