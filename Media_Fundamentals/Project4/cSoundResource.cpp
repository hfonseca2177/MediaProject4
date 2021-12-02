#include "cSoundResource.h"



void cSoundResource::SetId(std::string _id)
{
	this->id = _id;
}

std::string cSoundResource::GetId()
{
	return this->id;
}

std::string cSoundResource::GetUrl()
{
	return this->url;
}

void cSoundResource::SetState(eSoundResourceState _state)
{
	this->state = _state;
}

cSoundResource::cSoundResource(int& _index, std::string& _name)
{
	this->index = _index;
	this->name = _name;
	this->state = eSoundResourceState::UNSET;
	this->channel = nullptr;
	this->sound = nullptr;
	this->group = nullptr;
	this->url = "";
	this->tags.str("");
}
cSoundResource::~cSoundResource()
{
	//Dont delete pointer here because they need to be stopped/released before
}
void cSoundResource::SetSound(FMOD::Sound* _sound)
{
	this->sound = _sound;
}

void cSoundResource::SetChannel(FMOD::Channel* _channel)
{
	this->channel = _channel;
}

void cSoundResource::SetGroup(FMOD::ChannelGroup* _group)
{
	this->group = _group;
}

void cSoundResource::SetUrl(std::string _url)
{
	this->url = _url;
}

FMOD::Sound* cSoundResource::GetSound()
{
	return this->sound;
}

FMOD::Channel* cSoundResource::GetChannel()
{
	return this->channel;
}

FMOD::ChannelGroup* cSoundResource::GetGroup()
{
	return this->group;
}

std::string cSoundResource::GetTags()
{
	return tags.str();
}

void cSoundResource::CheckTags()
{
	tags.str("Tags: ");
	int numTags, numTagsUpdated;
	FMOD_RESULT result = sound->getNumTags(&numTags, &numTagsUpdated);
	if (result != FMOD_OK)
	{
		fprintf(stderr, "unable to Get Tags\n");
		tags << "none";
		return;
	}
	
	for (size_t i = 0; i < numTags; i++)
	{
		FMOD_TAG tag;
		sound->getTag(0, i, &tag);
		//Verify if it is possible to set frequency 
		if (tag.type == FMOD_TAGTYPE_FMOD && (!strcmp(tag.name, "Sample Rate Change") && channel))
		{
			float frequency = *((float*) tag.data);
			result = channel->setFrequency(frequency);
			if (result != FMOD_OK)
			{
				fprintf(stderr, "unable to Set Frequency\n");
			}
		}
		//Retrieve tag descriptors
		else if (tag.type == FMOD_TAGDATATYPE_STRING || tag.type == FMOD_TAGDATATYPE_STRING_UTF8)
		{
			char buffer[255];
			sprintf_s(buffer, "%s = '%s'", tag.name, (char*)tag.data);
			tags << buffer;
		}
	}
}

void cSoundResource::Update()
{
	FMOD_OPENSTATE openState;
	
	bool starving, playing, paused;
	FMOD_RESULT result = sound->getOpenState(&openState, &bufferingPercentage, &starving, 0);
	if (result != FMOD_OK)
	{
		fprintf(stderr, "unable get state\n");
		return;
	}
	channel->isPlaying(&playing);
	channel->getPaused(&paused);
	if (openState == FMOD_OPENSTATE_CONNECTING) SetState(eSoundResourceState::LOADING);
	if (playing) SetState(eSoundResourceState::PLAYING);
	if (starving) SetState(eSoundResourceState::STARVING);
	if (openState == FMOD_OPENSTATE_BUFFERING) SetState(eSoundResourceState::BUFFERING);
	if (paused) SetState(eSoundResourceState::PAUSED);
}

char* cSoundResource::GetCurrentState()
{
	char buffer[255];
	switch (state)
	{
	case eSoundResourceState::CREATED:
	{
		sprintf_s(buffer,"Created in FMOD");
		break;
	}
	case eSoundResourceState::LOADING:
	{
		sprintf_s(buffer, "Loading on FMOD buffer");
		break;
	}
	case eSoundResourceState::PAUSED:
	{
		sprintf_s(buffer, "Sound paused");
		break;
	}
	case eSoundResourceState::PLAYING:
	{
		sprintf_s(buffer, "Sound Playing right now");
		break;
	}
	case eSoundResourceState::BUFFERING:
	{
		sprintf_s(buffer, "Buffering...('%d')", bufferingPercentage);
		break;
	}
	case eSoundResourceState::STARVING:
	{
		sprintf_s(buffer, "Starving... Gonna be muted");
		break;
	}
	default:
	{
		sprintf_s(buffer, "Not Used");
		break;
	}
		
	}

	return buffer;
}

bool cSoundResource::IsStarving()
{
	return state == eSoundResourceState::STARVING;
}

int cSoundResource::GetIndex()
{
	return this->index;
}

bool cSoundResource::IsActive()
{
	return state == eSoundResourceState::PLAYING || state == eSoundResourceState::BUFFERING;
}

char* cSoundResource::GetCurrentBuffering()
{
	char buffer[255];
	sprintf_s(buffer, "Buffering...('%d')", bufferingPercentage);
	return buffer;
}