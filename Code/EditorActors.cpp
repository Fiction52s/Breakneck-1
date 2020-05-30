#include "EditorActors.h"
#include "EditSession.h"
#include "ActorParams.h"
#include "EditorRail.h"

using namespace std;
using namespace sf;

ActorType::ActorType(ParamsInfo &pi)
	:info(pi)
{
	panel = CreatePanel();
	Session *sess = Session::GetSession();

	if (info.ts == NULL)
	{
		info.ts = sess->GetTileset(string("Editor/") + info.name + string("_editor.png"));
	}

	//defaultParams = NULL;
}

void ActorType::CreateDefaultEnemy()
{
	if (info.pMaker == NULL)
		return;
	
	defaultParamsVec.resize(info.numLevels);
	for (int i = 0; i < info.numLevels; ++i)
	{
		defaultParamsVec[i] = info.pMaker(this, i+1);
		defaultParamsVec[i]->CreateMyEnemy();
		if (defaultParamsVec[i]->myEnemy != NULL)
		{
			defaultParamsVec[i]->myEnemy->facingRight = true;
		}
	}
}

ActorType::~ActorType()
{
	if( panel != NULL)
		delete panel;

	for (auto it = defaultParamsVec.begin(); it != defaultParamsVec.end(); ++it)
	{
		delete (*it);
	}
}

bool ActorType::CanBeGrounded()
{
	return info.canBeGrounded;//(info.pmGround != NULL);
}

bool ActorType::CanBeRailGrounded()
{
	return info.canBeRailGrounded;
}

bool ActorType::CanBeAerial()
{
	//might adjust this later
	return info.canBeAerial;// || info.name == "player";
}

sf::Sprite ActorType::GetSprite(int xSize, int ySize)
{
	Tileset *ts = info.ts;

	if (ts == NULL)
	{
		return Sprite();
	}

	Sprite spr(*ts->texture);
	spr.setTextureRect(ts->GetSubRect(info.imageTileIndex));

	if (xSize != 0 && ySize != 0)
	{
		float xx = ((float)xSize) / ts->tileWidth;
		float yy = ((float)ySize) / ts->tileHeight;
		float scale = min(xx, yy);
		spr.setScale(scale, scale);
	}
	return spr;
}

sf::Sprite ActorType::GetSprite(bool grounded)
{
	Sprite s = GetSprite();
	if (grounded)
	{
		if (info.name != "poi")
			s.setOrigin(s.getLocalBounds().width / 2 + info.offset.x, s.getLocalBounds().height + info.offset.y);
		else
		{
			s.setOrigin(s.getLocalBounds().width / 2 + info.offset.x, s.getLocalBounds().height / 2 + info.offset.y);
		}
	}
	else
	{
		s.setOrigin(s.getLocalBounds().width / 2 + info.offset.x, s.getLocalBounds().height / 2 + info.offset.y);
	}

	return s;
}


Panel *ActorType::CreateDefaultPanel()
{
	EditSession *edit = EditSession::GetSession();
	if (edit == NULL)
		return NULL;

	int charHeight = 20;

	string panelName = info.name + "_options";
	Panel * p = new Panel(panelName, 1920, 36, edit);
	p->SetPosition(Vector2i(0, 300));
	p->SetAutoSpacing(true, false, Vector2i(10, 0), Vector2i( 20, 0 ));
	p->SetColor(Color::Green);

	if (info.writeLoop)
	{
		p->AddLabel("loop_label", Vector2i(0, 0), charHeight, "loop");
		p->AddCheckBox("loop", Vector2i(0, 0));
	}

	if (info.writePath)
	{
		p->AddButton("createpath", Vector2i(0, 0), Vector2f(200, 28 + 4), "Create Path");
	}

	if (info.writeMonitor)
	{
		p->AddLabel("Has Key:", Vector2i(30, 6), charHeight, "key");
		p->AddCheckBox("monitor", Vector2i(0, 0));
	}
	return p;
}

void ActorType::AddSetDirectionButton(Panel *p)
{
	p->AddButton("setdirection", Vector2i(0, 0), Vector2f(150, 24), "Set Direction");
}

void ActorType::AddLabeledSlider( Panel *p, const std::string &name,
	const std::string &label, int minValue,
	int maxValue, int defaultValue)
{
	int width = (maxValue - minValue) * 4;
	if (width > 400)
		width /= 2;
	p->AddLabel(name, Vector2i(0, 6), 20, label);
	p->AddSlider(name, Vector2i(0, 6), width, minValue, maxValue, defaultValue);
}

const std::string &ActorType::GetSelectedSpecialDropStr()
{
	return panel->dropdowns["specialoptions"]->GetSelectedText();
}

const std::string & ActorType::GetSpecialDropStr(int ind)
{
	return panel->dropdowns["specialoptions"]->options[ind];
}

int ActorType::GetSelectedSpecialDropIndex()
{
	return panel->dropdowns["specialoptions"]->selectedIndex;
}

void ActorType::SetSpecialDropIndex(int i)
{
	panel->dropdowns["specialoptions"]->SetSelectedIndex(i);
}

Panel *ActorType::CreatePanel()
{
	EditSession *edit = EditSession::GetSession();

	if (edit == NULL)
		return NULL;

	Panel *p = NULL;
	string &name = info.name;
	//extra
	if (name == "poi")
	{
		p = CreateDefaultPanel();
		AddSpecialOptionDropdown(p);
	}
	else if (name == "xbarrier")
	{
		p = CreateDefaultPanel();
		AddSpecialOptionDropdown(p);
		p->AddCheckBox("hasedge", Vector2i(0, 0));
	}
	else if (name == "extrascene")
	{
		p = CreateDefaultPanel();
		AddSpecialOptionDropdown(p);
		std::vector<string> sceneTypes{ "Pre-Level", "Post-Level" };
		p->AddDropdown("scenetype", Vector2i(0, 10), Vector2i(200, 28), sceneTypes, 0);
	}
	else if (name == "camerashot")
	{
		p = CreateDefaultPanel();
		AddSpecialOptionDropdown(p);
		p->AddButton("setzoom", Vector2i(0, 0), Vector2f(200, 28 + 4), "Set Zoom");
	}
	else if (name == "shippickup")
	{
		p = CreateDefaultPanel();
		p->AddCheckBox("facingright", Vector2i(0, 0));
	}
	else if (name == "blocker" || name == "greenblocker")
	{
		p = CreateDefaultPanel();
		std::vector<string> bTypes{ "Grey", "Blue", "Green", "Yellow",
		"Orange", "Red", "Magenta", "Black" };
		p->AddDropdown("btype", Vector2i(0, 10), Vector2i(200, 28), bTypes, 0);
		AddLabeledSlider(p, "spacing", "Spacing:", 40, 300, 40);
		p->AddLabel("fill", Vector2i(0, 0), 28, "Fill Mode:");
		p->AddCheckBox("fill", Vector2i(0, 0), true);
		p->AddButton("createchain", Vector2i(0, 0), Vector2f(200, 28 + 4), "Create Chain");
	}
	else if (name == "healthfly")
	{
		p = CreateDefaultPanel();
		std::vector<string> fTypes{ "Grey", "Blue", "Green", "Yellow",
			"Orange", "Red", "Magenta", "Black" };
		p->AddDropdown("ftype", Vector2i(0, 10), Vector2i(200, 28), fTypes, 0);
		AddLabeledSlider(p, "spacing", "Spacing:", 40, 300, 40);
		p->AddLabel("fill", Vector2i(0, 0), 28, "Fill Mode:");
		p->AddCheckBox("fill", Vector2i(0, 0), true);
		p->AddButton("createchain", Vector2i(0, 0), Vector2f(200, 28 + 4), "Create Chain");
	}
	else if (name == "flowerpod")
	{
		p = new Panel("flowerpod_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("podtype", Vector2i(20, 150), 200, 20, "0");
	}
	else if (name == "shard")
	{
		p = CreateDefaultPanel();
	}
	else if (name == "grindjugglercw" || name == "grindjugglerccw" || name == "groundedgrindjugglercw"
		|| name == "groundedgrindjugglerccw" || name == "hungrycomboer" || name == "hungryreturncomboer"
		|| name == "relativecomboer" || name == "relativecomboerdetach"
		|| name == "downgravityjuggler" || name == "upgravityjuggler" || name == "bouncejuggler"
		|| name == "wirejuggler")
	{
		p = CreateDefaultPanel();
		AddLabeledSlider(p, "numJuggles", "required juggles:", 2, 20, 3);
	}
	else if (name == "spring")
	{
		p = CreateDefaultPanel();
		AddSetDirectionButton(p);
		AddLabeledSlider(p, "speed", "launch speed:", 15, 60, 30);
	}
	//w1
	//w2
	/*else if (name == "gravityspring" || name == "bouncespring"
		|| name == "airbouncespring" || name == "swinglaunchercw" || name == "swinglauncherccw")
	{
		p = CreateDefaultPanel();
		p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");
		p->AddTextBox("speed", Vector2i(20, 200), 200, 3, "");
	}*/
	else if (name == "teleporter")
	{
		p = CreateDefaultPanel();
		AddSetDirectionButton(p);
		//p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");
	}
	//w3
	else if (name == "rail" || name == "grindrail")
	{
		p = new Panel("rail_options", 200, 500, edit);
		p->AddCheckBox("accelerate", Vector2i(120, 250));
		p->AddButton("createrail", Vector2i(20, 300), Vector2f(100, 50), "Create Rail");
	}
	//w5
	else if (name == "narwhal")
	{
		p = new Panel("narwhal_options", 200, 600, edit);
		p->AddTextBox("moveframes", Vector2i(20, 300), 200, 20, "10");
		p->AddButton("createpath", Vector2i(20, 350), Vector2f(100, 50), "Create Path");

		p->AddCheckBox("monitor", Vector2i(20, 400));
	}
	else if (name == "nexus")
	{
		p = new Panel("nexus_options", 200, 500, edit);
		p->AddTextBox("nexusindex", Vector2i(20, 150), 200, 20, "1");
	}

	else if (name == "groundtrigger")
	{
		p = new Panel("groundtrigger_options", 200, 500, edit);
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");

		p->AddCheckBox("facingright", Vector2i(20, 250));
		p->AddTextBox("triggertype", Vector2i(20, 150), 200, 20, "0");
	}
	else if (name == "airtrigger")
	{
		p = new Panel("airtrigger_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("triggertype", Vector2i(20, 150), 200, 20, "0");
		p->AddButton("createrect", Vector2i(20, 350), Vector2f(100, 50), "Create Rect");
	}
	else
	{
		p = CreateDefaultPanel();
	}

	return p;
}

bool ActorType::IsInSpecialOptions(const std::string & n)
{
	for (auto it = specialTypeOptions.begin(); it != specialTypeOptions.end(); ++it)
	{
		if (n == (*it))
		{
			return true;
		}
	}

	return false;
}

bool ActorType::LoadSpecialTypeOptions()
{
	string fileName = "Resources/Editor/SpecialOptions/" + info.name + "_options.txt";
	ifstream is;
	is.open(fileName);
	string s;

	if (is.is_open())
	{
		specialTypeOptions.reserve(10); //can bump this up later

		while (true)
		{
			is >> s;
			specialTypeOptions.push_back(s);
			if (is.eof())
			{
				break;
			}
		}
	}
	else
	{
		cout << "failed to open options file: " << fileName << endl;
		assert(0);
		return false;
	}

	return true;
}

void ActorType::AddSpecialOptionDropdown( Panel * p)
{
	if (!LoadSpecialTypeOptions())
	{
		cout << "failed to load special type options" << endl;
		assert(0);
	}
	else
	{
		p->AddDropdown("specialoptions", Vector2i(0, 10), Vector2i(200, 28), specialTypeOptions, 0);
	}
}

void ActorType::LoadEnemy(std::ifstream &is, ActorPtr &a)
{
	int terrainIndex;
	int edgeIndex;
	Vector2i pos;
	double edgeQuantity;
	int hasMonitor;
	string &name = info.name;

	a = info.pLoader(this, is);

	a->posInfo.AddActor(a);
}

bool ActorType::IsGoalType()
{
	string &name = info.name;
	return name == "goal"
		|| name == "greengoal"
		|| name == "shippickup"
		|| name == "nexus";
}

ActorGroup::ActorGroup(const std::string &n)
	:name(n)
{

}

ActorGroup::~ActorGroup()
{
	/*for (auto it = actors.begin(); it != actors.end(); ++it)
	{
		delete (*it);
	}*/
}

void ActorGroup::Draw(sf::RenderTarget *target)
{
	for (list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void ActorGroup::DrawPreview(sf::RenderTarget *target)
{
	//CircleShape cs;
	//cs.setFillColor( Color::Red );

	for (list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it)
	{

		//(*it)->DrawPreview( target );
	}
}

void ActorGroup::WriteFile(std::ofstream &of)
{
	//group name and number of actors in the group
	if (name == "player")
		return;

	cout << "group size: " << actors.size() << endl;
	of << name << " " << actors.size() << endl;
	for (list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it)
	{
		(*it)->WriteFile(of);
	}
}