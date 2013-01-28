//#include "dxfw.h"
#include <Windows.h>
#include "dxfw.h"
#include "xmodel.h"
#include "scene_node.h"
#include "scene_node_container.h"

void fuck(int id)
{
	MessageBox(0,"h","",MB_OK);
}


int WINAPI WinMain(HINSTANCE hist,HINSTANCE phist,LPSTR cmd,int show)
{
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
	camera.setPosition(0.0f,60.0f,0.0f);
	camera.lookAt(Vector3(0.0f,0.0f,0.0f));

	n->yaw(PI);

	//Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	//Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	GUISystem* guisys = GUISystem::getSingletonPtr();
	guisys->initOnce(df);

	GUILayout* layout = guisys->createLayout(0);
	guisys->createFont("",15,false,15,0);
	GUILabel* label = layout->createLabel(0,0,800,600,0,0);
	label->setColor(0xffff0000);
	label->setText("wo le ge fuck");
	guisys->changeCurrentLayout(layout);

	GUIButton* btn = layout->createButton(0,100,128,32,1);
	btn->setTexture("menu/startUp.png","menu/startDown.png","menu/startOver.png");
	btn->setClickListener(fuck);

	//GUIImage* img = layout->createImage(0,0,800,600,2);
	//img->setImage("menu/mainMenu.jpg");

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
		n->translate(walk,Node::TS_PARENT);

		isys->capture();
		Point p = isys->getMouseClientPosition();
		guisys->processGUI(p.x,p.y,isys->mouseButtonDown(0));
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