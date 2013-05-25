#include <dx/dx_3dsound.h>

#pragma comment(lib,"fmodex_vc.lib")

bool ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
		char buffer[1024];
        sprintf_s(buffer,"FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		MessageBox(0,buffer,"Fmod",MB_OK);
		return false;
    }

	return true;
}


bool DxSoundSystem::init()
{
	FMOD_RESULT result;
	unsigned int version;

	result = FMOD::System_Create(&mSystem);
    if (!ERRCHECK(result))
		return false;
    
    result = mSystem->getVersion(&version);
    if (!ERRCHECK(result))
		return false;

    if (version < FMOD_VERSION)
    {
		char buffer[512];
        sprintf_s(buffer,"Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		MessageBox(0,buffer,"Fmode",MB_OK);
        return false;
    }
    
    result = mSystem->getNumDrivers(&mNumdrivers);
    if (!ERRCHECK(result))
	{
		return false;
	}

    if (mNumdrivers == 0)
    {
        result = mSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        if (!ERRCHECK(result))
			return false;
    }
    else
    {
        result = mSystem->getDriverCaps(0, &mCaps, 0, &mSpeakermode);
        if (!ERRCHECK(result))
			return false;

        result = mSystem->setSpeakerMode(mSpeakermode);       /* Set the user selected speaker mode. */
        if (!ERRCHECK(result))
			return false;

        if (mCaps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
        {                                                   /* You might want to warn the user about this. */
            result = mSystem->setDSPBufferSize(1024, 10);
            if (!ERRCHECK(result))
				return false;
        }

        result = mSystem->getDriverInfo(0, mName, 256, 0);
        if (!ERRCHECK(result))
			return false;

        if (strstr(mName, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
        {
            result = mSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            if (!ERRCHECK(result))
				return false;
        }
    }

    result = mSystem->init(DX_SOUND_MAX_CHANNEL, FMOD_INIT_NORMAL, 0);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
    {
        result = mSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        if (!ERRCHECK(result))
			return false;
            
        result = mSystem->init(100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        if (!ERRCHECK(result))
			return false;
    }

	FMOD::ChannelGroup*	masterGroup;
	mSystem->getMasterChannelGroup(&masterGroup);
	for (int i=0; i<DX_SOUND_MAX_GROUPS; ++i)
	{
		result = mSystem->createChannelGroup("Group A", &mChannelGroups[i]);
		if (!ERRCHECK(result))
			return false;
		masterGroup->addGroup(mChannelGroups[i]);
	}

	for (int i=0; i<DX_SOUND_MAX_CHANNEL; ++i)
	{
		mChannelInstances[i].bindingPosition = 0;
	}

	return true;
}

void DxSoundSystem::setDistanceFactor(float distanceFactor)
{
	mDistanceFactor = distanceFactor;
	assert(ERRCHECK(mSystem->set3DSettings(1.0, mDistanceFactor, 1.0f)));
}

void DxSoundSystem::setGroupVolume(int group,float volume)
{
	assert(ERRCHECK(mChannelGroups[group]->setVolume(volume)));
}

void DxSoundSystem::setGroupPaused(int group,bool paused)
{
	assert(ERRCHECK(mChannelGroups[group]->setPaused(paused)));
}

void DxSoundSystem::stopGroup(int group)
{
	int num;
	mChannelGroups[group]->getNumChannels(&num);
	FMOD::Channel* channel;
	for (int i=0; i<num; ++i)
	{
		mChannelGroups[group]->getChannel(i,&channel);
		int index;
		channel->getIndex(&index);
		mChannelInstances[index].bindingPosition = 0;
		channel->stop();
	}
}

void DxSoundSystem::stop(int channelIndex)
{
	if (channelIndex == INVALID_SOUND_CHANNEL)
      return;

   FMOD::Channel *soundChannel;

   assert((channelIndex > 0) && (channelIndex < DX_SOUND_MAX_CHANNEL));

   mSystem->getChannel(channelIndex, &soundChannel);
   soundChannel->stop();
   mChannelInstances[channelIndex].bindingPosition = 0;
}

void DxSoundSystem::setVolume(int channelIndex,float volume)
{
	if (channelIndex == INVALID_SOUND_CHANNEL)
      return;

   FMOD::Channel *soundChannel;

   assert((channelIndex > 0) && (channelIndex < DX_SOUND_MAX_CHANNEL));

   mSystem->getChannel(channelIndex, &soundChannel);
   soundChannel->setVolume(volume);
}

void DxSoundSystem::setPaused(int channelIndex,bool paused)
{
	if (channelIndex == INVALID_SOUND_CHANNEL)
      return;

   FMOD::Channel *soundChannel;

   assert((channelIndex > 0) && (channelIndex < DX_SOUND_MAX_CHANNEL));

   mSystem->getChannel(channelIndex, &soundChannel);
   soundChannel->setPaused(paused);
}

void DxSoundSystem::setListener(Node* node)
{
	this->mListener = node;
}

void DxSoundSystem::update(float delta)
{
	Vector3	currentPosition = mListener->getDerivedPosition();
	Vector3	vectorVelocity;
	if (delta > 0)
		vectorVelocity = (currentPosition - mPrevListenerPosition) / delta;
	else
      vectorVelocity = Vector3(0, 0, 0);

	Vector3 mup = mListener->getDerivedOrientation().yAxis();
	mup.normalize();
	Vector3 mforward = mListener->getDerivedOrientation().zAxis();
	mforward.normalize();
	FMOD_VECTOR pos = {currentPosition.x,currentPosition.y,currentPosition.z};
	FMOD_VECTOR up = {mup.x,mup.y,mup.z};
	FMOD_VECTOR forward = {mforward.x,mforward.y,mforward.z};
	FMOD_VECTOR	vel = {vectorVelocity.x,vectorVelocity.y,vectorVelocity.z};
	mSystem->set3DListenerAttributes(0,&pos,&vel,&forward,&up);
	mSystem->update();
	mPrevListenerPosition = currentPosition;

	FMOD::Channel* nextChannel;
	for (int channelIndex = 0; channelIndex < DX_SOUND_MAX_CHANNEL; channelIndex++)
	{
		 if (mChannelInstances[channelIndex].bindingPosition != 0)
         {
			 mSystem->getChannel(channelIndex, &nextChannel);
			 if (delta > 0)
				vectorVelocity = (*mChannelInstances[channelIndex].bindingPosition - mChannelInstances[channelIndex].prevPosition) / delta;
			 else
				vectorVelocity = Vector3(0, 0, 0);

			 pos.x = mChannelInstances[channelIndex].bindingPosition->x;
			 pos.y = mChannelInstances[channelIndex].bindingPosition->y;
			 pos.z = mChannelInstances[channelIndex].bindingPosition->z;

			 vel.x = vectorVelocity.x;
			 vel.y = vectorVelocity.y;
			 vel.z = vectorVelocity.z;

			 nextChannel->set3DAttributes(&pos, &vel);
			 mChannelInstances[channelIndex].prevPosition = *mChannelInstances[channelIndex].bindingPosition;
         }
	}
}

FMOD::Sound* DxSoundSystem::getSound(int id)
{
	return mSounds.at(id);
}

FMOD::ChannelGroup* DxSoundSystem::getChannelGroup(int group)
{
	assert(group>-1 && group < DX_SOUND_MAX_GROUPS);
	return mChannelGroups[group];
}

int	DxSoundSystem::playSound(int soundId,int group,const Vector3& position)
{
	int index;
	FMOD::Channel*	channel;
	if (!ERRCHECK(mSystem->playSound(FMOD_CHANNEL_FREE,getSound(soundId),true,&channel)))
	{
		return INVALID_SOUND_CHANNEL;
	}

	channel->getIndex(&index);
	channel->setChannelGroup(getChannelGroup(group));
	mChannelInstances[index].bindingPosition = 0;
	FMOD_VECTOR pos = {position.x,position.y,position.z};
	channel->set3DAttributes(&pos,0);
	channel->setPaused(false);
	mChannelInstances[index].bindingPosition = 0;

	return index;
}

int	DxSoundSystem::playSound(int soundId,int group,Vector3* bindingPosition)
{
	int index;
	FMOD::Channel*	channel;
	if (!ERRCHECK(mSystem->playSound(FMOD_CHANNEL_FREE,getSound(soundId),true,&channel)))
	{
		return INVALID_SOUND_CHANNEL;
	}

	channel->getIndex(&index);
	channel->setChannelGroup(getChannelGroup(group));
	if (bindingPosition != 0)
	{
		mChannelInstances[index].bindingPosition = bindingPosition;
		mChannelInstances[index].prevPosition = *bindingPosition;
		FMOD_VECTOR pos = {bindingPosition->x,bindingPosition->y,bindingPosition->z};
		channel->set3DAttributes(&pos,0);
	}
	else
	{
		mChannelInstances[index].bindingPosition = 0;
	}
	channel->setPaused(false);
	return index;
}

void DxSoundSystem::createSound(const char* file,SoundType type,bool loop,int id)
{
	assert(mSounds.find(id) == mSounds.end());
	FMOD::Sound* sound;
	switch (type)
	{
	case SOUND_TYPE_2D:
		if (loop)
		{
			mSystem->createStream(file,FMOD_2D|FMOD_LOOP_NORMAL,0,&sound);
		}
		else
		{
			mSystem->createSound(file,FMOD_2D,0,&sound);
		}
		break;
	case SOUND_TYPE_3D:
		if (loop)
		{
			mSystem->createStream(file,FMOD_3D|FMOD_LOOP_NORMAL,0,&sound);
		}
		else
		{
			mSystem->createSound(file,FMOD_3D,0,&sound);
		}
		break;
	}
	sound->set3DMinMaxDistance(50.0f*mDistanceFactor,10000.0f*mDistanceFactor);
	mSounds[id] = sound;
}

void DxSoundSystem::release()
{
	typedef std::map<int,FMOD::Sound*>::iterator Iter;
	Iter end = mSounds.end();
	for (Iter iter=mSounds.begin(); iter != end; ++iter)
	{
		iter->second->release();
	}

	for (int i=0; i<DX_SOUND_MAX_GROUPS; ++i)
	{
		mChannelGroups[i]->release();
	}

	mSystem->release();
	mSystem->close();
}