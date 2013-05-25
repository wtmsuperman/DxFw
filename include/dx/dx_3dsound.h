#ifndef __DX_3D_SOUND__

#include <dx/dx_d3d_input.h>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <list>
#include <map>
#include <node/node.h>

#define	DX_SOUND_MAX_CHANNEL 200
#define DX_SOUND_MAX_GROUPS	10
#define INVALID_SOUND_INDEX      0
#define INVALID_SOUND_CHANNEL    -1

bool ERRCHECK(FMOD_RESULT result);

enum SoundType
{
	SOUND_TYPE_3D,
	SOUND_TYPE_2D,
};

class DxSoundSystem
{
public:
	struct ChannelInstance
	{
		Vector3*	bindingPosition;
		Vector3		prevPosition;
	};
public:
	static DxSoundSystem* getSingletonPtr()
	{
		static DxSoundSystem* self = new DxSoundSystem;
		return self;
	}

public:
	bool	init();
	bool	load(const char* file);
	void	release();

	void	update(float delta);

	void	setDistanceFactor(float distanceFactor);
	void	setGroupVolume(int group,float volume);
	void	setGroupPaused(int group,bool paused);
	void	stopGroup(int group);
	void	stop(int channelIndex);
	void	setVolume(int channelIndex,float volume);
	void	setPaused(int channelIndex,bool paused);

	void	setListener(Node* listenerNode);
public:
	int		playSound(int soundId,int group,const Vector3& position);
	int		playSound(int soundId,int group,Vector3* bindingPosition);
public:
	FMOD::Sound*			getSound(int id);
	FMOD::ChannelGroup*		getChannelGroup(int group);
	void					createSound(const char* file,SoundType type,bool loop,int id);
private:
	FMOD::System*		mSystem;
	FMOD_SPEAKERMODE	mSpeakermode;
    FMOD_CAPS			mCaps;
    char				mName[256];
	int					mNumdrivers;
	float				mDistanceFactor;
	int					mMaxChannel;
	ChannelInstance		mChannelInstances[DX_SOUND_MAX_CHANNEL];
	FMOD::ChannelGroup*	mChannelGroups[DX_SOUND_MAX_GROUPS];
	Node*				mListener;
	Vector3				mPrevListenerPosition;
private:
	typedef std::map<int,FMOD::Sound*>			SoundMap;
	SoundMap									mSounds;
private:
	DxSoundSystem()
	{}
	DxSoundSystem(const DxSoundSystem&);
	DxSoundSystem& operator=(const DxSoundSystem&);
};

#endif