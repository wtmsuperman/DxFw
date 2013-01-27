#include "dx_resource_manager.h"

const char* DEFAULT_GROUP_NAME = "default";

DxResourceGroup::DxResourceGroup(IDirect3DDevice9* device,const char* groupName)
{
	mDevice = device;
	mGroupName = new char[strlen(groupName) + 1];
	strcpy(mGroupName,groupName);
}

DxResourceGroup::~DxResourceGroup()
{
	release();
	safe_deleteArray(mGroupName);
}

DxBuffer* DxResourceGroup::createStaticBuffer(const char* name,DWORD fvf,D3DPRIMITIVETYPE primType
							,unsigned int vertexNum,unsigned int indicesNum
							,unsigned int stride
							,void** data,unsigned int* indices)
{
	assert(mBuffers.find(name) == mBuffers.end() && "already have this static buffer");

	DxBuffer buffer;

	ZeroMemory(&buffer,sizeof(buffer));

	buffer.vertexNum = vertexNum;
	buffer.indiceNum = indicesNum;
	buffer.fvf = fvf;
	buffer.type = primType;
	buffer.stride = stride;
	void* ptr = 0;
	if (indices > 0)
	{
		if (FAILED(mDevice->CreateIndexBuffer(sizeof(UINT)*indicesNum,D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&buffer.ib,0)))
		{
			//log("create index failed");
			return 0;
		}
		if (FAILED(buffer.ib->Lock(0,0,&ptr,0)))
		{
			//log("Lock indices failed");
			return 0;
		}
		memcpy(ptr,indices,indicesNum * sizeof(UINT));
		buffer.ib->Unlock();
	}
	else
	{
		buffer.ib = 0;
	}

	if (FAILED(mDevice->CreateVertexBuffer(stride*vertexNum,D3DUSAGE_WRITEONLY,buffer.fvf,D3DPOOL_DEFAULT,&buffer.vb,0)))
	{
		//log("create vertex failed");
		return 0;
	}
	if (FAILED(buffer.vb->Lock(0,0,&ptr,0)))
	{
		//log("Lock vertices fialed");
		return 0;
	}

	memcpy(ptr,data,vertexNum * stride);
	buffer.vb->Unlock();

	buffer.name = new char[strlen(name) + 1];
	strcpy(buffer.name,name);
	buffer.groupName = mGroupName;

	mBuffers[name] = buffer;
	
	return &mBuffers[buffer.name];
}

DxTexture* DxResourceGroup::loadTexture(const char* fileName)
{

	assert(mTextures.find(fileName) == mTextures.end() && "already exists this texture");
	if (fileName == 0)
		return 0;

	size_t fl = strlen(fileName);
	if (fl == 0)
		return 0;

	DxTexture d3dTexture;
	d3dTexture.name = new char[fl+1];
	strcpy(d3dTexture.name,fileName);
	D3DXIMAGE_INFO info;
	D3DCOLOR colorkey = 0xff000000;
	if (FAILED(D3DXCreateTextureFromFileEx(mDevice,fileName,0,0,0,0
		,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT
		,colorkey
		,&info,0,&d3dTexture.data)))
	{
		return 0;
	}
	d3dTexture.data->SetAutoGenFilterType(D3DTEXF_LINEAR);
	d3dTexture.data->GenerateMipSubLevels();

	d3dTexture.height = info.Height;
	d3dTexture.width = info.Width;
	d3dTexture.groupName = mGroupName;

	mTextures[fileName] = d3dTexture;
	return &mTextures[fileName];
}

DxMaterial* DxResourceGroup::createMaterial(const char* name,const D3DMATERIAL9& mtrl,DxTexture** textures,size_t len)
{
	assert(mMaterials.find(name) == mMaterials.end() && "already exists this material");

	DxMaterial dxMtrl;
	dxMtrl.mtrl = mtrl;
	memset(dxMtrl.textures,0,sizeof(DxTexture*));

	for (size_t i=0; i<len; ++i)
	{
		dxMtrl.textures[i] = textures[i];
	}

	dxMtrl.groupName = mGroupName;
	dxMtrl.name = new char[strlen(name) + 1];
	strcpy(dxMtrl.name,name);
	mMaterials[name] = dxMtrl;
	return &mMaterials[name];
}

DxMaterial* DxResourceGroup::createMaterial(const char* name,const D3DMATERIAL9& mtrl,DxTexture* texture)
{
	assert(mMaterials.find(name) == mMaterials.end() && "already exists this material");

	DxMaterial dxMtrl;
	dxMtrl.mtrl = mtrl;
	memset(dxMtrl.textures,0,sizeof(DxTexture*));
	dxMtrl.textures[0] = texture;
	dxMtrl.groupName = mGroupName;
	dxMtrl.name = new char[strlen(name) + 1];
	strcpy(dxMtrl.name,name);
	mMaterials[name] = dxMtrl;
	return &mMaterials[name];
}

DxMesh*	DxResourceGroup::createMesh(const char* name,ID3DXMesh* mesh)
{
	assert(mMeshes.find(name) == mMeshes.end() && "already exists this mesh");

	DxMesh	dxmesh;
	dxmesh.groupName = mGroupName;
	dxmesh.meshData = mesh;
	dxmesh.name = new char[strlen(name) + 1];
	strcpy(dxmesh.name,name);
	mMeshes[name] = dxmesh;
	return &mMeshes[name];
}

DxModel* DxResourceGroup::loadXModel(const char* fileName)
{
	assert(mModels.find(fileName) == mModels.end() && "already exists this model");

	if (fileName == 0)
		return 0;

	size_t fl = strlen(fileName);
	if (fl == 0)
		return 0;

	DxModel model;
	model.groupName = mGroupName;
	model.name = new char[strlen(fileName) + 1];
	strcpy(model.name,fileName);

	HRESULT hr = 0;

	ID3DXMesh* mesh = 0;

    ID3DXBuffer* adjBuffer  = 0;
    ID3DXBuffer* mtrlBuffer = 0;
	DWORD        numMtrls   = 0;

	hr = D3DXLoadMeshFromX(  
		fileName,
		D3DXMESH_MANAGED,
		mDevice,
		&adjBuffer,
		&mtrlBuffer,
		0,
		&numMtrls,
		&mesh);

	if(FAILED(hr))
	{
		return 0;
	}

	model.mesh = createMesh(fileName,mesh);
	model.mtrls = new DxMaterial*[numMtrls];
	model.faceNum = numMtrls;
	
	//
	// Extract the materials, and load textures.
	//

	if( mtrlBuffer != 0 && numMtrls != 0 )
	{
		D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();
		for(DWORD i = 0; i < numMtrls; ++i)
		{
			char mtrlName[512];
			sprintf(mtrlName,"%s_%d",fileName,i);
			// the MatD3D property doesn't have an ambient value set
			// when its loaded, so set it now:
			mtrls[i].MatD3D.Ambient = mtrls[i].MatD3D.Diffuse;

			// save the ith material
			
			DxTexture* tex = 0;
			// check if the ith material has an associative texture
			if( mtrls[i].pTextureFilename != 0 )
			{
				// yes, load the texture for the ith subset
				DxTexture* tex = loadTexture(mtrls[i].pTextureFilename);
			}

			model.mtrls[i] = createMaterial(mtrlName,mtrls[i].MatD3D,tex);
		}
	}

	safe_Release(mtrlBuffer); // done w/ buffer

	//
	// Optimize the mesh.
	//

	hr = mesh->OptimizeInplace(		
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT  |
		D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjBuffer->GetBufferPointer(),
		0, 0, 0);

	safe_Release(adjBuffer); // done w/ buffer

	mModels[fileName] = model;
	return &mModels[fileName];
}

void DxResourceGroup::releaseBuffer(const char* name)
{
	assert(mBuffers.find(name) != mBuffers.end() && "do not have that buffer");
	BufferIter iter = mBuffers.find(name);
	DxBuffer& buffer = iter->second;
	safe_Release(buffer.ib);
	safe_Release(buffer.vb);
	safe_deleteArray(buffer.name);
	mBuffers.erase(iter);
}

void DxResourceGroup::releaseMaterial(const char* name)
{
	assert(mMaterials.find(name) != mMaterials.end() && "do not have that material ");
	MaterialIter iter = mMaterials.find(name);
	DxMaterial& mtrl = iter->second;
	safe_deleteArray(mtrl.name);
	for (DWORD i=0; i<MAX_TEXTURE; ++i)
	{
		releaseTexture(mtrl.textures[i]->name);
	}
	mMaterials.erase(iter);
}


void DxResourceGroup::releaseTexture(const char* name)
{
	assert(mTextures.find(name) != mTextures.end() && "do not have that texture");
	TextureIter iter = mTextures.find(name);
	DxTexture& tex = iter->second;
	safe_Release(tex.data);
	safe_deleteArray(tex.name);
	mTextures.erase(iter);
}

void DxResourceGroup::releaseMesh(const char* name)
{
	assert(mMeshes.find(name) != mMeshes.end() && "do not have that mesh");
	MeshIter iter = mMeshes.find(name);
	DxMesh& mesh = iter->second;
	safe_Release(mesh.meshData);
	safe_deleteArray(mesh.name);
	mMeshes.erase(iter);
}

void DxResourceGroup::releaseModel(const char* name)
{
	assert(mModels.find(name) != mModels.end() && "do not have that x model");

	ModelIter iter = mModels.find(name);
	DxModel& model = iter->second;
	releaseMesh(model.mesh->name);
	for (unsigned int i=0; i<model.faceNum; ++i)
	{
		releaseMaterial(model.mtrls[i]->name);
	}
	safe_deleteArray(model.mtrls);
	safe_deleteArray(model.name);
	mModels.erase(iter);
}

void DxResourceGroup::releaseAllBuffer()
{
	BufferIter end = mBuffers.end();
	for (BufferIter beg = mBuffers.begin(); beg != end; ++beg)
	{
		DxBuffer& buffer = beg->second;

		safe_Release(buffer.ib);
		safe_Release(buffer.vb);
		safe_deleteArray(buffer.name);
	}

	mBuffers.clear();
}

void DxResourceGroup::releaseAllMaterial()
{
	MaterialIter end = mMaterials.end();
	for (MaterialIter iter = mMaterials.begin(); iter != end; ++iter)
	{
		DxMaterial& mtrl = iter->second;

		safe_deleteArray(mtrl.name);
		for (DWORD i=0; i<MAX_TEXTURE; ++i)
		{
			if (mtrl.textures[i] != 0)
				releaseTexture(mtrl.textures[i]->name);
		}
	}

	mMaterials.clear();
}

void DxResourceGroup::releaseAllTexture()
{
	TextureIter end = mTextures.end();
	for (TextureIter iter = mTextures.begin(); iter != end; ++iter)
	{
		DxTexture& tex = iter->second;

		safe_Release(tex.data);
		safe_deleteArray(tex.name);
	}

	mTextures.clear();
}

void DxResourceGroup::releaseAllMesh()
{
	MeshIter end = mMeshes.end();
	for (MeshIter iter = mMeshes.end(); iter != end; ++iter)
	{
		DxMesh& mesh = iter->second;

		safe_Release(mesh.meshData);
		safe_deleteArray(mesh.name);
	}

	mMeshes.clear();
}

void DxResourceGroup::releaseAllModel()
{
	ModelIter end = mModels.end();
	for (ModelIter iter = mModels.end(); iter != end; ++iter)
	{
		DxModel& model = iter->second;
		releaseMesh(model.mesh->name);
		for (unsigned int i=0; i<model.faceNum; ++i)
		{
			releaseMaterial(model.mtrls[i]->name);
		}
		safe_deleteArray(model.name);
		safe_deleteArray(model.mtrls);
	}

	mModels.clear();
}

void DxResourceGroup::release()
{
	releaseAllBuffer();
	releaseAllTexture();
	releaseAllMaterial();
	releaseAllMesh();
	releaseAllModel();
}

/////////////////////////////////////////////////////////////////////////////////////////////

DxResourceManager::DxResourceManager(IDirect3DDevice9* device)
{
	mDevice = device;
	createResourceGroup(DEFAULT_GROUP_NAME);
}

DxResourceGroup* DxResourceManager::createResourceGroup(const char*name)
{
	assert(mResourceGroups.find(name) == mResourceGroups.end() && "assert already exists that group");

	DxResourceGroup* group = new DxResourceGroup(mDevice,name);
	mResourceGroups[name] = group;
	return group;
}

DxBuffer* DxResourceManager::createStaticBuffer(const char* groupName,const char* name
							,DWORD fvf,D3DPRIMITIVETYPE primType
							,unsigned int vertexNum,unsigned int indicesNum
							,unsigned int stride
							,void** data,unsigned int* indices)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");

	return mResourceGroups[groupName]->createStaticBuffer(name,fvf,primType
							,vertexNum,indicesNum
							,stride
							,data,indices);
}

DxMaterial* DxResourceManager::createMaterial(const char* groupName,const char* name,const D3DMATERIAL9& mtrl,DxTexture* texture)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");

	return mResourceGroups[groupName]->createMaterial(name,mtrl,texture);
}

DxMaterial* DxResourceManager::createMaterial(const char* groupName,const char* name,const D3DMATERIAL9& mtrl,DxTexture** textures,size_t len)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");

	return mResourceGroups[groupName]->createMaterial(name,mtrl,textures,len);
}

DxTexture* DxResourceManager::loadTexture(const char* groupName,const char* fileName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");

	return mResourceGroups[groupName]->loadTexture(fileName);
}

DxModel* DxResourceManager::loadXModel(const char* groupName,const char* fileName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");

	return mResourceGroups[groupName]->loadXModel(fileName);
}

DxMesh* DxResourceManager::createMesh(const char* groupName,const char* name,ID3DXMesh* mesh)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");

	return mResourceGroups[groupName]->createMesh(name,mesh);
}

void DxResourceManager::releaseBuffer(const char* groupName,const char* name)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseBuffer(name);
}

void DxResourceManager::releaseTexture(const char* groupName,const char* name)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseTexture(name);
}

void DxResourceManager::releaseMaterial(const char* groupName,const char* name)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseMaterial(name);
}

void DxResourceManager::releaseMesh(const char* groupName,const char* name)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseMesh(name);
}

void DxResourceManager::releaseModel(const char* groupName,const char* name)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseModel(name);
}

void DxResourceManager::releaseAllBuffer(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseAllBuffer();
}

void DxResourceManager::releaseAllMaterial(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseAllMaterial();
}

void DxResourceManager::releaseAllMesh(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseAllMesh();
}

void DxResourceManager::releaseAllTexture(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseAllTexture();
}

void DxResourceManager::releaseAllModel(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->releaseAllModel();
}

void DxResourceManager::releaseResource(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	mResourceGroups[groupName]->release();
}

void DxResourceManager::releaseAndRemove(const char* groupName)
{
	assert(mResourceGroups.find(groupName) != mResourceGroups.end() && "do not have that group");
	ResourceGroupIter iter = mResourceGroups.find(groupName);
	iter->second->release();
	mResourceGroups.erase(iter);
}

void DxResourceManager::release()
{
	ResourceGroupIter end = mResourceGroups.end();
	for (ResourceGroupIter iter = mResourceGroups.begin(); iter != end; ++iter)
	{
		safe_delete(iter->second);
	}

	mResourceGroups.clear();
}

DxResourceManager::~DxResourceManager()
{
	release();
}


