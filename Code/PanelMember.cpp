#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

void PanelMember::SetToolTip(const std::string &str)
{
	if (toolTip != NULL)
	{
		delete toolTip;
		toolTip = NULL;
	}
	//assert(toolTip == NULL);
	toolTip = new ToolTip(str);
}

PanelMember::~PanelMember()
{
	if (toolTip != NULL)
		delete toolTip;
}

void PanelMember::UpdateToolTip(bool contains)
{
	if (contains)
	{
		panel->SetFocusedMember(this);
	}
	else
	{
		panel->RemoveAsFocusedMember(this);
	}
}