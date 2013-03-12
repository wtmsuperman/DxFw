#ifndef __DX_RESOURCE_MANAGER__
#define __DX_RESOURCE_MANAGER__

#include <string>
#include <unordered_map>
#include "dx_resource.h"
#include "dx_defines.h"

extern const char* DEFAULT_GROUP_NAME;

class DxResourceManager;

class DxResourceGroup
{

	friend class DxResourceManager;
public:
	DxBuffer*	createStaticBuffer(const char* name,DWORD fvf,D3DPRIMITIVETYPE primType
								,unsigned int vertexNum,unsigned int indicesNum
								,unsigned int stride
								,void** data,unsigned int* indices);
	DxTexture*	loadTexture(const char* fileName);
	DxMaterial* createMaterial(const char* name,const D3DMATERIAL9& mtrl,DxTexture** textures,size_t len);
	DxMaterial* createMaterial(const char* name,const D3DMATERIAL9& mtrl,DxTexture* texture);
	DxModel*	loadXModel(const char* fileName);
	DxMesh*		createMesh(const char* name,ID3DXMesh* mesh);

	void		release();
	void		releaseTexture(const char* name);
	void		releaseMaterial(const char* name);
	void		releaseModel(const char* name);
	void		releaseMesh(const char* name);
	void		releaseBuffer(const char* name);
	void		releaseAllTexture();
	void		releaseAllModel();
	void		releaseAllMesh();
	void		releaseAllMaterial();
	void		releaseAllBuffer();

	DxTexture*	getTexture(const char* name);
	DxModel*	getModel(const char* name);
	DxMaterial* getMaterial(const char* name);
	DxMesh*		getMesh(const char* name);
	DxBuffer*	getBuffer(const char* name);
	void		notifyParent(DxResourceManager* parent);

	~DxResourceGroup();
public:
	typedef std::string								str_key;
	typedef std::unordered_map<str_key,DxTexture>	TextureMap;
	typedef std::unordered_map<str_key,DxMesh>		MeshMap;
	typedef std::unordered_map<str_key,DxBuffer>	BufferMap;
	typedef std::unordered_map<str_key,DxMaterial>	MaterialMap;
	typedef std::unordered_map<str_key,DxModel>		ModelMap;

	typedef TextureMap::iterator					TextureIter;
	typedef MeshMap::iterator						MeshIter;
	typedef BufferMap::iterator						BufferIter;
	typedef MaterialMap::iterator					MaterialIter;
	typedef ModelMap::iterator						ModelIter;
private:

	TextureMap				mTextures;
	MeshMap					mMeshes;
	BufferMap				mBuffers;
	MaterialMap				mMaterials;
	ModelMap				mModels;
	char*					mGroupName;
	IDirect3DDevice9*		mDevice;

	DxResourceManager*		mParent;

	DxResourceGroup(IDirect3DDevice9* device,const char* name);

};

class DxResourceManager
{
public:
						DxResourceManager(IDirect3DDevice9* device);
						~DxResourceManager();
	DxResourceGroup*	createResourceGroup(const char* groupName);
	DxResourceGroup*	getResourceGroup(const char* groupName);

	DxBuffer*			createStaticBuffer(const char* groupName,const char* name,DWORD fvf,D3DPRIMITIVETYPE primType
										,unsigned int vertexNum,unsigned int indicesNum
										,unsigned int stride
										,void** data,unsigned int* indices);
	DxTexture*			loadTexture(const char* groupName,const char* fileName);
	DxMaterial*			createMaterial(const char* groupName,const char* name,const D3DMATERIAL9& mtrl,DxTexture** textures,size_t len);
	DxMaterial*			createMaterial(const char* groupName,const char* name,const D3DMATERIAL9& mtrl,DxTexture* texture);
	DxModel*			loadXModel(const char* groupName,const char* fileName);
	DxMesh*				createMesh(const char* groupName,const char* name,ID3DXMesh* mesh);

	
	void				releaseTexture(const char* groupName,const char* name);
	void				releaseMaterial(const char* groupName,const char* name);
	void				releaseModel(const char* groupName,const char* name);
	void				releaseMesh(const char* groupName,const char* name);
	void				releaseBuffer(const char* groupName,const char* name);
	void				releaseAllTexture(const char* groupName);
	void				releaseAllModel(const char* groupName);
	void				releaseAllMesh(const char* groupName);
	void				releaseAllMaterial(const char* groupName);
	void				releaseAllBuffer(const char* groupName);

	void				releaseResource(const char* groupName);
	DxResourceGroup*	remove(const char* groupName);

	void				release();
public:
	typedef std::string										str_key;
	typedef std::unordered_map<str_key,DxResourceGroup*>	ResourceMap;
	typedef ResourceMap::iterator							ResourceGroupIter;
private:
	ResourceMap			mResourceGroups;
	IDirect3DDevice9*	mDevice;
};

#endif