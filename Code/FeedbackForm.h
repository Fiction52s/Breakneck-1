#ifndef __FEEDBACKFORM_H__
#define __FEEDBACKFORM_H__

#include "GUI.h"

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

	FeedbackForm();
	~FeedbackForm();
	void Activate();
	void ButtonCallback(Button *b,
		const std::string &e);
	void CancelCallback(Panel *p);
	void ConfirmCallback(Panel *p);
	void Update();
	bool HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
};

#endif