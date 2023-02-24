#ifndef __PLAYERBOX_H__
#define __PLAYERBOX_H__

#include <SFML\Graphics.hpp>

struct PlayerSkinShader;
struct ControlProfileMenu;
struct ControllerDualStateQueue;
struct ControlProfile;
struct PlayerBoxGroup;
struct Tileset;
struct TilesetManager;

struct PlayerBox
{
	enum Action
	{
		A_WAITING_FOR_JOIN,
		A_HAS_PLAYER,
		A_CHANGING_CONTROLS,
	};

	enum Mode
	{
		MODE_DEFAULT,
		MODE_CONTROLLER_ONLY,
	};

	sf::Text playerNameText;
	sf::Vector2f topLeft;
	sf::Vertex bgQuad[4];
	sf::Vertex selectQuad[4];
	

	int selectBorderSize;

	PlayerBoxGroup *boxGroup;
	//SinglePlayerControllerJoinScreen *joinScreen;
	sf::Text numberText;
	sf::Text pressText;
	sf::Text skinNumberText;
	int skinIndex;

	sf::Vertex controllerIconQuad[4];
	sf::Vertex portIconQuad[4];

	sf::Vertex kinQuad[4];

	Tileset *ts_buttons;

	sf::Vertex skinChangeIconQuads[4 * 2];

	PlayerSkinShader *playerShader;

	ControllerDualStateQueue *controllerStates;

	ControlProfileMenu *controlMenu;

	sf::Sprite kinSprite;

	

	int action;
	int mode;

	int index;

	PlayerBox(PlayerBoxGroup *p_group, int index );
	~PlayerBox();

	bool IsChangingControls();
	void SetMode(int m);
	void UpdateButtonIconsWhenControllerIsChanged();
	void Update();
	void SetSkin(int index);
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
	void SetControllerStates(ControllerDualStateQueue *conStates, int p_skinIndex);
	void SetCurrProfile(ControlProfile *cp);
	void UpdateCurrProfileText();
	void Show();
	void Hide();
	ControlProfile *GetCurrProfile();
	void SetTopLeft(sf::Vector2f &pos);
	void ClearInfo();
private:
	bool show;
	std::string playerNameStr;
};

struct PlayerBoxGroup
{
	std::vector<PlayerBox*> playerBoxes;
	Tileset *ts_kin;
	Tileset *ts_controllerIcons;
	Tileset *ts_portIcons;

	TilesetManager *tilesetManager;

	int playerBoxWidth;
	int playerBoxHeight;
	int playerBoxSpacing;

	PlayerBoxGroup(TilesetManager *tm, int numBoxes, int p_playerBoxWidth, int p_playerBoxHeight, int p_playerBoxSpacing);
	~PlayerBoxGroup();
	bool IsStateChangingControls(ControllerDualStateQueue *conStates);
	bool IsReadyAndStartPressed();
	bool BackButtonPressed();
	void SetControlProfile(int index, ControlProfile *cp);
	bool IsReady();
	bool IsFull();
	void Update();
	int GetNumFullBoxes();
	void SetBoxTopLeft(int index, sf::Vector2f &pos);
	void SetBoxCenter(int index, sf::Vector2f &pos);
	void SetMode(int m);
	bool AlreadyJoined(ControllerDualStateQueue *conStates);
	void SetControllerStates(int index, ControllerDualStateQueue *conStates, int p_skinIndex);
	void NextSkin(int playerBoxIndex);
	void PrevSkin(int playerBoxIndex);
	bool IsSkinAvailable(int p_skinIndex);
	int GetFirstAvailableSkinIndex();
	bool IsBoxChangingControls(int ind);
	bool CheckControllerJoins();
	ControllerDualStateQueue * GetControllerStates(int ind);
	ControlProfile *GetControlProfile(int ind);
	int GetSkinIndex(int ind);
	void ClearInfo();
	void ClearInfo(int ind);
	void Draw(sf::RenderTarget *target);
	bool TryControllerJoin(ControllerDualStateQueue *conStates);
};

#endif