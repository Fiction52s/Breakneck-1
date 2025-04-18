#ifndef __MEDALTIMEFORM_H__
#define __MEDALTIMEFORM_H__

#include "GUI.h"
#include "Tileset.h"

struct GameSession;

struct MedalTimeForm : GUIHandler//TilesetManager, GUIHandler
{
	enum Action
	{
		A_SHOW,
		A_CONFIRM,
		A_CANCEL,
	};


	Panel *panel;
	int action;

	Button *okButton;
	Button *cancelButton;

	Label *levelNameLabel;
	TextBox *goldTimeTextBox;
	TextBox *silverTimeTextBox;

	GameSession *game;

	std::string levelName;
	std::string previewPath;

	int rating;

	MedalTimeForm();
	~MedalTimeForm();
	void Activate(GameSession *g);
	void ButtonCallback(Button *b,
		const std::string &e);
	void CancelCallback(Panel *p);
	void ConfirmCallback(Panel *p);
	void Update();
	bool HandleEvent(sf::Event ev);
	bool Submit();
	void Draw(sf::RenderTarget *target);
};

#endif