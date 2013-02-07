#ifndef __ACT_ACTION__
#define __ACT_ACTION__

#include <string.h>
#include <malloc.h>

struct IAction
{
	virtual ~IAction(){}
	virtual bool act(float delta) = 0;
};

struct ICondition
{
	virtual ~ICondition(){}
	virtual bool onCondition(float delta) = 0;
};

class ParallelAction : public IAction
{
public:
	ParallelAction(IAction** actions,size_t size)
	{
		mActions = (IAction**) malloc(size * sizeof(IAction*));
		memcpy(mActions,actions,size * sizeof(IAction*));
		mActionSize = size;
	}

	virtual ~ParallelAction()
	{
		if (mActions != 0)
		{
			for (size_t i=0; i<mActionSize; ++i){
				delete mActions[i];
			}
			free(mActions);
		}
	}

	virtual bool act(float delta)
	{
		bool f = true;
		for (size_t i=0; i<mActionSize; ++i){
			f ^= mActions[i]->act(delta);
		}
		return f == true;
	}

	size_t	actionSize() const {return mActionSize;}

private:
	IAction**	mActions;
	size_t		mActionSize;
};

class ConditionalAction : public IAction
{
public:
	ConditionalAction(IAction* action,ICondition* condition)
	{
		mAction = action;
		mCondition = condition;
	}

	virtual ~ConditionalAction()
	{
		delete mAction;
		delete mCondition;
	}

	virtual bool act(float delta)
	{
		if (mCondition->onCondition(delta)){
			return mAction->act(delta);
		}
	}
private:
	IAction*		mAction;
	ICondition*		mCondition;
};

class SequenceActions : public IAction
{
public:
	SequenceActions(IAction** actions,size_t size)
	{
		mActions = (IAction**) malloc(size * sizeof(IAction*));
		memcpy(mActions,actions,size * sizeof(IAction*));
		mActionSize = size;
		mCurrentActionIndex = 0;
	}

	virtual ~SequenceActions()
	{
		if (mActions)
			free(mActions);
	}

	virtual bool act(float delta)
	{
		if (mCurrentActionIndex == mActionSize)
			return true;
		else
		{
			if (mActions[mCurrentActionIndex]->act(delta))
			{
				mCurrentActionIndex++;
			}
		}
		return false;
	}

private:
	IAction**	mActions;
	size_t		mActionSize;
	size_t		mCurrentActionIndex;
};

class OnTimeCondition : public ICondition
{
public:
	float time;
	float currentTime;

	OnTimeCondition(float time)
	{
		this->time = time;
		currentTime = 0.0f;
	}
	virtual ~OnTimeCondition(){}

	virtual bool onCondition(float delta)
	{
		currentTime += delta;
		return currentTime >= time;
	}
};

#endif