#include "FeedbackForm.h"

using namespace std;
using namespace sf;

FeedbackForm::FeedbackForm()
{
	Vector2i size(700, 400);

	panel = new Panel("feedbackpanel", size.x, size.y, this, true);
	panel->SetCenterPos(Vector2i(960, 540));
	//panel->SetPosition(Vector2i(0, 0));//Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	int textBoxXBorder = 25;
	Vector2i textBoxSize(size.x - textBoxXBorder * 2, size.y - 100);
	//feedbackTextBox = panel->AddTextBox("feedbacktextbox", Vector2i(textBoxXBorder, 50), size.x - 2 * textBoxXBorder, 1000, "");
	feedbackTextBox = panel->AddTextBox("feedbacktextbox", Vector2i(textBoxXBorder, 50), 100, 5, 20, 1000, ""); //,size.x - 2 * textBoxXBorder, 5, 20, 1000, "");

	//feedbackName = panel->AddLabel("feedbacktitle", Vector2i(10, 10), 28, "");

	//okButton = panel->AddButton("okbutton", Vector2i(10, 100), Vector2f(50, 30), "OK");
	//cancelButton = panel->AddButton("cancelbutton", Vector2i(70, 100), Vector2f(50, 30), "Cancel");

	//panel->SetConfirmButton(okButton);
	//panel->SetCancelButton(cancelButton);

	action = A_SHOW;
}

FeedbackForm::~FeedbackForm()
{
	delete panel;
}

void FeedbackForm::Activate()
{
	action = A_SHOW;
}

void FeedbackForm::ButtonCallback(Button *b,
	const std::string &e)
{
	if (b == okButton)
	{
		ConfirmCallback(panel);
	}
	else if (b == cancelButton)
	{
		CancelCallback(panel);
	}
}

void FeedbackForm::ConfirmCallback(Panel *p)
{
	action = A_CONFIRM;
}

void FeedbackForm::CancelCallback(Panel *p)
{
	action = A_CANCEL;
}

void FeedbackForm::Update()
{
	panel->MouseUpdate();
}

bool FeedbackForm::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void FeedbackForm::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}