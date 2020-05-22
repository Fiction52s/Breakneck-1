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

struct UIMouse
{
	UIMouse()
	{
		ResetMouse();
	}
	bool IsMouseDownLeft();
	bool IsMouseDownRight();
	bool IsMouseLeftClicked();
	bool IsMouseLeftReleased();
	bool IsMouseRightClicked();
	bool IsMouseRightReleased();
	void Update(bool mouseDownL,
		bool mouseDownR,
		sf::Vector2i &mousePos);
	const sf::Vector2i &GetPos() 
	{ return mousePos; }
	sf::Vector2f GetFloatPos()
	{return sf::Vector2f(mousePos);}
	void ResetMouse();
	bool IsConsumed() { return consumed; }
	void Consume() { consumed = true; }
private:
	sf::Vector2i mousePos;
	bool isMouseDownLeft;
	bool lastMouseDownLeft;

	bool isMouseDownRight;
	bool lastMouseDownRight;
	bool consumed;

};

struct ChooseRectContainer : UIMouseUser
{
	ChooseRectContainer(sf::Vector2i &pos,
		sf::Vector2f &size);
	void Draw(sf::RenderTarget *target);
	sf::Vertex quad[4];
	sf::Vector2f size;
};

struct ChooseRect
{
	enum ChooseRectType
	{
		IMAGE,
		ENEMY,
	};

	enum ChooseRectEventType : int
	{
		E_CLICKED,
		E_FOCUSED,
		E_UNFOCUSED,
		E_RELEASED,
		E_RIGHTCLICKED,
		E_RIGHTRELEASED,
	};

	enum ChooseRectIdentity : int
	{
		I_WORLDCHOOSER,
		I_SEARCHENEMYLIBRARY,
		I_ENEMYHOTBAR,
		I_ENEMYLIBRARY,
		I_SEARCHDECORLIBRARY,
		I_DECORHOTBAR,
		I_DECORLIBRARY
	};

	ChooseRectIdentity rectIdentity;
	ChooseRectType chooseRectType;
	EnemyChooseRect *GetAsEnemyChooseRect();
	ImageChooseRect *GetAsImageChooseRect();

	ChooseRect( ChooseRectIdentity ident, 
		ChooseRectType crType, 
		sf::Vertex *v, UIMouseUser *mouseUser,
	float size, sf::Vector2f &pos );
	void Init();
	sf::Vector2f GetGlobalPos();
	void SetPosition(sf::Vector2f &pos);
	virtual void SetSize(float s);
	void UpdateRectDimensions();
	float boxSize;
	sf::Vector2f pos;
	bool Update();
	virtual void UpdateSprite(int frameUpdate) {}
	virtual void Draw(sf::RenderTarget *target) {}
	sf::IntRect bounds;
	int quadIndex;
	sf::Vertex *quad;
	bool active;
	bool show;
	void SetShown(bool s);
	void SetActive(bool a);
	UIMouseUser *mouseUser;
	bool focused;

	sf::Color mouseOverColor;
	sf::Color idleColor;
};

struct ActorParams;
struct EnemyChooseRect : ChooseRect
{
	EnemyChooseRect( ChooseRectIdentity ident, 
		sf::Vertex *v,
		UIMouseUser *mouseUser, sf::Vector2f &position,
		ActorType * type, 
		int level );
	void UpdateSprite(int frameUpdate);
	void Draw(sf::RenderTarget *target);
	void SetSize(float s);
	void SetType(ActorType *type, int lev);

	ActorType *actorType;
	Enemy *enemy;
	ActorParams *enemyParams;
	sf::View view;
	int level;
};

struct Tileset;
struct EditorDecorInfo;
struct ImageChooseRect : ChooseRect
{
	ImageChooseRect( ChooseRectIdentity ident, 
		sf::Vertex *v,
		UIMouseUser *mouseUser, sf::Vector2f &position,
		Tileset *ts, int tileIndex );

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

struct CreateEnemyModeUI;

struct EnemyVariationSelector : UIMouseUser
{
	EnemyVariationSelector();
	bool Update();
	EnemyChooseRect *centerRect;
	EnemyChooseRect *varRects[6];
	int numVariations;
	void SetType(ActorType *type );
	void Draw(sf::RenderTarget *target);
	void SetPosition(sf::Vector2f &pos);
	sf::Vertex testQuad[4];
	sf::Vertex enemyQuads[28];
	bool show;
};



struct EditSession;
struct CreateEnemyModeUI
{
	CreateEnemyModeUI();
	~CreateEnemyModeUI();

	EnemyVariationSelector *varSelector;
	void ExpandVariation(EnemyChooseRect *ceRect);
	//std::vector<ChooseRect> myRects;
	std::vector<EnemyChooseRect> allEnemyRects;
	
	std::vector<EnemyChooseRect> hotbarEnemies;
	ImageChooseRect *librarySearchRect;
	//sf::Vertex *hotbarQuads;
	//sf::Vertex *worldSelectQuads;
	//sf::Vertex *allEnemyQuads;
	sf::Vertex *allQuads;
	int numAllQuads;
	int activeHotbarSize;
	std::vector<std::vector<EnemyChooseRect*>> 
		libraryEnemiesVec;
	void SetActiveLibraryWorld(int w);
	int activeLibraryWorld;
	std::vector<ImageChooseRect> worldSelectRects;
	
	void UpdateSprites(int sprUpdateFrames);
	void Update(bool mouseDownL,
		bool mouseDownR,
		sf::Vector2i &mousePos);
	void Draw(sf::RenderTarget *target);
	void UpdateHotbarTypes();
	ChooseRectContainer *topbarCont;
	ChooseRectContainer *libCont;
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

	ChooseRectContainer *topbarCont;
	ChooseRectContainer *libCont;

	std::vector<ImageChooseRect> allImageRects;
	std::vector<std::vector<ImageChooseRect*>>
		libraryImagesVec;
	std::vector<ImageChooseRect> hotbarImages;
	int activeHotbarSize;

	ImageChooseRect *librarySearchRect;

	sf::Vertex *allQuads;
	int numAllQuads;


	
};

struct GridSelector
{

	GridSelector(sf::Vector2i pos, int xSize, int ySize, int iconX, int iconY,
		bool displaySelected, bool displayMouseOver, Panel * p);
	~GridSelector();
	void Set(int xi, int yi, sf::Sprite s, const std::string &name);
	void Draw(sf::RenderTarget *target);
	bool Update(bool mouseDown, int posx, int posy);
	//void SetPanelPos(const sf::Vector2i &p_pos);
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
	Panel *owner;
	int selectedX;
	int selectedY;
	int mouseOverX;
	int mouseOverY;
	bool displaySelected;
	bool displayMouseOver;
	//GUIHandler *handler;
};

struct TextBox
{
	TextBox( const std::string &name, int posx, int posy, int width, int lengthLimit, sf::Font &f, Panel *p, const std::string & initialText);
	void SendKey( sf::Keyboard::Key k, bool shift );
	void Draw( sf::RenderTarget *rt );
	bool Update( bool mouseDown, int posx, int posy );
	void SetCursorIndex( int index );
	void SetCursorIndex( sf::Vector2i &mousePos );
	//void SetPanelPos(const sf::Vector2i &p_pos);
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
	Panel *owner;
};

struct Button
{
	Button( const std::string &name, int posx, int posy, int width, int height, sf::Font &f, const std::string & text, Panel *owner );
	void Draw( sf::RenderTarget *rt );
	bool Update( bool mouseDown, int posx, int posy );
	//void SetPanelPos(const sf::Vector2i &p_pos);
	sf::Vector2i pos;
	sf::Vector2f size;
	sf::Text text;
	std::string name;

	int characterHeight;
	bool clickedDown;
	Panel *owner;
};

struct Slider
{
	Slider(const std::string &name, sf::Vector2i &pos,
		int width, sf::Font &f,
		int minValue, int maxValue, int defaultValue, 
		Panel *panel);
	//~Slider();
	void Draw(sf::RenderTarget *rt);
	bool Update();

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
	Panel *panel;
	
};

struct Dropdown
{
	Dropdown(const std::string &name, sf::Vector2i &pos,
		sf::Vector2i &size, sf::Font &f, 
		const std::vector<std::string> &p_options, 
		int defaultIndex, Panel *owner);
	~Dropdown();
	void SetOptions(const std::vector<std::string> &options);
	void Draw(sf::RenderTarget *rt);
	bool Update();

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
	Panel *panel;
	int defaultIndex;
};

struct CheckBox
{
	CheckBox( const std::string &name, int posx, int posy, Panel *owner );
	void Draw( sf::RenderTarget *target );
	bool Update( bool mouseDown, int posx, int posy );
	//void SetPanelPos(const sf::Vector2i &p_pos);
	sf::Vector2i pos;
	std::string name;
	Panel *owner;
	bool clickedDown;
	bool checked;
};

struct Panel
{
	Panel( const std::string &name, int width, int height, GUIHandler *handler );
	~Panel();
	void Draw(sf::RenderTarget *rt);
	bool ContainsPoint(sf::Vector2i &pos);
	bool Update(bool mouseDownLeft, bool mouseDownRight,
		int posx, int posy, bool checkContained = false );
	
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
	void SetPosition(const sf::Vector2i &p_pos);
	

	void SendKey( sf::Keyboard::Key k, bool shift );
	void SendEvent( Button *b, const std::string & e );
	void SendEvent( GridSelector *gs, const std::string & e );
	void SendEvent( TextBox *tb, const std::string & e );
	void SendEvent( CheckBox *cb, const std::string & e );
	sf::Font arial;
	std::string name;
	//TextBox t;
	//TextBox t2;
	//Button b;
	std::map<std::string, TextBox*> textBoxes;
	std::map<std::string, Button*> buttons;
	std::map<std::string, sf::Text*> labels;
	std::map<std::string, CheckBox*> checkBoxes;
	std::map<std::string, GridSelector*> gridSelectors;
	std::map<std::string, Dropdown*> dropdowns;
	std::map<std::string, Slider*> sliders;

	sf::Vector2i pos;
	sf::Vector2f size;
	GUIHandler *handler;

	

	bool IsMouseDownLeft();
	bool IsMouseDownRight();
	bool IsMouseLeftClicked();
	bool IsMouseLeftReleased();
	bool IsMouseRightClicked();
	bool IsMouseRightReleased();
	const sf::Vector2i & GetMousePos();
	
	bool active;

private:
	sf::Vector2i mousePos;
	bool isMouseDownLeft;
	bool lastMouseDownLeft;

	bool isMouseDownRight;
	bool lastMouseDownRight;
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