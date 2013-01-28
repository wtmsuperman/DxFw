#include "dx_tiny_gui.h"

GUISystem::GUISystem()
{
	mCurrentLayout = 0;
	mResourceMgr = 0;
	mRenderer = 0;
	mDevice = 0;
}

void GUISystem::initOnce(const DxFw& df)
{
	static bool flag = true;
	if (flag)
	{
		mResourceMgr = df.getResourceManager();
		mRenderer = df.getRenderer();
		mDevice = df.getDevice();
		flag = false;
	}
	else
	{
		assert(false && "gui syetem should only initialize once");
	}
}

GUILayout*	GUISystem::createLayout(int id)
{
	assert(getLayout(id) == 0 && "already exists this layout");

	char buffer[32];
	sprintf(buffer,"gui_%d",id);
	GUILayout* layout = new GUILayout(id,mResourceMgr->createResourceGroup(buffer),mRenderer);
	mLayouts.push_back(layout);
	return layout;
}

GUILayout* GUISystem::getLayout(int id)
{
	LayoutIter end = mLayouts.end();
	for (LayoutIter iter = mLayouts.begin(); iter != end; ++iter){
		if ( (*iter)->mID == id){
			return *iter;
		}
	}

	return 0;
}

void GUISystem::destroyLayout(int id)
{
	assert(getLayout(id) != 0 && "layout do not exists");

	LayoutIter end = mLayouts.end();
	for (LayoutIter iter = mLayouts.begin(); iter != end; ++iter){
		if ( (*iter)->mID == id){
			char buffer[32];
			sprintf(buffer,"gui_%d",(*iter)->mID);
			mResourceMgr->releaseAndRemove(buffer);
			delete *iter;
			mLayouts.erase(iter);
			return;
		}
	}
}

DxFont* GUISystem::getFontByID(int id)
{
	FontIter end = mFonts.end();
	for (FontIter iter = mFonts.begin(); iter != end; ++iter){
		if ((*iter)->id == id)
		{
			return *iter;
		}
	}

	return 0;
}

void GUISystem::releaseFont(int id)
{
	FontIter end = mFonts.end();
	for (FontIter iter = mFonts.begin(); iter != end; ++iter){
		if ((*iter)->id == id)
		{
			(*iter)->font->Release();
			delete *iter;
			mFonts.erase(iter);
			return;
		}
	}
	assert(false && "font does not exists");
}

void GUISystem::release()
{
	//release all layout
	LayoutIter end = mLayouts.end();
	for (LayoutIter iter = mLayouts.begin(); iter != end; ++iter){
		GUILayout* layout = *iter;
		char buffer[32];
		sprintf(buffer,"gui_%d",layout->mID);
		mResourceMgr->releaseAndRemove(buffer);
		delete layout;
	}
	mLayouts.clear();

	//release all font
	FontIter fend = mFonts.end();
	for (FontIter iter = mFonts.begin(); iter != fend; ++iter){
		safe_Release((*iter)->font);
		delete *iter;
	}
	mFonts.clear();
}

GUILayout* GUISystem::changeCurrentLayout(int id)
{
	GUILayout* last = mCurrentLayout;
	mCurrentLayout = getLayout(id);
	assert(mCurrentLayout != 0 && "layout does not exists");
	return last;
}

GUILayout* GUISystem::changeCurrentLayout(GUILayout* layout)
{
	GUILayout* last = mCurrentLayout;
	mCurrentLayout = layout;
	assert(mCurrentLayout != 0 && "layout value error");
	return last;
}

DxFont* GUISystem::createFont(const char* fontName,int weight,bool italic,int size,int id)
{
	assert(getFontByID(id) == 0 && "font already exists");
	LPD3DXFONT font;
	if (FAILED(D3DXCreateFont(mDevice,size,0,weight,1,italic
		,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,DEFAULT_QUALITY
		,DEFAULT_PITCH | FF_DONTCARE
		,fontName
		,&font)))
	{
		0;
	}

	DxFont* newfont = new DxFont;
	newfont->font = font;
	newfont->id = id;
	mFonts.push_back(newfont);
	return newfont;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//	GUILayout
//

GUILayout::GUILayout(int id,DxResourceGroup* group,DxRenderer* renderer)
	:mID(id),mRenderer(renderer),mGUIResourceGroup(group)
{

}

GUILabel* GUILayout::createLabel(int x,int y,int width,int height,int id,int fontID)
{
	GUILabel* label = new GUILabel;
	label->type = GUIControl::GUI_LABEL;
	label->x = x;
	label->y = y;
	label->width = width;
	label->height = height;
	label->id = id;
	label->setFont(fontID);
	label->layout = this;
	mGUIControls.push_back(label);
	return label;
}

GUIButton* GUILayout::createButton(int x,int y,int width,int height,int id)
{
	GUIButton* btn = new GUIButton;
	btn->x = x;
	btn->y = y;
	btn->width = width;
	btn->height = height;
	btn->id = id;
	btn->type = GUIControl::GUI_BUTTON;
	GUIVertex vertex[] = {
		{(float)x,(float)y,0.0f,1.0f,0.0f,0.0f}
		,{(float)(x+width),(float)y,0.0f,1.0f,1.0f,0.0f}
		,{(float)x,(float)(y+height),0.0f,1.0f,0.0f,1.0f}
		,{(float)(x+width),(float)(y+height),0.0f,1.0f,1.0f,1.0f}
	};
	char name[32];
	sprintf(name,"gui_%d_%d",mID,id);
	DxBuffer* buffer = mGUIResourceGroup->createStaticBuffer(name,GUIVertex::GUI_FVF,D3DPT_TRIANGLESTRIP,4,0,sizeof(GUIVertex),(void**)&vertex,0);
	btn->setStaticBuffer(buffer);
	btn->layout = this;
	mGUIControls.push_back(btn);
	return btn;
}

GUIImage* GUILayout::createImage(int x,int y,int width,int height,int id)
{
	GUIImage* img = new GUIImage;
	img->x = x;
	img->y = y;
	img->width = width;
	img->height = height;
	img->id = id;
	img->type = GUIControl::GUI_IMAGE;
	GUIVertex vertex[] = {
		{(float)x,(float)y,0.0f,1.0f,0.0f,0.0f}
		,{(float)(x+width),(float)y,0.0f,1.0f,1.0f,0.0f}
		,{(float)x,(float)(y+height),0.0f,1.0f,0.0f,1.0f}
		,{(float)(x+width),(float)(y+height),0.0f,1.0f,1.0f,1.0f}
	};
	char name[32];
	sprintf(name,"gui_%d_%d",mID,id);
	DxBuffer* buffer = mGUIResourceGroup->createStaticBuffer(name,GUIVertex::GUI_FVF,D3DPT_TRIANGLESTRIP,4,0,sizeof(GUIVertex),(void**)&vertex,0);
	img->setStaticBuffer(buffer);
	img->layout = this;
	mGUIControls.push_back(img);
	return img;
}

void GUILayout::render()
{
	mRenderer->beginScene();
	GUIIter end = mGUIControls.end();
	for (GUIIter iter = mGUIControls.begin(); iter != end; ++iter){
		GUIControl* ctrl = *iter;
		ctrl->preRender(mRenderer);
		ctrl->onRender(mRenderer);
		ctrl->postRender(mRenderer);
	}
	mRenderer->endScene();
}

void GUILayout::destroyControl(int id)
{
	assert(getControlById(id) != 0 && "this gui control does not exists");
	GUIIter	end = mGUIControls.end();

	for (GUIIter iter = mGUIControls.begin(); iter != end; ++iter){
		if ((*iter)->id == id){
			mGUIControls.erase(iter);
			delete *iter;
			return;
		}
	}
}

void GUILayout::processGUI(int x,int y,bool mouseDown)
{
	GUIIter	end = mGUIControls.end();

	for (GUIIter iter = mGUIControls.begin(); iter != end; ++iter){
		(*iter)->callback(x,y,mouseDown);
	}
}

GUIControl* GUILayout::getControlById(int id)
{
	GUIIter	end = mGUIControls.end();

	for (GUIIter iter = mGUIControls.begin(); iter != end; ++iter){
		if ((*iter)->id == id){
			return *iter;
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////



//
//	GUILabel
//

GUILabel::GUILabel()
{
	mText = 0;
	mFontID = INT_MAX;
	mColor = 0xff000000;
}

GUILabel::~GUILabel()
{
	safe_deleteArray(mText);
}

void GUILabel::setText(const char* text)
{
	if (text == mText)
		return;

	safe_deleteArray(mText);
	mText = new char[strlen(text) + 1];
	strcpy(mText,text);
}

void GUILabel::printf(const char* s,...)
{
	safe_deleteArray(mText);
	mText = new char[512];
	va_list var;
	va_start(var,s);
	vsprintf(mText,s,var);
	va_end(var);
}

void GUILabel::setColor(DWORD color)
{
	mColor = color;
}

void GUILabel::setFont(int fontId)
{
	mFontID = fontId;
}

void GUILabel::callback(int x,int y,bool mouseDown)
{
	//do nothing
}

void GUILabel::preRender(DxRenderer* renderer)
{
	//do nothing
}

void GUILabel::onRender(DxRenderer* renderer)
{
	DxFont*	dxfont = GUISystem::getSingletonPtr()->getFontByID(mFontID);
	assert(dxfont != 0 && "font id error");
	ID3DXFont* font = dxfont->font;
	RECT rect = {x,y,x+width,y+height};
	font->DrawTextA(0,mText,-1,&rect,DT_SINGLELINE | DT_TOP | DT_LEFT,mColor);
}

void GUILabel::postRender(DxRenderer* renderer)
{
	//do nothing
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// GUIImage
//

void GUIImage::setImage(const char* img)
{
	mImage = layout->getResourceGroup()->loadTexture(img);
}

void GUIImage::onRender(DxRenderer* renderer)
{
	renderer->applyTexture(0,mImage);
	renderer->render(mBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////


//
// GUIButton
//

void GUIButton::setTexture(const char* up,const char* down,const char* over)
{
	mUpTexture = layout->getResourceGroup()->loadTexture(up);
	mOverTexture = layout->getResourceGroup()->loadTexture(over);
	mDownTexture = layout->getResourceGroup()->loadTexture(down);
}

void GUIButton::callback(int mx,int my,bool mouseDown)
{
	if (mx >= x && mx <= x+width && my >= y && my <= y + height){
		if (mouseDown){
			mState = GUI_BUTTON_DWON;
		}
		else{
			mState = GUI_BUTTON_MOVE;
		}
	}

	if (mState == GUI_BUTTON_DWON && mClickListener != 0)
	{
		mClickListener(id);
	}
}

void GUIButton::preRender(DxRenderer* renderer)
{
	renderer->enableTransparent();
}

void GUIButton::onRender(DxRenderer* renderer)
{
	switch (mState){
	case GUI_BUTTON_UP:
		renderer->applyTexture(0,mUpTexture);
		break;
	case GUI_BUTTON_DWON:
		renderer->applyTexture(0,mDownTexture);
		break;
	case GUI_BUTTON_MOVE:
		renderer->applyTexture(0,mOverTexture);
		break;
	}
	renderer->render(mStaticBuffer);
}

void GUIButton::postRender(DxRenderer* renderer)
{
	renderer->disableTransparent();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////