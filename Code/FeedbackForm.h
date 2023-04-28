#ifndef __FEEDBACKFORM_H__
#define __FEEDBACKFORM_H__

#include "GUI.h"

struct GameSession;

struct FeedbackForm : GUIHandler
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

	Label *feedbackName;

	TextBox *feedbackTextBox;

	Tileset *ts_star;
	std::vector<ImageChooseRect *> starRects;

	GameSession *game;

	int rating;

	FeedbackForm();
	~FeedbackForm();
	void Activate(GameSession *g);
	void ButtonCallback(Button *b,
		const std::string &e);
	void CancelCallback(Panel *p);
	void ConfirmCallback(Panel *p);
	void SetRating(int r);
	void Update();
	bool HandleEvent(sf::Event ev);
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void Submit();
	void Draw(sf::RenderTarget *target);
};

#endif