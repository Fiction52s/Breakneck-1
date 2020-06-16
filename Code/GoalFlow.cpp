#include "GoalFlow.h"
#include <iostream>
#include <assert.h>
#include "Session.h"

using namespace sf;
using namespace std;

GoalFlow::GoalFlow(Vector2f &gPos, list<list<pair<V2d, bool>>> &infoList)
{
	flowFrameCount = 60;
	flowFrame = 0;
	maxFlowRadius = 10000;
	radDiff = 100;
	flowSpacing = 600;
	maxFlowRings = 40;
	goalEnergyFlowVA = NULL;

	sess = Session::GetSession();

	if (!flowShader.loadFromFile("Resources/Shader/flow_shader.frag", sf::Shader::Fragment))
	{
		cout << "flow SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}

	flowShader.setUniform("radDiff", radDiff);
	flowShader.setUniform("Resolution", Vector2f(1920, 1080));// window->getSize().x, window->getSize().y);
	flowShader.setUniform("flowSpacing", flowSpacing);
	flowShader.setUniform("maxFlowRings", maxFlowRadius / maxFlowRings);

	Setup(gPos, infoList);
}

GoalFlow::~GoalFlow()
{
	delete goalEnergyFlowVA;
}

void GoalFlow::Setup( sf::Vector2f &gPos, list<list<pair<V2d, bool>>> &infoList)
{
	SetGoalPos(gPos);

	int totalPoints = 0;
	for (auto it = infoList.begin(); it != infoList.end(); ++it)
	{
		list<pair<V2d, bool>> &pointList = (*it);
		totalPoints += pointList.size();
	}

	//cout << "number of quads: " << totalPoints / 2 << endl;
	goalEnergyFlowVA = new VertexArray(sf::Quads, (totalPoints / 2) * 4);
	VertexArray &va = *goalEnergyFlowVA;
	int extra = 0;
	double width = 16;

	for (auto it2 = infoList.begin(); it2 != infoList.end(); ++it2)
	{
		list<pair<V2d, bool>> &pointList = (*it2);
		for (list<pair<V2d, bool>>::iterator it = pointList.begin(); it != pointList.end(); ++it)
		{
			V2d startPoint = (*it).first;
			++it;
			V2d endPoint = (*it).first;

			V2d along = normalize(endPoint - startPoint);
			V2d other(along.y, -along.x);

			V2d startLeft = startPoint - other * width / 2.0 + along * 16.0;
			V2d startRight = startPoint + other * width / 2.0 + along * 16.0;
			V2d endLeft = endPoint - other * width / 2.0 - along * 16.0;
			V2d endRight = endPoint + other * width / 2.0 - along * 16.0;

			va[extra + 0].color = Color::Red;
			va[extra + 1].color = Color::Red;
			va[extra + 2].color = Color::Red;
			va[extra + 3].color = Color::Red;

			va[extra + 0].position = Vector2f(startLeft.x, startLeft.y);
			va[extra + 1].position = Vector2f(startRight.x, startRight.y);
			va[extra + 2].position = Vector2f(endRight.x, endRight.y);
			va[extra + 3].position = Vector2f(endLeft.x, endLeft.y);

			extra += 4;
		}
	}
}

void GoalFlow::SetGoalPos(sf::Vector2f &gPos)
{
	goalPos = gPos;
	flowShader.setUniform("goalPos", goalPos );
}

void GoalFlow::Update( float camZoom, sf::Vector2f &topLeft  )
{
	V2d pPos = sess->GetPlayerPos(0);
	flowRadius = (maxFlowRadius - (maxFlowRadius / flowFrameCount) * flowFrame);

	flowShader.setUniform("radius", flowRadius / maxFlowRings);
	flowShader.setUniform("zoom", camZoom);
	flowShader.setUniform("playerPos", Vector2f(pPos));

	flowShader.setUniform("topLeft", topLeft );

	++flowFrame;
	if (flowFrame == flowFrameCount)
	{
		flowFrame = 0;
	}
}

void GoalFlow::Draw(sf::RenderTarget *target)
{
	target->draw(*goalEnergyFlowVA, &flowShader);
}