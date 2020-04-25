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
		defaultParamsVec[i]->myEnemy = defaultParamsVec[i]->GenerateEnemy();
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


Panel *ActorType::CreateDefaultPanel(const std::string &n, bool mon, bool level, bool path, bool loop)
{
	EditSession *edit = EditSession::GetSession();
	if (edit == NULL)
		return NULL;

	int height = 800;

	Panel * p = new Panel(n, 200, height, edit);

	p->AddButton("ok", Vector2i(100, height - 90), Vector2f(100, 50), "OK");
	p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
	p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");

	if (loop)
	{
		p->AddLabel("loop_label", Vector2i(20, 150), 20, "loop");
		p->AddCheckBox("loop", Vector2i(120, 155));
	}

	if (level)
	{
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
	}

	if (path)
	{
		p->AddButton("createpath", Vector2i(20, 250), Vector2f(100, 50), "Create Path");
	}


	if (mon)
	{
		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	return p;
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
		p = new Panel("poi_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "NO NAME");
	}
	if (name == "xbarrier")
	{
		p = new Panel("xbarrier_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "-----");
		p->AddCheckBox("hasedge", Vector2i(20, 240));
	}
	if (name == "extrascene")
	{
		p = new Panel("extrascene_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "-----");
		p->AddTextBox("scenetype", Vector2i(20, 240), 200, 20, "0");
	}
	else if (name == "shippickup")
	{
		p = new Panel("shippickup_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "NO NAME");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		//p->AddTextBox( "barrier", Vector2i( 20, 330 ), 50, 1, "-" );
		p->AddCheckBox("facingright", Vector2i(20, 250));
	}
	else if (name == "key")
	{
		p = new Panel("key_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		p->AddTextBox("numkeys", Vector2i(20, 150), 200, 20, "3");
		p->AddTextBox("zonetype", Vector2i(20, 200), 200, 20, "0");
	}
	else if (name == "blocker" || name == "greenblocker")
	{
		p = new Panel("blocker_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		//p->AddLabel("type_label", Vector2i(20, 150), 20, "type");
		p->AddCheckBox("armored", Vector2i(120, 155));
		p->AddTextBox("btype", Vector2i(20, 200), 200, 20, "0");
		p->AddTextBox("spacing", Vector2i(20, 250), 200, 20, "0");
		p->AddTextBox("level", Vector2i(20, 300), 200, 20, "1");
		p->AddButton("createpath", Vector2i(20, 340), Vector2f(100, 50), "Create Chain");


		/*GridSelector *gs = p->AddGridSelector("blockerSelector", Vector2i(0, 0), 8, 1, 64, 64, true, true);
		Tileset *tsws = edit->GetTileset("Editor/whitesquare.png");
		Sprite sqSpr;
		sqSpr.setTexture(*tsws->texture);

		gs->Set(0, 0, sqSpr, "");*/

		//p->
	}
	else if (name == "flowerpod")
	{
		p = new Panel("flowerpod_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");

		p->AddTextBox("podtype", Vector2i(20, 150), 200, 20, "0");
	}
	else if (name == "healthfly")
	{
		p = CreateDefaultPanel("healthfly_options", true, false);
	}
	else if (name == "shard")
	{
		p = new Panel("shard_options", 700, 1080, edit);
		p->AddLabel("shardtype", Vector2i(20, 900), 24, "SHARD_W1_TEACH_JUMP");

		edit->CreateShardGridSelector(p, Vector2i(0, 0));
		p->AddButton("ok", Vector2i(100, 1000), Vector2f(100, 50), "OK");
	}
	else if (name == "ship")
	{
		p = CreateDefaultPanel("ship_options", false, false, false, false);
	}
	else if (name == "booster")
	{
		p = CreateDefaultPanel("booster_options", false, true);
	}
	else if (name == "gravityincreaser" || name == "gravitydecreaser")
	{
		p = CreateDefaultPanel("gravchanger_options", false, true);
	}
	else if (name == "comboer" || name == "splitcomboer")
	{
		p = CreateDefaultPanel("comboer_options", false, true, true, true);
	}
	else if (name == "gravdowncomboer" || name == "gravupcomboer"
		|| name == "bouncecomboer" )
	{
		p = CreateDefaultPanel("gravcomboer_options", true, true, true, true);
	}
	else if (name == "grindjugglercw" || name == "grindjugglerccw" || name == "groundedgrindjugglercw"
		|| name == "groundedgrindjugglerccw" || name == "hungrycomboer" || name == "hungryreturncomboer"
		|| name == "relativecomboer" || name == "relativecomboerdetach")
	{
		p = CreateDefaultPanel("grindjuggler_options", true, true, false, false);
		p->AddTextBox("numjuggles", Vector2i(20, 600), 200, 20, "0");
	}
	else if (name == "airdashjuggler")
	{
		p = new Panel("airdashjuggler_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
		p->AddCheckBox("monitor", Vector2i(20, 330));
		p->AddButton("createpath", Vector2i(20, 250), Vector2f(100, 50), "Create Path");
	}
	else if (name == "downgravityjuggler" || name == "upgravityjuggler" || name == "bouncejuggler"
		|| name == "wirejuggler" )
	{
		p = CreateDefaultPanel("juggler_options", true, true, true, false);
		p->AddTextBox("numjuggles", Vector2i(20, 600), 200, 20, "0");


		/*p = new Panel("juggler_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
		p->AddCheckBox("monitor", Vector2i(20, 330));

		numjuggles*/
	}
	else if (name == "jugglercatcher")
	{
		p = new Panel("jugglercatcher_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	else if (name == "spring")
	{
		p = new Panel("spring_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");

		p->AddTextBox("speed", Vector2i(20, 200), 200, 3, "");
	}
	//w1
	else if (name == "patroller")
	{
		p = new Panel("patroller_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		p->AddLabel("loop_label", Vector2i(20, 150), 20, "loop");
		p->AddCheckBox("loop", Vector2i(120, 155));
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
		p->AddButton("createpath", Vector2i(20, 250), Vector2f(100, 50), "Create Path");

		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	else if (name == "basicturret")
	{
		p = new Panel("basicturret_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 150), 200, 20, "0");

		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	else if (name == "crawler")
	{
		p = new Panel("crawler_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	else if (name == "shroom")
	{
		p = new Panel("shroom_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");
		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	else if (name == "airdasher")
	{
		p = new Panel("airdasher_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		p->AddTextBox("level", Vector2i(20, 150), 200, 20, "0");
		p->AddCheckBox("monitor", Vector2i(20, 330));
	}

	//w2
	else if (name == "bat")
	{
		p = new Panel("bat_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		p->AddLabel("loop_label", Vector2i(20, 150), 20, "loop");
		p->AddCheckBox("loop", Vector2i(120, 155));
		//p->AddTextBox("bulletspeed", Vector2i(20, 200), 200, 20, "10");
		//p->AddTextBox( "nodedistance", Vector2i( 20, 250 ), 200, 20, "10" );
		//p->AddTextBox("framesbetweennodes", Vector2i(20, 300), 200, 20, "10");
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");

		p->AddButton("createpath", Vector2i(20, 350), Vector2f(100, 50), "Create Path");

		p->AddCheckBox("monitor", Vector2i(20, 400));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		//p->
	}
	else if (name == "poisonfrog")
	{
		p = new Panel("poisonfrog_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 150), 200, 20, "0");
		//p->AddTextBox("ystrength", Vector2i(20, 200), 200, 20, "10");
		//p->AddTextBox("gravfactor", Vector2i(20, 250), 200, 20, "5");
		//p->AddTextBox("jumpwaitframes", Vector2i(20, 300), 200, 20, "10");
		//p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 


		p->AddCheckBox("monitor", Vector2i(20, 330));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "gravityfaller")
	{
		p = new Panel("gravityfaller_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 150), 200, 20, "0");
		//p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 


		p->AddCheckBox("monitor", Vector2i(20, 330));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "gravityspring" || name == "bouncespring"
		|| name == "airbouncespring" || name == "swinglaunchercw" || name == "swinglauncherccw")
	{
		p = new Panel("gravityspring_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");

		p->AddTextBox("speed", Vector2i(20, 200), 200, 3, "");
	}
	else if (name == "teleporter")
	{
		p = new Panel("teleporter_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");

		//p->AddTextBox("speed", Vector2i(20, 200), 200, 3, "");
	}
	else if (name == "stagbeetle")
	{
		p = new Panel("stagbeetle_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		//p->AddLabel("clockwise_label", Vector2i(20, 150), 20, "clockwise");
		//p->AddCheckBox("clockwise", Vector2i(120, 155));
		p->AddTextBox("level", Vector2i(20, 200), 200, 20, "0");

		p->AddCheckBox("monitor", Vector2i(20, 330));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "curveturret")
	{
		p = new Panel("curveturret_options", 200, 550, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("level", Vector2i(20, 150), 200, 20, "0");
		//p->AddTextBox("waitframes", Vector2i(20, 200), 200, 20, "10");
		//p->AddTextBox("xgravfactor", Vector2i(20, 250), 200, 20, "0");
		//p->AddTextBox("ygravfactor", Vector2i(20, 300), 200, 20, "0");
		//p->AddCheckBox("relativegrav", Vector2i(20, 350));

		p->AddCheckBox("monitor", Vector2i(20, 400));

	}

	//w3
	else if (name == "pulser")
	{
		p = CreateDefaultPanel("pulser_options", true, true, true, true);
	}
	else if (name == "upbouncebooster" || name == "omnibouncebooster")
	{
		p = CreateDefaultPanel("booster_options", false, true);
	}
	//else if (name == "bouncespring" )
	//{
	//	p = new Panel("bouncespring_options", 200, 500, edit);
	//	p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
	//	p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
	//	p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

	//	p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");

	//	//p->AddTextBox("moveframes", Vector2i(20, 200), 200, 3, "");
	//}
	else if (name == "cactus")
	{
		p = CreateDefaultPanel("cactus_options", true, true);
	}
	else if (name == "badger")
	{
		p = CreateDefaultPanel("badger_options", true, true, false, false);
	}
	else if (name == "roadrunner")
	{
		p = CreateDefaultPanel("roadrunner_options", true, true, false, false);
	}
	else if (name == "bouncefloater")
	{
		p = CreateDefaultPanel("bouncefloater_options", false, true, false, false);
	}
	else if (name == "owl")
	{
		p = CreateDefaultPanel("owl_options", true, true, false, false);
		/*p = new Panel("owl_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddTextBox("movespeed", Vector2i(20, 150), 200, 1, "1");
		p->AddTextBox("bulletspeed", Vector2i(20, 200), 200, 1, "1");
		p->AddTextBox("rhythmframes", Vector2i(20, 250), 200, 1, "1");



		p->AddCheckBox("monitor", Vector2i(20, 400));*/
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		//p->
	}

	//w4
	else if (name == "turtle")
	{

		p = CreateDefaultPanel("turtle_options", true, true, false, false);
		/*p = new Panel("turtle_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");*/

		//maybe bullet speed later but it might be too hard if it has variation
		//could have params to have them teleport to offsets around your position
		//instead of always DIRECTLY on it

		//p->AddTextBox( "movespeed", Vector2i( 20, 150 ), 200, 1, "1" ); 
		//p->AddTextBox( "bulletspeed", Vector2i( 20, 200 ), 200, 1, "1" ); 
		//p->AddTextBox( "rhythmframes", Vector2i( 20, 250 ), 200, 1, "1" ); 



		//p->AddCheckBox("monitor", Vector2i(20, 400));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "cheetah")
	{
		p = CreateDefaultPanel("cheetah_options", true, true, false, false);
		/*p = new Panel("cheetah_options", 200, 550, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddCheckBox("monitor", Vector2i(20, 400));*/

		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "railtest")
	{
		p = CreateDefaultPanel("railtest_options", true, true, false, false);
	}
	else if (name == "spider")
	{
		p = CreateDefaultPanel("spider_options", true, true, false, false);
	}
	else if (name == "coral")
	{
		p = new Panel("coral_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddTextBox("moveframes", Vector2i(20, 150), 200, 20, "60");

		p->AddCheckBox("monitor", Vector2i(20, 400));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "rail" || name == "grindrail")
	{
		p = new Panel("rail_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddCheckBox("accelerate", Vector2i(120, 250));
		p->AddButton("createrail", Vector2i(20, 300), Vector2f(100, 50), "Create Rail");
	}
	//w5
	else if (name == "shark")
	{
		p = CreateDefaultPanel("shark_options", true, true, false, false);
		/*p = new Panel("shark_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddTextBox("circleframes", Vector2i(20, 150), 200, 20, "60");*/


		//p->AddTextBox( "movespeed", Vector2i( 20, 150 ), 200, 1, "1" ); 
		//p->AddTextBox( "bulletspeed", Vector2i( 20, 200 ), 200, 1, "1" ); 
		//p->AddTextBox( "rhythmframes", Vector2i( 20, 250 ), 200, 1, "1" ); 



		//p->AddCheckBox("monitor", Vector2i(20, 400));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "overgrowth")
	{
		p = new Panel("overgrowth_options", 200, 550, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddCheckBox("monitor", Vector2i(20, 400));

		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "growingtree")
	{
		p = CreateDefaultPanel("growingtree_options", true, true, false, false);
	}
	else if (name == "ghost")
	{
		p = CreateDefaultPanel("ghost_options", true, true, false, false);
	}
	else if (name == "swarm")
	{
		p = CreateDefaultPanel("swarm_options", true, true, false, false);
	}
	else if (name == "specter")
	{
		p = CreateDefaultPanel("specter_options", true, true, false, false);
	}
	else if (name == "gorilla")
	{
		p = CreateDefaultPanel("gorilla_options", true, true, false, false);
	}
	else if (name == "wiretarget")
	{
		p = CreateDefaultPanel("wiretarget_options", true, true, false, false);
	}
	else if (name == "copycat")
	{
		p = new Panel("copycat_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddCheckBox("monitor", Vector2i(20, 400));
	}
	else if (name == "narwhal")
	{
		p = new Panel("narwhal_options", 200, 600, edit);
		p->AddButton("ok", Vector2i(100, 450), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddTextBox("moveframes", Vector2i(20, 300), 200, 20, "10");
		p->AddButton("createpath", Vector2i(20, 350), Vector2f(100, 50), "Create Path");

		p->AddCheckBox("monitor", Vector2i(20, 400));
	}
	else if (name == "nexus")
	{
		p = new Panel("nexus_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");

		p->AddTextBox("nexusindex", Vector2i(20, 150), 200, 20, "1");
	}

	else if (name == "groundtrigger")
	{
		p = new Panel("groundtrigger_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");

		p->AddCheckBox("facingright", Vector2i(20, 250));
		p->AddTextBox("triggertype", Vector2i(20, 150), 200, 20, "0");
	}
	else if (name == "airtrigger")
	{
		p = new Panel("airtrigger_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("triggertype", Vector2i(20, 150), 200, 20, "0");
		p->AddButton("createrect", Vector2i(20, 350), Vector2f(100, 50), "Create Rect");
	}
	else if (name == "camerashot")
	{
		p = new Panel("camerashot_options", 200, 500, edit);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "----");
		p->AddButton("setzoom", Vector2i(20, 350), Vector2f(100, 50), "Set Zoom");
	}

	return p;
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

void ActorType::PlaceEnemy(ActorParams *ap)
{
	if (ap == NULL)
		return;



	EditSession *edit = EditSession::GetSession();

	if (edit == NULL)
		return;

	ap->enemyLevel = edit->enemySelectLevel;

	bool hasPanel = (ap->type->panel != NULL);

	if (hasPanel)
	{
		edit->tempActor = ap;
		edit->showPanel = panel;
		edit->tempActor->SetPanelInfo();
	}
	else
	{
		edit->showPanel = edit->enemySelectPanel;
		//edit->trackeene
		//edit->trackingEnemy = NULL;
		ActorPtr ac(ap);
		ac->group = edit->groups["--"];

		edit->CreateActor(ac);
	}
}

//depreciated
void ActorType::PlaceEnemy()
{
	EditSession *edit = EditSession::GetSession();

	if (edit == NULL)
		return;


	Vector2i worldPos(edit->worldPos);
	bool placed = false;
	/*if (info.pmGround != NULL)
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			PlaceEnemy(info.pmGround(this));
			placed = true;
		}
	}
	if (info.pmRail != NULL)
	{
		if (edit->enemyEdgeRail != NULL)
		{
			PlaceEnemy(info.pmRail(this));
			placed = true;
		}
	}
	if (!placed && info.pmAir != NULL)
	{
		PlaceEnemy(info.pmAir(this));
	}*/

	string &name = info.name;
	if (name == "blocker" || name == "greenblocker" || name == "spring" || name == "patroller" || name == "bat"
		|| name == "pulser" || name == "narwhal")//curve turret??
	{
		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}


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