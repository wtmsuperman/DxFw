#include "dx_tiny_gui.h"

GUISystem::GUISystem(DxRenderer* renderer,DxResourceManager* resMgr)
	:mRenderer(renderer),mResourceMgr(resMgr),mLayouts()
{
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
		 {x,y,0.0f,1.0f,0.0f,0.0f}
		,{x+width,y,0.0f,1.0f,1.0f,0.0f}
		,{x,y+height,0.0f,1.0f,0.0f,1.0f}
		,{x+width,y+height,0.0f,1.0f,1.0f,1.0f}
	};
	char name[32];
	sprintf(name,"gui_%d_%d",mID,id);
	DxBuffer* buffer = mGUIResourceGroup->createStaticBuffer(name,GUIVertex::GUI_FVF,D3DPT_TRIANGLESTRIP,4,0,sizeof(GUIVertex),(void**)&vertex,0);
	btn->setStaticBuffer(buffer);
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
		 {x,y,0.0f,1.0f,0.0f,0.0f}
		,{x+width,y,0.0f,1.0f,1.0f,0.0f}
		,{x,y+height,0.0f,1.0f,0.0f,1.0f}
		,{x+width,y+height,0.0f,1.0f,1.0f,1.0f}
	};
	char name[32];
	sprintf(name,"gui_%d_%d",mID,id);
	DxBuffer* buffer = mGUIResourceGroup->createStaticBuffer(name,GUIVertex::GUI_FVF,D3DPT_TRIANGLESTRIP,4,0,sizeof(GUIVertex),(void**)&vertex,0);
	img->setStaticBuffer(buffer);
	mGUIControls.push_back(img);
	return img;
}

void GUILayout::render()
{
	GUIIter end = mGUIControls.end();
	for (GUIIter iter = mGUIControls.begin(); iter != end; ++iter){
		(*iter)->preRender(mRenderer);
		(*iter)->onRender(mRenderer);
		(*iter)->postRender(mRenderer);
	}
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

void GUILabel::setText(const char* s,...)
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
	ID3DXFont* font = FontManager::getSingletonPtr()->getFontByID(mFontID);
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

void GUIImage::onRender(DxRenderer* renderer)
{
	renderer->applyTexture(0,mImage);
	renderer->render(mBuffer);
	renderer->applyTexture(0,0); //
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////


//
// GUIButton
//

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
		renderer->applyTexture(0,mMoveTexture);
		break;
	}
	renderer->render(mStaticBuffer);
	renderer->applyTexture(0,0);
}

void GUIButton::postRender(DxRenderer* renderer)
{
	renderer->disableTransparent();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////