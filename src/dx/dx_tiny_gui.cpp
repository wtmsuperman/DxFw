#include "dx/dx_tiny_gui.h"

int getIntVal(const char* val,int base)
{
	if (val == 0)
		return 0;

	if (strchr(val,'.') != 0){
		return int(atof(val) * base);
	}else{
		return atoi(val);
	}
}

void parseControlAttribute(const TiXmlElement* element,GUIControl* out,int width,int height)
{
	assert(out != 0);
	const TiXmlAttribute* attr = element->FirstAttribute();
	
	while (attr != 0){
		if (strcmp("id",attr->Name()) == 0){
			out->id = atoi(attr->Value());
		}
		else if (strcmp("x",attr->Name()) == 0){
			out->x = getIntVal(attr->Value(),width);
		}
		else if (strcmp("y",attr->Name()) == 0){
			out->y = getIntVal(attr->Value(),height);
		}
		else if (strcmp("width",attr->Name()) == 0){
			out->width = getIntVal(attr->Value(),width);
		}else if (strcmp("height",attr->Name()) == 0){
			out->height = getIntVal(attr->Value(),height);
		}
		attr = attr->Next();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////


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
		mDxParam = df.getDxParam();
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
	layout->notifyParent(this);
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
			delete mResourceMgr->remove(buffer);
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
		delete mResourceMgr->remove(buffer);
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

void GUISystem::load(const char* file)
{
	TiXmlDocument doc(file);
	if (!doc.LoadFile())
	{
		assert(false && "file does not exists or file format error");
	}
	TiXmlElement* root = doc.RootElement();
	assert(root != 0 && strcmp("DxGUI",root->Value()) == 0);
	std::cout << root->Value() << std::endl;

	TiXmlElement* element = root->FirstChildElement();

	while (element != 0)
	{
		if (strcmp("Font",element->Value()) == 0){
			const TiXmlAttribute* attr = element->FirstAttribute();
			int id,weight,size;
			const char* name =0;
			bool italic = false;
			while (attr != 0)
			{
				if (strcmp("id",attr->Name())==0){
					id = atoi(attr->Value());
				}else if (strcmp("weight",attr->Name())==0){
					weight = atoi(attr->Value());
				}else if (strcmp("size",attr->Name())==0){
					size = atoi(attr->Value());
				}else if (strcmp("italic",attr->Name())==0){
					if (strcmp("true",attr->Value())==0){
						italic = true;
					}
				}else if (strcmp("name",attr->Name())==0){
					name = attr->Value();
				}
				attr = attr->Next();
			}
			createFont(name,weight,italic,size,id);
		}
		else if (strcmp("Layout",element->Value()) ==0){
			int id = atoi(element->Attribute("id"));
			GUILayout* layout = createLayout(id);
			const char* file = element->Attribute("file");
			if (file != 0){
				layout->load(element->Attribute("file"));
			}else{
				layout->load(element);
			}
		}

		element = element->NextSiblingElement();
	}

	changeCurrentLayout(mLayouts.at(0));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//	GUILayout
//

GUILayout::GUILayout(int id,DxResourceGroup* group,DxRenderer* renderer)
	:mID(id),mRenderer(renderer),mGUIResourceGroup(group),mParent(0)
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
		{(float)x+width,(float)y,0.0f,1.0f,1.0f,0.0f}
		,{(float)(x+width),(float)(y+height),0.0f,1.0f,1.0f,1.0f}
		,{(float)x,(float)y,0.0f,1.0f,0.0f,0.0f}
		,{(float)x,(float)(y+height),0.0f,1.0f,0.0f,1.0f}
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
		{(float)x+width,(float)y,0.0f,1.0f,1.0f,0.0f}
		,{(float)(x+width),(float)(y+height),0.0f,1.0f,1.0f,1.0f}
		,{(float)x,(float)y,0.0f,1.0f,0.0f,0.0f}
		,{(float)x,(float)(y+height),0.0f,1.0f,0.0f,1.0f}
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

void GUILayout::load(const char* file)
{
	TiXmlDocument doc(file);
	if (!doc.LoadFile())
	{
		assert(false && "file does not exists or file format error");
	}
	TiXmlElement* root = doc.RootElement();
	assert(root != 0 && strcmp("DxGUI",root->Value()) == 0);
	std::cout << root->Value() << std::endl;

	TiXmlElement* element = root->FirstChildElement();

	assert(strcmp("Layout",element->Value()) == 0 && "not a layout file");
	assert(mID == atoi(element->Attribute("id")) && "id does not match");
	load(element);
}

void GUILayout::load(const TiXmlElement* root)
{
	const TiXmlElement* element = root->FirstChildElement();
	const DxParam* dxparam = GUISystem::getSingletonPtr()->getDxParam();
	int width = dxparam->width;
	int height = dxparam->height;
	while (element != 0)
	{
		if (strcmp("Button",element->Value()) ==0){
			GUIButton btn;
			parseControlAttribute(element,&btn,width,height);
			const char* up,*down,*over = 0;
			const TiXmlElement* textureElement = element->FirstChildElement();
			while (textureElement != 0){
				if (strcmp("up",textureElement->Value()) ==0){
					up = textureElement->Attribute("file");
				}else if (strcmp("down",textureElement->Value()) ==0){
					down = textureElement->Attribute("file");
				}else if (strcmp("over",textureElement->Value()) ==0){
					over = textureElement->Attribute("file");
				}
				textureElement = textureElement->NextSiblingElement();
			}
			GUIButton* newButton = createButton(btn.x,btn.y,btn.width,btn.height,btn.id);
			newButton->setTexture(up,down,over);
		}
		else if (strcmp("Label",element->Value()) ==0){
			GUILabel label;
			parseControlAttribute(element,&label,width,height);
			const char* text = element->Attribute("text");
			int fontId = 0;
			const char* fontStr = element->Attribute("font");
			fontId = atoi(fontStr);
			unsigned long color = 0xffffffff;
			const char* colorStr = element->Attribute("color");
			if (colorStr != 0){
				color = strtoul(colorStr,0,16);
			}
			GUILabel* newLabel = createLabel(label.x,label.y,label.width,label.height,label.id,fontId);
			newLabel->setText(text);
			newLabel->setColor(color);
		}
		else if (strcmp("Image",element->Value()) ==0){
			GUIImage img;
			parseControlAttribute(element,&img,width,height);
			const char* file = element->Attribute("file");
			GUIImage* newImg = createImage(img.x,img.y,img.width,img.height,img.id);
			newImg->setImage(file);
		}
		else
		{
			assert(false && "parse layout xml error");
		}

		element = element->NextSiblingElement();
	}
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
	mText = new char[1024];
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
	font->DrawTextA(0,mText,-1,&rect,DT_TOP | DT_LEFT,mColor);
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
	renderer->enableTransparent();
	renderer->applyTexture(0,mImage);
	renderer->render(mBuffer);
	renderer->disableTransparent();
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
	mState = GUI_BUTTON_UP;
	if (mx >= x && mx <= x+width && my >= y && my <= y + height){
		if (mouseDown){
			mState = GUI_BUTTON_DWON;
		}
		else{
			mState = GUI_BUTTON_OVER;
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
	case GUI_BUTTON_OVER:
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