#pragma once
#include <FMOD/fmod.hpp>
#include <string>
#include <sstream>

//Sound Resource State
enum class eSoundResourceState
{
	UNSET, CREATED, LOADING, PLAYING, PAUSED, STARVING, BUFFERING
};

//Represents a sound resource
class cSoundResource
{
private:
	int index;
	std::string id;
	std::string name;
	FMOD::Sound* sound;
	FMOD::ChannelGroup* group;
	std::string url;
	eSoundResourceState state;
	std::stringstream tags;
	unsigned int bufferingPercentage;

public:
	FMOD::Channel* channel;
	cSoundResource(int& _index, std::string& name);
	virtual ~cSoundResource();
	void SetId(std::string _id);
	void SetSound(FMOD::Sound* _sound);
	void SetChannel(FMOD::Channel* _channel);
	void SetGroup(FMOD::ChannelGroup* _group);
	void SetUrl(std::string _url);
	void SetState(eSoundResourceState _state);
	std::string GetId();
	std::string GetUrl();
	FMOD::Sound* GetSound();
	FMOD::Channel* GetChannel();
	FMOD::ChannelGroup* GetGroup();
	std::string GetTags();
	void CheckTags();
	void Update();
	char* GetCurrentState();
	bool IsStarving();
	int GetIndex();
	bool IsActive();
	char* GetCurrentBuffering();
};