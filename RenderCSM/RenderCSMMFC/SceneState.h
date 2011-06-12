#ifndef __SCENE_STATE_H__
#define __SCENE_STATE_H__

class DXManager;

template < class entity_type >
class State
{
public:
	virtual void Enter( entity_type * ) = 0;
	virtual void Execute( entity_type * ) = 0;
	virtual void Exit( entity_type * ) = 0;
	virtual ~State(){}
};

class SceneState_Nothing : public State< DXManager >
{
private:
	SceneState_Nothing(){}
	SceneState_Nothing( const SceneState_Nothing& );
	SceneState_Nothing& operator=( const SceneState_Nothing& );

public:
	static SceneState_Nothing * Instance();
	
	virtual void Enter( DXManager * );
	virtual void Execute( DXManager * );
	virtual void Exit( DXManager * );
};

class SceneState_EntireModel : public State< DXManager >
{
private:
	SceneState_EntireModel(){}
	SceneState_EntireModel( const SceneState_EntireModel& );
	SceneState_EntireModel& operator=( const SceneState_EntireModel& );

public:
	static SceneState_EntireModel * Instance();

	virtual void Enter( DXManager * );
	virtual void Execute( DXManager * );
	virtual void Exit( DXManager * );
};

class SceneState_MergingModel : public State< DXManager >
{
private:
	SceneState_MergingModel(){}
	SceneState_MergingModel( const SceneState_MergingModel& );
	SceneState_MergingModel& operator=( const SceneState_MergingModel& );

public:
	static SceneState_MergingModel * Instance();

	virtual void Enter( DXManager * );
	virtual void Execute( DXManager * );
	virtual void Exit( DXManager * );
};

#endif