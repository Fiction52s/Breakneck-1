#ifndef __GUI_H__
#define __GUI_H__
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <list>
#include "Movement.h"

struct Panel;
struct GUIHandler;


struct GridSelector
{

	GridSelector( sf::Vector2i pos, int xSize, int ySize, int iconX, int iconY, 
		bool displaySelected, bool displayMouseOver, Panel * p );
	~GridSelector();
	void Set( int xi, int yi, sf::Sprite s, const std::string &name );
	void Draw( sf::RenderTarget *target );
	bool Update( bool mouseDown, int posx, int posy );
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

struct ActorType;
struct EnemyChooser;
struct Enemy;
struct EnemyChooseRect;
struct ImageChooseRect;

struct UIMouseUser
{
	UIMouseUser(sf::Vector2i &pos)
		:position(pos),
		isMouseDownLeft(false),
		lastMouseDownLeft(false),
		isMouseDownRight(false),
		lastMouseDownRight(false)
	{

	}
	bool IsMouseDownLeft();
	bool IsMouseDownRight();
	bool IsMouseLeftClicked();
	bool IsMouseLeftReleased();
	bool IsMouseRightClicked();
	bool IsMouseRightReleased();
	const sf::Vector2i & GetMousePos();
	void Update(bool mouseDownL,
		bool mouseDownR,
		sf::Vector2i &mousePos);
	sf::Vector2i position;
	sf::Vector2f GetFloatPos()
	{
		return sf::Vector2f(position);
	}
private:
	sf::Vector2i mousePos;
	bool isMouseDownLeft;
	bool lastMouseDownLeft;

	bool isMouseDownRight;
	bool lastMouseDownRight;

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
		E_RELEASED
	};

	ChooseRectType chooseRectType;
	EnemyChooseRect *GetAsEnemyChooseRect();
	ImageChooseRect *GetAsImageChooseRect();

	ChooseRect( ChooseRectType crType, 
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

struct EnemyChooseRect : ChooseRect
{
	EnemyChooseRect( sf::Vertex *v,
		UIMouseUser *mouseUser, sf::Vector2f &position,
		ActorType * type, 
		int level );
	void UpdateSprite(int frameUpdate);
	void Draw(sf::RenderTarget *target);
	void SetSize(float s);

	ActorType *actorType;
	Enemy *enemy;
	sf::View view;
	int level;
};

struct Tileset;
struct ImageChooseRect : ChooseRect
{
	ImageChooseRect(sf::Vertex *v,
		UIMouseUser *mouseUser, sf::Vector2f &position,
		Tileset *ts, int tileIndex );

	void UpdateSprite(int frameUpdate);
	void Draw(sf::RenderTarget *target);
	void SetSize(float s);

	sf::Sprite spr;
	Tileset *ts;
	sf::View view;
	int tileIndex;
};







struct EditSession;
struct CreateEnemyModeUI
{
	CreateEnemyModeUI();
	~CreateEnemyModeUI();
	std::vector<ChooseRect> myRects;
	std::vector<EnemyChooseRect> allEnemyRects;
	sf::Vertex *allEnemyQuads;
	std::vector<EnemyChooseRect> hotbarEnemies;
	sf::Vertex *hotbarQuads;
	int activeHotbarSize;
	std::vector<std::vector<EnemyChooseRect*>> 
		libraryEnemiesVec;
	void SetActiveLibraryWorld(int w);
	int activeLibraryWorld;
	std::vector<ImageChooseRect> worldSelectRects;
	sf::Vertex *worldSelectQuads;
	void UpdateSprites(int sprUpdateFrames);
	void Update(bool mouseDownL,
		bool mouseDownR,
		sf::Vector2i &mousePos);
	void Draw(sf::RenderTarget *target);
	ChooseRectContainer *topbarCont;
	ChooseRectContainer *libCont;
	//Panel *topbarPanel;
	//Panel *libraryPanel;
	EditSession *edit;
};

//struct ChooseRectGroup
//{
//	ChooseRectGroup(int numRects);
//	void AddRect(ChooseRect *cr);
//	std::vector<ChooseRect*> rects;
//	sf::Vertex *allQuads;
//	sf::Vector2f pos;
//	void Update();
//};


//struct EnemyChooser
//{
//	EnemyChooser(std::map<std::string, ActorType*> &types, Panel *p );
//	~EnemyChooser();
//	void Draw(sf::RenderTarget *target);
//	void UpdateSprites(int frameUpdate);
//	bool Update();
//	int tileSizeX;
//	int tileSizeY;
//	ActorType *actorType;
//	int numEnemies;
//	std::vector<ChooseEnemyRect> chooseRects;
//	sf::Vertex *allQuads;
//	
//	bool active;
//	int focusX;
//	int focusY;
//	sf::Vector2i pos;
//	Panel *panel;
//	int selectedX;
//	int selectedY;
//	int mouseOverIndex;
//
//	bool displaySelected;
//	bool displayMouseOver;
//	//GUIHandler *handler;
//};



struct TextBox
{
	TextBox( const std::string &name, int posx, int posy, int width, int lengthLimit, sf::Font &f, Panel *p, const std::string & initialText);
	void SendKey( sf::Keyboard::Key k, bool shift );
	void Draw( sf::RenderTarget *rt );
	bool Update( bool mouseDown, int posx, int posy );
	void SetCursorIndex( int index );
	void SetCursorIndex( sf::Vector2i &mousePos );
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
	sf::Vector2i pos;
	sf::Vector2f size;
	sf::Text text;
	std::string name;

	int characterHeight;
	bool clickedDown;
	Panel *owner;
};

struct CheckBox
{
	CheckBox( const std::string &name, int posx, int posy, Panel *owner );
	void Draw( sf::RenderTarget *target );
	bool Update( bool mouseDown, int posx, int posy );

	const static int SIZE = 16;
	


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
	void Update( bool mouseDownLeft, bool mouseDownRight,
		int posx, int posy );
	
	void AddButton( const std::string &name, sf::Vector2i pos, sf::Vector2f size, const std::string &text );
	void AddTextBox( const std::string &name, sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText );
	void AddLabel( const std::string &name, sf::Vector2i pos, int characterHeight, const std::string &text );
	void AddCheckBox( const std::string &name, sf::Vector2i pos );
	GridSelector * AddGridSelector( const std::string &name, sf::Vector2i pos, 
		int sizex, int sizey, 
		int tilesizex, int tilesizey,
		bool displaySelected,
		bool displayMouseOver );
	

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

struct GUIHandler
{
	virtual void ButtonCallback( Button *b, const std::string & e ) = 0;
	virtual void TextBoxCallback( TextBox *tb, const std::string & e ) = 0;
	virtual void GridSelectorCallback( GridSelector *gs, const std::string & e ) = 0;
	virtual void CheckBoxCallback( CheckBox *cb, const std::string & e ) = 0;
};



#endif