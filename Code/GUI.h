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

struct PanelUpdater
{
	PanelUpdater()
	{

	}
	virtual bool MouseUpdate() = 0;
	virtual void Draw(sf::RenderTarget *target){}
	virtual void Deactivate() {}
private:
};

struct PanelMember
{
	PanelMember(Panel * p)
		:panel(p), locked( false)
	{

	}
	virtual void Draw(sf::RenderTarget *target) = 0;
	virtual bool MouseUpdate() = 0;
	virtual void Deactivate() {}
	//virtual void Lock() { locked = true; }
	//virtual void Unlock() { locked = false; }

	bool locked;
	Panel *panel;
	
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
		I_DECORLIBRARY
	};

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
	virtual void SetSize(float s);
	void UpdateRectDimensions();
	float boxSize;
	sf::Vector2f pos;
	
	virtual void UpdateSprite(int frameUpdate) {}

	
	sf::IntRect bounds;
	int quadIndex;
	sf::Vertex *quad;
	bool active;
	bool show;
	void SetShown(bool s);
	void SetActive(bool a);
	//virtual void Unfocus() {}
	bool focused;

	sf::Color mouseOverColor;
	sf::Color idleColor;

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
		Tileset *ts, int tileIndex, Panel *p );

	void UpdateSprite(int frameUpdate);
	void Draw(sf::RenderTarget *target);
	void SetSize(float s);
	void SetImage(Tileset *ts, int index);
	EditorDecorInfo * CreateDecor();
	sf::Sprite spr;
	Tileset *ts;
	sf::View view;
	int tileIndex;
	std::string decorName;
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


struct CreateEnemyModeUI
{
	CreateEnemyModeUI();
	~CreateEnemyModeUI();

	EnemyVariationSelector *varSelector;
	void ExpandVariation(EnemyChooseRect *ceRect);
	//std::vector<ChooseRect> myRects;
	std::vector<EnemyChooseRect*> allEnemyRects;
	
	std::vector<EnemyChooseRect*> hotbarEnemies;
	ImageChooseRect *librarySearchRect;
	//sf::Vertex *hotbarQuads;
	//sf::Vertex *worldSelectQuads;
	//sf::Vertex *allEnemyQuads;
	//sf::Vertex *allQuads;
	int numAllQuads;
	int activeHotbarSize;
	std::vector<std::vector<EnemyChooseRect*>> 
		libraryEnemiesVec;
	void SetActiveLibraryWorld(int w);
	int activeLibraryWorld;
	std::vector<ImageChooseRect*> worldSelectRects;
	
	void UpdateSprites(int sprUpdateFrames);
	void UpdateHotbarTypes();
	Panel *topbarPanel;
	Panel *libPanel;
	//Panel *topbarPanel;
	//Panel *libraryPanel;
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
	//std::vector<ChooseRect> myRects;
	
	
	void SetActiveLibraryWorld(int w);
	int activeLibraryWorld;
	std::vector<ImageChooseRect> worldSelectRects;

	void UpdateSprites(int sprUpdateFrames);
	void Update(bool mouseDownL,
		bool mouseDownR,
		sf::Vector2i &mousePos);
	void Draw(sf::RenderTarget *target);
	void UpdateHotbarTypes();

	
	
	void SetShown(bool s);
	void SetLibraryShown(bool s);
	void FlipLibraryShown();

	EditSession *edit;

	bool showLibrary;
	bool show;

	Panel *topbarPanel;
	Panel *libPanel;

	std::vector<ImageChooseRect> allImageRects;
	std::vector<std::vector<ImageChooseRect*>>
		libraryImagesVec;
	std::vector<ImageChooseRect> hotbarImages;
	int activeHotbarSize;

	ImageChooseRect *librarySearchRect;

	sf::Vertex *allQuads;
	int numAllQuads;


	
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
	void SendKey( sf::Keyboard::Key k, bool shift );
	void Draw( sf::RenderTarget *target );
	bool MouseUpdate();
	void SetCursorIndex( int index );
	void SetCursorIndex( sf::Vector2i &mousePos );
	void Deactivate();
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
	//~Slider();
	void Draw(sf::RenderTarget *rt);
	bool MouseUpdate();
	void Deactivate();
	float GetCurrFactor(const sf::Vector2i &mousePos);
	int GetCurrValue(float factor);
	int GetCurrX(float factor);
	float circleRad;
	void SetCircle(int x);
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
	~Dropdown();
	void SetOptions(const std::vector<std::string> &options);
	void Draw(sf::RenderTarget *rt);
	bool MouseUpdate();
	void Deactivate();
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
};

struct Panel
{
	Panel( const std::string &name, int width, 
		int height, GUIHandler *handler,
		bool pop = false );
	~Panel();
	void Deactivate();
	void SetColor(sf::Color c);
	//void SetImage(Tileset *ts, int tile);
	void DrawQuad(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *rt);
	bool ContainsPoint(sf::Vector2i &pos);
	/*bool Update(bool mouseDownLeft, bool mouseDownRight,
		int posx, int posy, bool checkContained = false );*/
	bool MouseUpdate();
	void UpdateSprites(int numUpdateFrames = 1);
	void AddSlider(const std::string &name, sf::Vector2i &pos,
		int width, int minValue, int maxValue, int defaultValue);
	void AddDropdown(const std::string &name, sf::Vector2i &pos,
		sf::Vector2i &size, const std::vector<std::string> &p_options,
		int defaultIndex );
	void AddButton( const std::string &name, sf::Vector2i pos, sf::Vector2f size, const std::string &text );
	void AddTextBox( const std::string &name, sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText );
	void AddLabel( const std::string &name, sf::Vector2i pos, int characterHeight, const std::string &text );
	void AddCheckBox( const std::string &name, sf::Vector2i pos, bool startChecked = false );
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
		sf::Vector2f &position, Tileset *ts, int tileIndex);
	void SetPosition(const sf::Vector2i &p_pos);
	void SetCenterPos(const sf::Vector2i &p_pos);
	void HandleEvent(sf::Event ev);

	void SendKey( sf::Keyboard::Key k, bool shift );
	void SendEvent( Button *b, const std::string & e );
	void SendEvent( GridSelector *gs, const std::string & e );
	void SendEvent( TextBox *tb, const std::string & e );
	void SendEvent( CheckBox *cb, const std::string & e );
	sf::Font arial;
	std::string name;
	std::map<std::string, TextBox*> textBoxes;
	std::map<std::string, Button*> buttons;
	std::map<std::string, sf::Text*> labels;
	std::map<std::string, CheckBox*> checkBoxes;
	std::map<std::string, GridSelector*> gridSelectors;
	std::map<std::string, Dropdown*> dropdowns;
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
	
	sf::Vector2f size;
	GUIHandler *handler;
	bool IsPopup();
	bool active;
	PanelUpdater *extraUpdater;
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

struct GUIHandler
{
	virtual void ButtonCallback( Button *b, const std::string & e ) = 0;
	virtual void TextBoxCallback( TextBox *tb, const std::string & e ) = 0;
	virtual void GridSelectorCallback( GridSelector *gs, const std::string & e ) = 0;
	virtual void CheckBoxCallback( CheckBox *cb, const std::string & e ) = 0;
};



#endif