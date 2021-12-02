#pragma once
#include<vector>
#include <RapidXml/rapidxml.hpp>
#include <RapidXml/rapidxml_utils.hpp>
#include<fstream>
#include<sstream>

#include "cSoundResource.h"
#include "cDSPFactory.h"


#include <map>

//Sound FX ground with assigned DSPs
struct sFxGroup 
{
	int index;
	std::vector<sDSP> _dsps;
};


class cSoundManager {
public:

	//system pointer
	FMOD::System* _system;

	//vector to hold sound data
	std::vector<cSoundResource*> stream_Sounds;

	//vector to store all channel groups
	std::vector<FMOD::ChannelGroup*> groups;

	std::vector<float> pan;

	//pointer to active channel
	FMOD::Channel* _activeStreamChannel;
	

	//pointer to the active channel group
	FMOD::ChannelGroup* _activeChannelGroup;
	
	//pointert to the master channel group
	FMOD::ChannelGroup* _masterChannelGroup;
	
	//DSPs FX groups
	std::vector<sFxGroup> dspGroups;
	
	//DSPs factory
	cDSPFactory dspFactory;
	
	//indexs for active channel and cycling
	int stream_index = 0;
	int group_index = 0;

	//constructor
	cSoundManager(FMOD::System* System);

	//do the cleanup here
	~cSoundManager();


	//functions
	//reads basic structure from XML file
	bool ReadSoundsFromXml();

	// create channel groups and call creat sound to populate them
	bool loadChannelGroups_Stream(rapidxml::xml_node<>* node);
	
	bool loadGroupDSPs(rapidxml::xml_node<>* channelNode);
	sDSP loadDSP(rapidxml::xml_node<>* dspNode);
	//helper function to load sounds
	FMOD::Sound* LoadRemoteSound(std::string url);

	//play Sound functions
	void PlayStreamSound(int index);

	//cycle channels
	void cycleStream();
	void cycleChannelGroup();
	bool hasCustomizableParam();
	sParam& getCustomizableParam();
	void AddToCustomizableParam();
	void SubToCustomizableParam();

	//modify attributes of stream sound
	void setpan(float p);
	void setpitch(float p);
	void setvol(float v);
	void setvolRamp();
	void setFrequency(float f);
	void addpan(float p);
	void subpan(float p);
	float getpan();

	//change bypass for dsp
	void changebypass(int x);
	void changebypassall(bool &stat);
	void pause();
	void changeVol(float amount);
	
};

//Check and print FMod error result
bool HandleFmodError(int result, const char* taskDescription);