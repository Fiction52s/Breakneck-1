#ifndef __GUI_H__
#define __GUI_H__
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <list>
#include "Movement.h"

struct Panel;
struct GUIHandler;

struct ActorType;
struct EnemyChooser;
struct Enemy;
struct EnemyChooseRect;
struct ImageChooseRect;

struct ActorParams;
struct Tileset;
struct EditorDecorInfo;
struct CreateEnemyModeUI;
struct EditSession;

struct Button;
struct GridSelector;
struct Button;
struct CheckBox;
struct Slider;
struct Dropdown;
struct TextBox;
struct ChooseRect;
struct GateInfo;
struct MenuDropdown;
struct TerrainRail;

struct Brush;

struct UIMouse
{
	static UIMouse &GetInstance()
	{
		static UIMouse instance;
		return instance;
	}
	
	UIMouse(UIMouse const&) = delete;
	void operator=(UIMouse const&) = delete;

	bool IsMouseDownLeft();
	bool IsMouseDownRight();
	bool IsMouseLeftClicked();
	bool IsMouseLeftReleased();
	bool IsMouseRightClicked();
	bool IsMouseRightReleased();
	void Update(sf::Vector2i &mousePos);
	const sf::Vector2i &GetPos() 
	{ return mousePos; }
	sf::Vector2f GetFloatPos()
	{return sf::Vector2f(mousePos);}
	void ResetMouse();
	bool IsConsumed() { return consumed; }
	void Consume() { consumed = true; }
private:
	UIMouse();
	
	sf::Vector2i mousePos;
	bool isMouseDownLeft;
	bool lastMouseDownLeft;

	bool isMouseDownRight;
	bool lastMouseDownRight;
	bool consumed;

};

#define MOUSE UIMouse::GetInstance()

struct GUIHandler
{
	virtual void ButtonCallback(Button *b, const std::string & e) {}
	virtual void TextBoxCallback(TextBox *tb, const std::string & e) {}
	virtual void GridSelectorCallback(GridSelector *gs, const std::string & e) {}
	virtual void CheckBoxCallback(CheckBox *cb, const std::string & e) {}
	virtual void SliderCallback(Slider *slider ) {}
	virtual void DropdownCallback(Dropdown *dropdown, const std::string & e) {}
	virtual void PanelCallback(Panel *p, const std::string & e) {}
	virtual void ChooseRectEvent(ChooseRect *cr, int eventType) {}
	virtual void MenuDropdownCallback(MenuDropdown *menuDrop, const std::string & e) {}

};

struct ToolTip
{
	ToolTip(const std::string &str);
	sf::Text toolTipText;
	sf::Vertex quad[4];
	void SetFromMousePos(const sf::Vector2i &pos);
	void Draw(sf::RenderTarget *target);
};

struct PanelUpdater
{
	PanelUpdater()
	{

	}
	virtual bool MouseUpdate() = 0;
	virtual void Draw(sf::RenderTarget *target){}
	virtual void LateDraw(sf::RenderTarget *target) {}
	virtual void Deactivate() {}
	virtual void MouseScroll(int delta) {}
private:
};

struct PanelMember
{
	PanelMember(Panel * p)
		:panel(p), locked(false),hidden(false),
		toolTip(NULL)
	{

	}
	void HideMember() { hidden = true; }
	void ShowMember() { hidden = false; }
	virtual ~PanelMember();
	void SetToolTip(const std::string &str);
	virtual void Draw(sf::RenderTarget *target) = 0;
	virtual bool MouseUpdate() = 0;
	virtual void Deactivate() {}
	void UpdateToolTip(bool contains);

	//void UpdateToolTip();
	//void ResetToolTip();
	//int toolTipCounter;
	//int toolTipThresh;

	sf::Vector2i lastMouse;
	//virtual void Lock() { locked = true; }
	//virtual void Unlock() { locked = false; }

	bool locked;
	bool hidden;
	Panel *panel;
	ToolTip *toolTip;
};

struct ChooseRect : PanelMember
{
	enum ChooseRectType
	{
		IMAGE,
		ENEMY,
	};

	enum ChooseRectEventType : int
	{
		E_LEFTCLICKED,
		E_FOCUSED,
		E_UNFOCUSED,
		E_LEFTRELEASED,
		E_RIGHTCLICKED,
		E_RIGHTRELEASED,
	};

	enum ChooseRectIdentity : int
	{
		I_WORLDCHOOSER,
		I_SEARCHENEMYLIBRARY,
		I_ENEMYHOTBAR,
		I_ENEMYLIBRARY,
		I_CHANGEENEMYVAR,
		I_SEARCHDECORLIBRARY,
		I_DECORHOTBAR,
		I_DECORLIBRARY,
		I_TERRAINSEARCH,
		I_TERRAINLIBRARY,
		I_GATESEARCH,
		I_SHARDLIBRARY,
		I_GATEBOSSLIBRARY,
		I_GATEPICKUPLIBRARY,
		I_RAILSEARCH,
		I_RAILLIBRARY,
		I_FILESELECT,
		I_BRUSHHOTBAR,
		I_ADVENTURECREATOR_WORLD_SEARCH,
		I_ADVENTURECREATOR_WORLD,
		I_ADVENTURECREATOR_SECTOR,
		I_ADVENTURECREATOR_MAP,
		I_BACKGROUNDLIBRARY,
	};

	sf::Text nameText;
	void SetName(const std::string &name);
	ChooseRectIdentity rectIdentity;
	ChooseRectType chooseRectType;
	EnemyChooseRect *GetAsEnemyChooseRect();
	ImageChooseRect *GetAsImageChooseRect();
	bool ContainsPoint(sf::Vector2i &mousePos);
	ChooseRect( ChooseRectIdentity ident, 
		ChooseRectType crType, 
		sf::Vertex *v,
	float size, sf::Vector2f &pos,
		Panel *panel );
	bool MouseUpdate();
	void SetCircleMode(int radius);
	void SetRectMode();
	virtual void Draw(sf::RenderTarget *target) {}

	void Init();
	virtual void Deactivate();
	sf::Vector2f GetGlobalPos();
	sf::Vector2f GetGlobalCenterPos();
	void SetPosition(sf::Vector2f &pos);
	void UpdateTextPosition();
	virtual void SetSize(float s);
	void UpdateRectDimensions();
	float boxSize;
	sf::Vector2f pos;
	
	virtual void UpdateSprite(int frameUpdate) {}
	virtual void UpdatePanelPos() {}
	
	sf::IntRect bounds;
	int quadIndex;
	sf::Vertex *quad;
	bool active;
	bool show;
	void SetShown(bool s);
	void SetActive(bool a);
	//virtual void Unfocus() {}
	bool focused;
	void SetInfo(void *inf);

	sf::Color mouseOverColor;
	sf::Color idleColor;
	void *info;
	bool circleMode;
	int circleRadius;
};

struct EnemyChooseRect : ChooseRect
{
	EnemyChooseRect( ChooseRectIdentity ident, 
		sf::Vertex *v, sf::Vector2f &position,
		ActorType * type, 
		int level, Panel *p );
	void UpdateSprite(int frameUpdate);
	void Draw(sf::RenderTarget *target);
	void SetSize(float s);
	void SetType(ActorType *type, int lev);
	void UpdatePanelPos();
	//void Unfocus();
	ActorType *actorType;
	Enemy *enemy;
	ActorParams *enemyParams;
	sf::View view;
	int level;
};

struct ImageChooseRect : ChooseRect
{
	ImageChooseRect( ChooseRectIdentity ident, 
		sf::Vertex *v, sf::Vector2f &position,
		Tileset *ts, int tileIndex, int boxSize, Panel *p );
	ImageChooseRect(ChooseRectIdentity ident,
		sf::Vertex *v, sf::Vector2f &position,
		Tileset *ts, const sf::IntRect &subRect, int boxSize, 
		Panel *p);

	void UpdatePanelPos();
	void UpdateSprite(int frameUpdate);
	void Draw(sf::RenderTarget *target);
	void SetSize(float s);
	void SetImage(Tileset *ts, int index);
	void SetImage(Tileset *ts, const sf::IntRect &subRect);

	
	
	EditorDecorInfo * CreateDecor();
	sf::Sprite spr;
	Tileset *ts;
	sf::View view;
	int tileIndex;
	std::string decorName;
};

struct BrushChooseRect : ChooseRect
{
	BrushChooseRect(ChooseRectIdentity ident,
		sf::Vertex *v, sf::Vector2f &position, 
		int boxSize, Panel *p);
	void SetBrush(Brush *b);
	Brush *myBrush;
};

struct EnemyVariationSelector : PanelUpdater
{
	EnemyVariationSelector(bool createMode);
	~EnemyVariationSelector();
	bool MouseUpdate();
	EnemyChooseRect *centerRect;
	EnemyChooseRect *varRects[6];
	void Deactivate();
	int numVariations;
	void SetType(ActorType *type );
	void Draw(sf::RenderTarget *target);
	void SetPosition(sf::Vector2f &pos);

	sf::Sprite orbSpr;
	//sf::Vertex testQuad[4];
	sf::Vertex enemyQuads[28];
	//bool show;
	Panel *panel;
	EditSession *edit;
	bool createMode;
};

struct PanelSlider : PanelUpdater
{
	PanelSlider(Panel *p,
		sf::Vector2i &origPos, 
		sf::Vector2i &destPos);
	bool MouseUpdate();
	void Deactivate();
	Panel *panel;
	bool slid;
	int outFrame;
	sf::Vector2i origPos;
	sf::Vector2i destPos;
	CubicBezier bez;
	int normalDuration;
};

struct GridSelector : PanelMember
{
	GridSelector(sf::Vector2i pos, int xSize, int ySize, int iconX, int iconY,
		bool displaySelected, bool displayMouseOver, Panel * p);
	~GridSelector();
	void Set(int xi, int yi, sf::Sprite s, const std::string &name);

	void Draw(sf::RenderTarget *target);
	bool MouseUpdate();
	void Deactivate();
	int tileSizeX;
	int tileSizeY;
	int xSize;
	int ySize;
	sf::Sprite ** icons;
	std::string ** names;
	bool active;
	int focusX;
	int focusY;
	sf::Vector2i pos;
	int selectedX;
	int selectedY;
	int mouseOverX;
	int mouseOverY;
	bool displaySelected;
	bool displayMouseOver;
};

struct TextBox : PanelMember
{
	TextBox( const std::string &name, int posx, int posy, int width, int lengthLimit, sf::Font &f, Panel *p, const std::string & initialText);
	void SetString(const std::string &str);
	std::string GetString();
	void SendKey( sf::Keyboard::Key k, bool shift );
	void Draw( sf::RenderTarget *target );
	bool MouseUpdate();
	void SetCursorIndex( int index );
	void SetCursorIndex( sf::Vector2i &mousePos );
	void Deactivate();
	void SetNumbersOnly(bool b);
	sf::Vector2i pos;
	int width;
	std::string name;
	int maxLength;
	sf::Text text;
	int cursorIndex;
	sf::Text cursor;
	int characterHeight;
	int verticalBorder;
	int leftBorder;
	bool clickedDown;
	bool focused;
	sf::Vector2i size;
	bool numbersOnly;
	
};

struct Button : PanelMember
{
	Button( const std::string &name, int posx, int posy, int width, int height, sf::Font &f, const std::string & text, Panel *panel);
	void Draw( sf::RenderTarget *target );
	bool MouseUpdate();
	void Deactivate();
	sf::Vector2i pos;
	sf::Vector2f size;
	sf::Text text;
	std::string name;

	int characterHeight;
	bool clickedDown;
};

struct Slider : PanelMember
{
	Slider(const std::string &name, sf::Vector2i &pos,
		int width, sf::Font &f,
		int minValue, int maxValue, int defaultValue,
		Panel *panel);
	void SetFloatMode(
		float minDec, float step);

	bool floatSlider;
	float minDec;
	float step;

	//~Slider();
	void Draw(sf::RenderTarget *rt);
	bool MouseUpdate();
	void Deactivate();
	float GetCurrFactor(const sf::Vector2i &mousePos);
	int CalcCurrValue(float factor);
	int GetCurrX(float factor);
	float circleRad;
	void SetCircle(int x);
	int GetCurrValue();
	void SetCurrValue(int v);
	float GetCurrValueF();
	void SetCurrValueF(float v);
	//int GetCurrValue(const sf::Vector2i &mousePos);

	sf::Vector2i pos;
	sf::Vector2f size;
	std::string name;

	bool IsPointOnRect(sf::Vector2f &point);
	void SetToFactor(float factor);
	sf::Text displayText;


	sf::Font &myFont;

	sf::CircleShape selectCircle;

	int minValue;
	int maxValue;
	int defaultValue;
	int currValue;

	int numOptions;
	sf::Vertex mainRect[4];
	sf::Vertex underRect[4];
	sf::Vertex displayRect[4];
	int characterHeight;
	bool clickedDown;
};

struct Dropdown : PanelMember
{
	Dropdown(const std::string &name, sf::Vector2i &pos,
		sf::Vector2i &size, sf::Font &f, 
		const std::vector<std::string> &p_options, 
		int defaultIndex, Panel *panel);
	bool ContainsMouse();
	~Dropdown();
	int GetIndex(const std::string &s);
	void SetOptions(const std::vector<std::string> &options);
	void Draw(sf::RenderTarget *rt);
	bool MouseUpdate();
	void Deactivate();
	void SetSelectedIndex(int ind);
	const std::string &GetSelectedText();
	sf::Vector2i pos;
	sf::Vector2f size;
	std::string name;
	std::vector<std::string> options;
	std::vector<sf::Text> optionText;
	sf::Font &myFont;
	bool IsMouseOnOption(int i, sf::Vector2f &point);

	bool expanded;
	int selectedIndex;
	sf::Text selectedText;

	int numOptions;
	sf::Vertex mainRect[4];
	sf::Vertex *dropdownRects;
	int characterHeight;
	bool clickedDown;
	int defaultIndex;
};

struct MenuDropdown : PanelMember
{
	MenuDropdown(const std::string &name, sf::Vector2i &pos,
		sf::Vector2i &size, int optionWidth, sf::Font &f,
		const std::vector<std::string> &p_options, 
		Panel *panel);
	~MenuDropdown();
	void SetOptions(const std::vector<std::string> &options);
	void Draw(sf::RenderTarget *rt);
	bool MouseUpdate();
	void Deactivate();
	void SetSelectedIndex(int ind);
	sf::Vector2i pos;
	sf::Vector2f size;
	std::string name;
	std::vector<std::string> options;
	std::vector<sf::Text> optionText;
	sf::Font &myFont;
	bool IsMouseOnOption(int i, sf::Vector2f &point);
	void SetExpanded(bool exp, bool onMain);

	sf::Color idleColor;
	sf::Color mouseOverColor;
	sf::Color expandedColor;
	int optionWidth;
	bool expanded;
	int selectedIndex;
	sf::Text menuText;

	int numOptions;
	sf::Vertex mainRect[4];
	sf::Vertex *dropdownRects;
	int characterHeight;
	bool clickedDown;
};

struct CheckBox : PanelMember
{
	CheckBox( const std::string &name, int posx, int posy, Panel *panel );
	void Draw( sf::RenderTarget *target );
	bool MouseUpdate();
	void Deactivate();
	void SetLockedStatus(bool check, bool lock);
	//void Lock();
	sf::Vector2i pos;
	std::string name;
	bool clickedDown;
	bool checked;
	sf::Vector2i size;
};

struct Panel
{
	Panel( const std::string &name, int width, 
		int height, GUIHandler *handler,
		bool pop = false );
	virtual ~Panel();

	void SetAutoSpacing(bool spacingX,
		bool spacingY = false, sf::Vector2i start = sf::Vector2i(),
		sf::Vector2i defaultExtra = sf::Vector2i() );
	void StopAutoSpacing();
	void PauseAutoSpacing();
	void UnpauseAutoSpacing();
	sf::Vector2<bool> autoSpace;
	sf::Vector2i autoStart;
	sf::Vector2i defaultExtra;
	bool autoSpacePaused;
	void AddAutoSpaceX(int x);
	void AddAutoSpaceY(int y);
	void SetSize(sf::Vector2f &size);

	bool hasFocusedTextbox;
	Button *confirmButton;
	void SetConfirmButton(Button *b);

	Button *cancelButton;
	void SetCancelButton(Button *b);

	PanelMember *focusedMember;
	void SetFocusedMember(PanelMember*pm);
	void RemoveAsFocusedMember(PanelMember *pm);

	void UpdateSlide( int numUpdateFrames );
	
	bool IsDropActive();
	bool IsSlideActive();
	bool IsSliding();
	void Deactivate();
	void SetColor(sf::Color c);
	//void SetImage(Tileset *ts, int tile);
	void DrawQuad(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *rt);
	bool ContainsPoint(sf::Vector2i &pos);
	void ShowToolTip(ToolTip *tt);
	void HideToolTip();
	ToolTip *currToolTip;
	/*bool Update(bool mouseDownLeft, bool mouseDownRight,
		int posx, int posy, bool checkContained = false );*/
	bool MouseUpdate();
	void UpdateSprites(int numUpdateFrames = 1);
	Slider * AddSlider(const std::string &name, sf::Vector2i &pos,
		int width, int minValue, int maxValue, int defaultValue);
	Slider * AddFloatSlider(const std::string &name, sf::Vector2i &pos,
		int width, float minValue, float maxValue, float defaultValue, float step);
	Dropdown * AddDropdown(const std::string &name, sf::Vector2i &pos,
		sf::Vector2i &size, const std::vector<std::string> &p_options,
		int defaultIndex );
	MenuDropdown * AddMenuDropdown(const std::string &name, sf::Vector2i &pos,
		sf::Vector2i &size, int optionWidth,
		const std::vector<std::string> &p_options );
	Button * AddButton( const std::string &name, sf::Vector2i pos, sf::Vector2f size, const std::string &text );
	TextBox * AddTextBox( const std::string &name, sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText );
	sf::Text * AddLabel( const std::string &name, sf::Vector2i pos, int characterHeight, const std::string &text );
	CheckBox * AddCheckBox( const std::string &name, sf::Vector2i pos, bool startChecked = false );
	CheckBox * AddLabeledCheckBox(const std::string &name, sf::Vector2i pos, const std::string &labelText, bool startChecked = false);
	TextBox * AddLabeledTextBox(const std::string &name, sf::Vector2i pos, int textBoxWidth,
		int textBoxLengthLimit, const std::string &initialText, const std::string &labelText );
	GridSelector * AddGridSelector( const std::string &name, sf::Vector2i pos, 
		int sizex, int sizey, 
		int tilesizex, int tilesizey,
		bool displaySelected,
		bool displayMouseOver );

	EnemyChooseRect * AddEnemyRect(
		ChooseRect::ChooseRectIdentity ident,
		sf::Vector2f &position,
		ActorType * type,
		int level);
	ImageChooseRect * AddImageRect(ChooseRect::ChooseRectIdentity ident,
		sf::Vector2f &position, Tileset *ts, int tileIndex, int bSize = 100 );
	ImageChooseRect * AddImageRect(ChooseRect::ChooseRectIdentity ident,
		sf::Vector2f &position, Tileset *ts, const sf::IntRect &rect, int bSize = 100 );
	void SetPosition(const sf::Vector2i &p_pos);
	void SetCenterPos(const sf::Vector2i &p_pos);
	bool HandleEvent(sf::Event ev);

	bool SendKey( sf::Keyboard::Key k, bool shift );
	void SendEvent( Button *b, const std::string & e );
	void SendEvent( GridSelector *gs, const std::string & e );
	void SendEvent( TextBox *tb, const std::string & e );
	void SendEvent( CheckBox *cb, const std::string & e );
	void SendEvent(Dropdown *drop, const std::string & e);
	void SendEvent(Slider *slide );
	void SendEvent(MenuDropdown *menuDrop, const std::string & e);
	sf::Font arial;
	std::string name;
	std::map<std::string, TextBox*> textBoxes;
	std::map<std::string, Button*> buttons;
	std::map<std::string, sf::Text*> labels;
	std::map<std::string, CheckBox*> checkBoxes;
	std::map<std::string, GridSelector*> gridSelectors;
	std::map<std::string, Dropdown*> dropdowns;
	std::map<std::string, MenuDropdown*> menuDropdowns;
	std::map<std::string, Slider*> sliders;
	void ReserveEnemyRects(int num);
	void ReserveImageRects(int num);
	int reservedEnemyRectCount;
	int reservedImageRectCount;
	std::vector<EnemyChooseRect*> enemyChooseRects;
	std::vector<ImageChooseRect*> imageChooseRects;
	sf::Vertex *enemyChooseRectQuads;
	sf::Vertex *imageChooseRectQuads;

	const sf::Vector2i &GetMousePos();

	sf::Vector2i pos;

	sf::Vector2i slideStart;
	sf::Vector2i slideEnd;
	CubicBezier slideBez;
	int slideDuration;
	int slideFrame;
	void Slide(sf::Vector2i &dest, CubicBezier &bez,
		int duration );

	
	sf::Vector2f size;
	GUIHandler *handler;
	bool IsPopup();
	bool active;
	PanelUpdater *extraUpdater;


	int toolTipCounter;
	int toolTipThresh;
	sf::Vector2i lastMouse;
	void UpdateToolTip( int frames );
	bool ToolTipCanBeTurnedOn();
private:
	sf::Vertex quad[4];
	bool popup;
	sf::Vector2i mousePos;
};

enum ErrorType : int
{
	ERR_NO_ERROR,
	ERR_PLAYER_INTERSECTS_POLY,
	ERR_POLY_INTERSECTS_POLY,
	ERR_POLY_CONTAINS_POLY,
	ERR_POLY_INTERSECTS_ENEMY,
	ERR_POLY_INTERSECTS_GATE,
	ERR_POLY_INCORRECT_WINDING_INVERSE,
	ERR_POLY_INCORRECT_WINDING,
	ERR_POINTS_MOVE_TOO_CLOSE,
	ERR_POLY_HAS_SLIVER,
	ERR_POLY_INTERSECTS_ITSELF,
	ERR_ENEMY_NEEDS_GROUND,
	ERR_ENEMY_NEEDS_RAIL,
	ERR_ENEMY_NEEDS_GROUND_OR_RAIL,
	ERR_GATE_INTERSECTS_POLY,
	ERR_GATE_SAME_POINT,
	ERR_GATE_CREATES_SLIVER,
	ERR_GATE_INTERSECTS_GATE,
	ERR_GATE_POINT_ALREADY_OCCUPIED,
	ERR_GATE_INSTERSECTS_ENEMY,
	ERR_GATE_NEEDS_BOTH_POINTS,

	ERR_POLY_NEEDS_THREE_OR_MORE_POINTS,
	ERR_POINTS_TOO_CLOSE_TO_SEG_IN_PROGRESS,
	ERR_POINT_TO_CLOSE_TO_OTHERS,
	ERR_LINES_INTERSECT_IN_PROGRESS,

	ERR_SELECTED_TERRAIN_MULTIPLE_LAYERS,
	ERR_INVERSE_CANT_INTERSECT_NORMAL_POLYS,
	ERR_CANT_DELETE_WHILE_MOVING,
	//ERR_GATE_INTERSECTS_
	ERR_Count
};

struct ErrorBar
{
	ErrorBar(sf::Font &f);
	sf::Text errorText;
	sf::RectangleShape rect;
	void Draw(sf::RenderTarget *target);
	void SetShown(bool show);
	void ShowError(ErrorType e);
	void SetText(const std::string &msg);
	void ShowText(const std::string &msg);
	bool show;

	void CreateErrorTable();
	std::vector<std::string> errorStringVec;
	sf::Font *myFont;

};

struct ConfirmPopup : GUIHandler
{
	enum ConfirmType
	{
		OVERWRITE_MAP,
		SAVE_CURRENT,
		SAVE_CURRENT_EXIT,
	};

	ConfirmType type;
	Panel *panel;
	EditSession *edit;

	


	ConfirmPopup();
	~ConfirmPopup();
	void Pop(ConfirmType ct);
	void ButtonCallback(Button *b,
		const std::string &e);
};

struct MapOptionsUI : GUIHandler
{
	Panel *mapOptionsPanel;
	Panel *bgOptionsPanel;
	std::string *bgNameArr;
	EditSession *edit;
	Button *bgButton;
	Button *okButton;
	TextBox* drainTextbox;

	MapOptionsUI();
	~MapOptionsUI();
	void CloseMapOptionsPopup();
	void OpenMapOptionsPopup();
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

struct GeneralUI : GUIHandler
{
	GeneralUI();
	~GeneralUI();

	enum FileOptions
	{
		NEW_MAP,
		OPEN_MAP,
		SAVE_MAP,
		SAVE_MAP_AS,
		EDIT_ADVENTURE,
		CREATE_ADVENTURE,
		EXIT_EDITOR,
	};

	enum EditOptions
	{
		UNDO,
		REDO,
		EditOptions_Count
	};

	enum ModeOptions
	{
		CREATE_TERRAIN,
		EDIT,
		CREATE_ENEMIES,
		CREATE_GATES,
		CREATE_IMAGES,
		MAP_OPTIONS,
		ModeOptions_Count
	};

	Panel *mainPanel;
	MenuDropdown *fileDropdown;
	MenuDropdown *editDropdown;
	MenuDropdown *modeDropdown;
	EditSession *edit;

	void Draw(sf::RenderTarget *target);
	int height;

	void MenuDropdownCallback(MenuDropdown *menuDrop, const std::string & e);

};

struct EditModeUI : GUIHandler
{
	EditModeUI();
	~EditModeUI();

	void SetEnemyPanel(ActorParams * ap);
	void SetCurrRailPanel(TerrainRail *rail);
	void SaveKinOptions();
	void CreateKinOptionsPanel();
	void AddKinOption(
		const std::string &text, 
		const std::string &toolTipText, 
		int upgradeIndex);
	Panel *currEnemyPanel;
	ActorParams *currParams;
	TerrainRail *currRail;

	Panel *mapOptionsPanel;
	Panel *bgOptionsPanel;
	Panel *nameBrushPanel;
	


	int labelCharHeight;
	int labelExtraSpacing;
	int labelExtraY;
	sf::Vector2i labelExtra;

	void ToggleKinOptionsPanel();
	Panel *kinOptionsPanel;

	std::vector<CheckBox*> kinCheckboxes;

	void SetShown(bool s);
	bool IsGridOn();
	void FlipGrid();
	bool IsEditPointsOn();
	void FlipEditPoints();
	bool IsShowGrassOn();
	void FlipShowGrass();
	bool IsMoveOn();
	void FlipMove();

	void SetGridSize(int gs);
	void ExpandTerrainLibrary(int layer);
	void ExpandShardLibrary();
	void ChooseMatType(ImageChooseRect *icRect);
	void ButtonCallback(Button *b, const std::string & e);
	void TextBoxCallback(TextBox *tb, const std::string & e);
	void GridSelectorCallback(GridSelector *gs, const std::string & e);
	void CheckBoxCallback(CheckBox *cb, const std::string & e);
	void SliderCallback(Slider *slider);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
	void ChooseRectEvent(ChooseRect *cr, int eventType);

	std::string GetLayerShowName(int layer);
	std::string GetLayerLockedName(int layer);
	bool IsLayerShowing(int layer);
	bool IsLayerLocked(int layer);
	bool IsLayerActionable(int layer);
	void UpdateLayerShow(int layer, bool show);
	void UpdateLayerLock(int layer, bool lock);

	void CreateLayerPanel();
	void AddLayerToPanel(
		const std::string &name,
		int currLayerIndex, int startY);

	int terrainGridSize;
	Panel *matTypePanel;
	Panel *shardTypePanel;
	//std::vector<ImageChooseRect*> *matTypeRects;
	sf::Vector2i matPanelPos;
	sf::Vector2i shardPanelPos;

	std::map<int, std::string> layerMap;
	std::map<std::string, int> reverseLayerMap;

	std::map<int, int> terrainEditLayerMap;

	//search for layer_ get terrainlayer_
	std::map<int, int> terrainEditLayerReverseMap;

	bool show;
	EditSession *edit;
	Panel *mainPanel;

	Panel *layerPanel;
	PanelSlider *lpSlider;

	CheckBox *gridCheckbox;
	CheckBox *editPointsCheckbox;
	CheckBox *moveToolCheckbox;
	CheckBox *showGrassCheckbox;

	TextBox *gridSizeTextbox;
	Button *deleteBrushButton;
	Button *transformBrushButton;
	Button *copyBrushButton;
	Button *pasteBrushButton;
};

struct CreateGatesModeUI : GUIHandler
{
	CreateGatesModeUI();
	~CreateGatesModeUI();
	void ExpandLibrary();
	void SetShown(bool s);
	void ExpandShardLibrary();
	void ExpandBossLibrary();
	void ExpandPickupLibrary();
	void ChooseShardType(ImageChooseRect *icRect);
	void ChooseBossGateType(ImageChooseRect *icRect);
	void ChoosePickupGateType(ImageChooseRect *icRect);
	//void SetCurrMatType( )
	void ButtonCallback(Button *b, const std::string & e);
	void TextBoxCallback(TextBox *tb, const std::string & e);
	void GridSelectorCallback(GridSelector *gs, const std::string & e);
	void CheckBoxCallback(CheckBox *cb, const std::string & e);
	void SliderCallback(Slider *slider);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void SetGateInfo(GateInfo *gi);
	void SetFromGateInfo(GateInfo *gi);
	void SetEditGate(GateInfo *gi);
	void CompleteEditingGate();
	void Draw(sf::RenderTarget *target);
	sf::RectangleShape modifyGateRect;
	GateInfo *origModifyGate;
	GateInfo *modifyGate;
	void SetShard(int world, int localIndex);

	int currShardWorld;
	int currShardLocalIndex;

	sf::Vector2f currGateTypeRectPos;
	sf::Vector2i popupPanelPos;

	int GetGateCategory();
	//void CreateShardTypePanel();
	void CreateBossGateTypePanel();
	void CreatePickupGateTypePanel();
	Tileset *ts_shards[7];

	Tileset *ts_gateCategories;
	Tileset *ts_bossGateTypes;
	Tileset *ts_pickupGateTypes;
	bool show;
	EditSession *edit;
	Panel *mainPanel;
	//Panel *gateTypePanel;
	Panel *shardGateTypePanel;
	Panel *pickupGateTypePanel;
	Panel *bossGateTypePanel;

	std::vector<int> currVariation;

	void UpdateCategoryDropdownType();

	TextBox *numToOpenTextbox;
	int numToOpen;
	Button *deleteGateButton;
	//Button *OKGateButton;


	Dropdown *gateCategoryDropdown;

	std::vector<ImageChooseRect*> currGateTypeRects;

	std::vector<ImageChooseRect*> *shardGateTypeRects;
	std::vector<ImageChooseRect*> pickupGateTypeRects;
	std::vector<ImageChooseRect*> bossGateTypeRects;
	int gateGridSize;
};

struct CreateRailModeUI : GUIHandler
{
	CreateRailModeUI();
	~CreateRailModeUI();
	void SetShown(bool s);
	bool IsGridOn();
	void FlipGrid();
	bool IsSnapPointsOn();
	void FlipSnapPoints();
	void SetGridSize(int gs);
	void ExpandLibrary();
	void ChoosePhysicalType(ImageChooseRect *icRect);
	void ChooseEnemyType(EnemyChooseRect *ecRect);
	//void SetCurrMatType( )
	void ButtonCallback(Button *b, const std::string & e);
	void TextBoxCallback(TextBox *tb, const std::string & e);
	void GridSelectorCallback(GridSelector *gs, const std::string & e);
	void CheckBoxCallback(CheckBox *cb, const std::string & e);
	void SliderCallback(Slider *slider);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	int GetRailCategory();
	void ExpandPhysicalLibrary();
	void ExpandEnemyLibrary();
	int GetRailType();

	int physTypeIndex;
	int enemyTypeIndex;


	enum RailCategories
	{
		PHYSICAL,
		ENEMY
	};

	enum RailPhysTypes
	{
		NORMAL,
		ACCELERATOR,
		FRICTION,
		PhysTypes_Count
	};

	bool show;
	EditSession *edit;

	int railTypeGridSize;

	Panel *mainPanel;
	Panel *enemyPanel;
	Panel *physicalPanel;


	CheckBox *gridCheckbox;
	CheckBox *snapPointsCheckbox;
	TextBox *gridSizeTextbox;
	Button *completeButton;
	Button *removePointButton;
	Button *removeAllPointsButton;

	Dropdown *railCategoryDropdown;

	sf::Vector2i matPanelPos;

	ImageChooseRect *currPhysicalTypeRect;
	EnemyChooseRect *currEnemyTypeRect;

	std::vector<ImageChooseRect*> physRects;
	std::vector<EnemyChooseRect*> enemyRects;
};

struct CreateTerrainModeUI : GUIHandler
{
	CreateTerrainModeUI();
	~CreateTerrainModeUI();
	void SetShown(bool s);
	bool IsGridOn();
	void FlipGrid();
	bool IsSnapPointsOn();
	void FlipSnapPoints();
	void SetGridSize(int gs);
	void ExpandTerrainLibrary();
	void ChooseMatType(ImageChooseRect *icRect);
	void ButtonCallback(Button *b, const std::string & e);
	void TextBoxCallback(TextBox *tb, const std::string & e);
	void GridSelectorCallback(GridSelector *gs, const std::string & e);
	void CheckBoxCallback(CheckBox *cb, const std::string & e);
	void SliderCallback(Slider *slider);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	int GetTerrainLayer();
	int GetCurrDrawTool();
	void SetDrawTool(int t);
	int GetCurrTerrainTool();
	void SetTerrainTool(int t);
	void SetTempTerrainTool(int t);
	void RevertTerrainTool();

	int realTerrainTool;
	int terrainGridSize;

	bool show;
	EditSession *edit;

	Panel *matTypePanel;
	Panel *mainPanel;
	//Panel *nameBrushPanel;
	Panel *brushHotbar;
	void UpdateBrushHotbar();
	//std::list<Brush*> recentBrushes;

	std::vector<ImageChooseRect*> brushHotbarImages;

	CheckBox *gridCheckbox;
	CheckBox *snapPointsCheckbox;
	TextBox *gridSizeTextbox;
	Button *completeButton;
	Button *removePointButton;
	Button *removeAllPointsButton;
	Dropdown *terrainActionDropdown;
	Dropdown *terrainLayerDropdown;
	Dropdown *drawModeDropdown;

	sf::Vector2i matPanelPos;

	std::vector<ImageChooseRect*> currMatRects;
};

struct CreateEnemyModeUI
{
	CreateEnemyModeUI();
	~CreateEnemyModeUI();

	EnemyVariationSelector *varSelector;
	void ExpandVariation(EnemyChooseRect *ceRect);
	std::vector<EnemyChooseRect*> allEnemyRects;

	std::vector<EnemyChooseRect*> hotbarEnemies;
	ImageChooseRect *librarySearchRect;
	int activeHotbarSize;
	std::vector<std::vector<EnemyChooseRect*>>
		libraryEnemiesVec;
	void SetActiveLibraryWorld(int w);
	int activeLibraryWorld;
	std::vector<ImageChooseRect*> worldSelectRects;

	void UpdateHotbarTypes();
	Panel *topbarPanel;
	Panel *libPanel;
	EditSession *edit;
	void SetShown(bool s);
	void SetLibraryShown(bool s);
	void FlipLibraryShown();
	bool showLibrary;
	bool show;
};

struct CreateDecorModeUI
{
	CreateDecorModeUI();
	~CreateDecorModeUI();

	void SetActiveLibraryWorld(int w);
	int activeLibraryWorld;
	std::vector<ImageChooseRect*> worldSelectRects;
	void UpdateHotbarTypes();
	void SetShown(bool s);
	void SetLibraryShown(bool s);
	void FlipLibraryShown();

	EditSession *edit;

	bool showLibrary;
	bool show;

	Panel *topbarPanel;
	Panel *libPanel;
	std::vector<ImageChooseRect*> allImageRects;
	std::vector<std::vector<ImageChooseRect*>>
		libraryImagesVec;
	std::vector<ImageChooseRect*> hotbarImages;
	int activeHotbarSize;

	ImageChooseRect *librarySearchRect;
};

#endif