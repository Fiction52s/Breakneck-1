#include "FeedbackForm.h"

using namespace std;
using namespace sf;

FeedbackForm::FeedbackForm()
{
	Vector2i size(700, 300 + 20);

	panel = new Panel("feedbackpanel", size.x, size.y, this, true);
	panel->SetCenterPos(Vector2i(960, 540));
	//panel->SetPosition(Vector2i(0, 0));//Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	int textBoxXBorder = 25;
	//Vector2i textBoxSize(size.x - textBoxXBorder * 2, size.y - 100);
	Vector2i textBoxPos(textBoxXBorder, 50 + 20);
	//feedbackTextBox = panel->AddTextBox("feedbacktextbox", Vector2i(textBoxXBorder, 50), size.x - 2 * textBoxXBorder, 1000, "");
	feedbackTextBox = panel->AddTextBox("feedbacktextbox", textBoxPos, size.x - 2 * textBoxXBorder, 8, 20, 1000, "");

	Vector2i textBoxBottomLeft(textBoxPos.x, textBoxPos.y + feedbackTextBox->size.y);

	feedbackName = panel->AddLabel("feedbacktitle", Vector2i(10, 10), 30, "");

	feedbackName->text.setString("Feedback form for");

	okButton = panel->AddButton("okbutton", textBoxBottomLeft + Vector2i(0, 20), Vector2f(150, 30), "Submit");
	cancelButton = panel->AddButton("cancelbutton", textBoxBottomLeft + Vector2i( 300, 20 ), Vector2f(200, 30), "Cancel");

	panel->SetConfirmButton(okButton);
	panel->SetCancelButton(cancelButton);

	action = A_SHOW;
}

FeedbackForm::~FeedbackForm()
{
	delete panel;
}

void FeedbackForm::Activate()
{
	action = A_SHOW;
	panel->SetFocusedMember(feedbackTextBox);
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