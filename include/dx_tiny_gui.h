#ifndef __DX_TINY_GUI__
#define __DX_TINY_GUI__

#include "dx_resource_manager.h"
#include "dx_renderer.h"
#include <vector>

class GUISystem;

struct GUIVertex
{
	float x,y,z,rhw;
	float u,v;
	static const DWORD GUI_FVF = D3DFVF_XYZRHW | D3DFVF_TEX0;
};



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
		GUI_BUTTON_MOVE = 2
	};
public:
	GUIButton()
	{
		mUpTexture = 0;
		mDownTexture = 0;
		mMoveTexture = 0;
		mStaticBuffer = 0;
		mClickListener = 0;
		mState = GUI_BUTTON_UP;
	}

		
	void	setClickListener(OnClick clickListener) {mClickListener = clickListener;}
	void	setStaticBuffer(DxBuffer* buffer) {mStaticBuffer = buffer;}
	void	setTexture( DxTexture* up, DxTexture* down, DxTexture* move)
	{
		mUpTexture = up;
		mMoveTexture = move;
		mDownTexture = down;
	}
	void	callback(int x,int y,bool mouseDown);

	void	preRender(DxRenderer* renderer);
	void	onRender(DxRenderer* renderer);
	void	postRender(DxRenderer* renderer);
private:
	DxTexture*		mUpTexture;
	DxTexture*		mDownTexture;
	DxTexture*		mMoveTexture;
	DxBuffer*		mStaticBuffer;
	OnClick			mClickListener;
	GUIButtonState	mState;
};

class GUILabel : public GUIControl
{
public:
	GUILabel();
	~GUILabel();
	void	setText(const char* text);

	//
	//maybe unsafe
	//
	void	setText(const char* s,...);

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

	void	setImage(DxTexture* tex) { mImage = tex;}
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
	GUILabel*	createLabel(int x,int y,int width,int height,int id,int fontID);
	GUIButton*	createButton(int x,int y,int width,int height,int id);
	GUIImage*	createImage(int x,int y,int width,int height,int id);

	GUIControl*	getControlById(int id);
	void		destroyControl(int id);
	bool		load(const char* fileName);
	void		processGUI(int x,int y,bool mouseDown);
	void		render();

	int			getID() {return mID;}
private:
	typedef std::vector<GUIControl*>	GUIContainer;
	typedef GUIContainer::iterator		GUIIter;

	int							mID;
	GUIContainer				mGUIControls;
	DxResourceGroup*			mGUIResourceGroup;
	DxRenderer*					mRenderer;

	GUILayout(int id,DxResourceGroup* group,DxRenderer* renderer);
};




//将font在guisystem里管理并不好，而且还把device耦合进来了，
//不过这里为了简化，并不实现复杂的font，就直接在System里通过id来管理font

class FontManager
{
public:
	static FontManager*	getSingletonPtr()
	{
		static FontManager*	mSelf = new FontManager;
		return mSelf;
	}

	bool				createFont(int id);
	ID3DXFont*			getFontByID(int id);
	void				releaseFont(int id);
	void				release();
private:
	typedef std::vector<ID3DXFont*>		FontContainer;
	typedef FontContainer::iterator		FontIter;


	FontContainer						mFonts;
};

class GUISystem
{
public:
				GUISystem(DxRenderer* renderer,DxResourceManager* resMgr);

	GUILayout*	createLayout(int id);
	GUILayout*	loadLayout(const char* layerFileName);
	GUILayout*	getLayout(int id);
	void		destroyLayout(int id);

private:
	typedef std::vector<GUILayout*>		LayoutContainer;	
	typedef LayoutContainer::iterator	LayoutIter;
	

	DxRenderer*				mRenderer;
	DxResourceManager*		mResourceMgr;
	LayoutContainer			mLayouts;
};
#endif