#ifndef __DX_TINY_GUI__
#define __DX_TINY_GUI__

#include "dx/dx_dxfw.h"
#include "dx/dx_renderer.h"
#include "dx/dx_resource_manager.h"
#include <vector>
#include "script/scriptlib.h"
#include "dx_vertex_structs.h"

class GUISystem;
class GUILayout;


struct GUIControl : IRenderable
{
	enum GUI_CONTROL_TYPE
	{
		GUI_BUTTON,
		GUI_LABEL,
		GUI_IMAGE
	};

	int					x;
	int					y;
	int					width;
	int					height;
	int					id;
	GUILayout*			layout;
	GUI_CONTROL_TYPE	type;


	virtual ~GUIControl(){}
	virtual void callback(int x,int y,bool mouseDown) = 0;
};

class GUIButton : public GUIControl
{
public:
	typedef void (*OnClick)(int id);
	enum GUIButtonState
	{
		GUI_BUTTON_UP = 0,
		GUI_BUTTON_DWON = 1,
		GUI_BUTTON_OVER = 2
	};
public:
	GUIButton()
	{
		mUpTexture = 0;
		mDownTexture = 0;
		mOverTexture = 0;
		mStaticBuffer = 0;
		mClickListener = 0;
		mState = GUI_BUTTON_UP;
	}

		
	void	setClickListener(OnClick clickListener) {mClickListener = clickListener;}
	void	setStaticBuffer(DxBuffer* buffer) {mStaticBuffer = buffer;}
	void	setTexture(const char* up,const char* down,const char* over);
	void	callback(int x,int y,bool mouseDown);

	void	preRender(DxRenderer* renderer);
	void	onRender(DxRenderer* renderer);
	void	postRender(DxRenderer* renderer);
private:
	DxTexture*		mUpTexture;
	DxTexture*		mDownTexture;
	DxTexture*		mOverTexture;
	DxBuffer*		mStaticBuffer;
	OnClick			mClickListener;
	GUIButtonState	mState;
};

class GUILabel : public GUIControl
{
public:
	GUILabel();
	~GUILabel();
	void			setText(const char* text);
	char*			getText() { return mText;}
	const char*		getText() const { return mText;}

	//
	//maybe unsafe,run like sprintf,but the max char array size is 1024
	//
	void	printf(const char* s,...);

	void	setColor(DWORD color);
	void	setFont(int fontID);
	void	callback(int x,int y,bool mouseDown);
	
	void	preRender(DxRenderer* renderer);
	void	onRender(DxRenderer* renderer);
	void	postRender(DxRenderer* renderer);
private:
	char*			mText;
	DWORD			mColor;
	int				mFontID;
};

class GUIImage : public GUIControl
{
public:
	GUIImage(){
		mImage = 0;
		mBuffer = 0;
	}

	void	setImage(const char* img);
	void	callback(int x,int y,bool mouseDown) {}
	void	setStaticBuffer(DxBuffer* buffer){ mBuffer = buffer;}

	void	preRender(DxRenderer* renderer) {}
	void	onRender(DxRenderer* renderer);
	void	postRender(DxRenderer* renderer) {}
private:
	DxTexture*	mImage;
	DxBuffer*	mBuffer;
};

class GUILayout
{
	friend class GUISystem;
public:
	GUILabel*			createLabel(int x,int y,int width,int height,int id,int fontID);
	GUIButton*			createButton(int x,int y,int width,int height,int id);
	GUIImage*			createImage(int x,int y,int width,int height,int id);

	GUIControl*			getControlById(int id);
	DxResourceGroup*	getResourceGroup() {return mGUIResourceGroup;}
	int					getID() {return mID;}

	void				destroyControl(int id);
	void				load(const char* fileName); 
	void				load(const TiXmlElement* root);
	void				processGUI(int x,int y,bool mouseDown);
	void				render();
	void				notifyParent(GUISystem* parent) {mParent = parent;}
private:
	typedef std::vector<GUIControl*>	GUIContainer;
	typedef GUIContainer::iterator		GUIIter;

	int							mID;
	GUIContainer				mGUIControls;
	DxResourceGroup*			mGUIResourceGroup;
	DxRenderer*					mRenderer;

	GUISystem*					mParent;

	GUILayout(int id,DxResourceGroup* group,DxRenderer* renderer);
	GUILayout(const GUILayout& r);
	GUILayout& operator=(const GUILayout& r);
};




//将font在guisystem里管理并不好，而且还把device耦合进来了，
//不过这里为了简化，并不实现复杂的font，就直接在System里通过id来管理font

struct DxFont
{
	ID3DXFont*	font;
	int			id;
};

class GUISystem
{
public:		
						~GUISystem(){ release();}
	//initialize gui system,call it before use gui system
    void				initOnce(const DxFw& fw);
	void				load(const char* xml);
	GUILayout*			createLayout(int id);
	GUILayout*			getLayout(int id);
	void				destroyLayout(int id);
	GUILayout*			currentLayout() {return mCurrentLayout;}

	// change current layout,return the last layout
	GUILayout*			changeCurrentLayout(int id);
	GUILayout*			changeCurrentLayout(GUILayout* layout);
	
	// render current layout
	// current layout could change by call changeCurrentLayout(int id);
	void				render() {if (mCurrentLayout!=0) mCurrentLayout->render(); }
	// process gui
	void				processGUI(int x,int y,bool mouseDown) {if (mCurrentLayout!=0) mCurrentLayout->processGUI(x,y,mouseDown);}
	//release befor frame work
	void				release();

	DxFont*				createFont(const char* fontName,int weight,bool italic,int size,int id);
	DxFont*				getFontByID(int id);
	void				releaseFont(int id);

	const DxParam*		getDxParam() const {return mDxParam;}

	static inline GUISystem*	getSingletonPtr();

private:
	typedef std::vector<GUILayout*>		LayoutContainer;	
	typedef LayoutContainer::iterator	LayoutIter;
	typedef std::vector<DxFont*>		FontContainer;
	typedef FontContainer::iterator		FontIter;
	
	GUILayout*				mCurrentLayout;
	DxRenderer*				mRenderer;
	const DxParam*			mDxParam;
	IDirect3DDevice9*		mDevice;
	DxResourceManager*		mResourceMgr;
	LayoutContainer			mLayouts;
	FontContainer			mFonts;

	GUISystem();
	GUISystem(const GUISystem& r);
	GUISystem& operator=(const GUISystem& r);
};

GUISystem*	GUISystem::getSingletonPtr()
{
	static GUISystem* self = new GUISystem;
	return self;
}

#endif