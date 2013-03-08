//#include "dxfw.h"
#include <Windows.h>
#include "dx/dxfw.h"
#include "xmodel.h"
#include "node/scene_node.h"
#include "node/scene_node_container.h"
#include "route/route.h"
#include "node/node_action.h"
#include "dx/dx_particle_system.h"
#include "dx/dx_logging.h"
#include <time.h>

void fuck(int id)
{
	MessageBox(0,"h","",MB_OK);
}

class Spaceship
{
public:
	SceneNode* node;
};


int WINAPI WinMain(HINSTANCE hist,HINSTANCE phist,LPSTR cmd,int show)
{
	srand((unsigned int)time(0));
	WinInfo info;
	if (!initWindow(800,600,false,"my game",hist,&info))
	{
		return -1;
	}

	DxParam param = {info.isFullScreen,info.width,info.height,info.hwnd};
	DxFw df;
	if (!df.initDx(param))
	{
		MessageBox(0,"fuck","",MB_OK);
	}

	if (!df.initInput(info.hwnd,info.hist,false))
	{
		MessageBox(0,"input failed","",MB_OK);
	}

	df.getRenderer()->setAsPerspectiveProjection(PI_OVER_2,800.0f / 600.0f,1.0f,1000.0f);

	SceneNodeContainer c;
	SceneNode* n = c.createNode("test");
	XModel x;
	x.mXmodel = df.getResourceManager()->loadXModel(DEFAULT_GROUP_NAME,"ship.x");
	n->attach(&x);
	MSG msg;

	D3DLIGHT9 light;
	::ZeroMemory(&light, sizeof(light));
	light.Type      = D3DLIGHT_DIRECTIONAL;
	light.Ambient   = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	light.Diffuse   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Specular  = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	light.Direction = D3DXVECTOR3(1.0f, -1.0f, 0.0f);

	IDirect3DDevice9* device = df.getDevice();

	device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_POINT);

	df.getRenderer()->setLight(0,&light);

	Node camera;
	camera.setPosition(0.0f,100.0f,0.0f);
	camera.lookAt(Vector3(0.0f,0.0f,0.0f));

	n->yaw(PI);

	//Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	//Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	GUISystem* guisys = GUISystem::getSingletonPtr();
	guisys->initOnce(df);

	//Route r(new LinearPathRel(Vector3(10.0f,0.0f,10.0f),0.5),true);
	//r.addPath(new LinearPathRel(Vector3(-10.0f,0.0f,10.0f),0.5));

	//TranslateToAction action(n,Vector3(10.0f,0.0f,0.0f),1.0,Node::TS_PARENT);
	//guisys->load("media/gui/dx.xml");
	//GUIButton* btn = (GUIButton*) guisys->getLayout(0)->getControlById(0);

	//btn->setClickListener(fuck);

	//GUIImage* img = layout->createImage(0,0,800,600,2);
	//img->setImage("menu/mainMenu.jpg");

	GUILayout* layout = guisys->createLayout(0);
	DxFont* font = guisys->createFont("",15,false,15,0);
	GUILabel* label = layout->createLabel(0,0,800,600,0,0);
	label->setColor(0xffff0000);
	guisys->changeCurrentLayout(0);
	loggingInit(label);

	DxParticleSystem ps;
	ps.init(&df,5012,"media/tex/smoke000.tga");
	DxParticleEmitter* em = new DxParticleEmitter;
	
	DxColorValue e = {0.7f,0.7f,0.8f,0.7f};
	DxColorValue b = {0.7f,0.7f,0.8f,0.7f};
 	em->colorBegine = b;
	em->colorEnd = e;
	
	em->angle = 0.0f;
	em->up = Vector3::UNIT_Y;
	em->direction = Vector3::UNIT_Z;
	em->duration = 0.0f;
	em->emitRate = 1000.0f;
	em->maxPosition = Vector3(1.0f,10.0f,2.0f);
	em->minPosition = Vector3(-1.0f,0.0f,-1.0f);
	em->minVelocity = 50.0f;
	em->maxVelocity = 80.0f;
	em->maxTimeLL = 0.5f;
	em->minTimeLL = 0.1f;
	em->size = 1.5f;
	em->repeatDelay = 2.5f;
	em->currentTime = em->duration;
	em->repeatTimeRemain = em->repeatDelay;

	ps.setEmitter(em);
	ps.addAffector(new LinearForceAffector(0.0f,0.0f,-100.0f,LinearForceAffector::FT_ADD));
	ps.addAffector(new ColorFaderAffector(0.0f,2.0f,-2.0f,-3.0f));
	n->attach(&ps);
	//logToScreen("p","fuck");
	/*
	SceneNode* nps = c.createNode("testps");
	nps->attach(&ps);
	nps->yaw(PI);
	nps->translate(0.0f,0.0f,-11.0f);
	*/

	while (true)
	{
		DxRenderer* renderer = df.getRenderer();
		renderer->clear(true,true,true,0xff000000);

		Matrix4x4 v;
		camera.generateParentToLocalMatrix(&v);
		renderer->setViewMatrix(v);

		renderer->render(&c);

		guisys->render();

		//n->yaw(getTimeSinceLastFrame() * 1.0f);
		float timeDelta = getTimeSinceLastFrame();
		Vector3 walk = Vector3::ZERO;

		float speed = 60.0f;
		float hspeed = speed;
		float vspeed = speed * 0.7f;

		IInputSystem* isys = df.getInputSystem();

		if (isys->keyDown(DIK_W))
		{
			walk.z += vspeed * timeDelta;
		}
		if (isys->keyDown(DIK_A))
		{
			walk.x -= hspeed * timeDelta;
		}
		if (isys->keyDown(DIK_D))
		{
			walk.x += hspeed * timeDelta;
		}
		if (isys->keyDown(DIK_S))
		{
			walk.z -= vspeed * timeDelta;
		}
		
		//Vector3 ani;
		//r.calcPosition(timeDelta,&ani);
		//n->translate(ani,Node::TS_PARENT);

		//if (action.act(timeDelta))
		//{
			
		//}

		n->translate(walk,Node::TS_PARENT);

		isys->capture();
		Point p = isys->getMouseClientPosition();
		guisys->processGUI(p.x,p.y,isys->mouseButtonDown(0));
		ps.update(timeDelta);
		//logToScreen("particle num:","%d",ps.size());

		renderer->present();

		messagePump(&msg);
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}
	
	guisys->release();
	df.release();
	UnregisterClass(info.className,info.hist);
	return 0;
}