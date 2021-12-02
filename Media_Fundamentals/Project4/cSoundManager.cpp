#include"cSoundManager.h"
#include<iostream>

//reads basic structure from the XMl file
//then call the appropriate function
bool cSoundManager::ReadSoundsFromXml() {
	
	//reads from xml file and call functions to load the sounds depending on its value
	

	std::string path = SOUND_DIR;
	path.append("SoundList.xml"); // name of the xml file that contains sounds


	// load the file
	rapidxml::file<>* xmlFile = new rapidxml::file<>(path.c_str());
	rapidxml::xml_document<>* doc = new rapidxml::xml_document<>;    // character type defaults to char
	doc->parse<0>(xmlFile->data());// 0 means default parse flags

	if (doc == NULL) {
		fprintf(stderr, "document Contains Nothing");
		return false;
	}

	rapidxml::xml_node<>* root = doc->first_node("sounds");
	rapidxml::xml_node<>* stream_node = root->first_node("stream");
		

	//loop through stream node children and creat a sound obj
	for (rapidxml::xml_node<>* child = stream_node->first_node(); child; child = child->next_sibling())
	{
		//temp container for sound
		loadChannelGroups_Stream(child);
		
	}

	//setup the default active channel group to 0
	_activeChannelGroup = groups[group_index];


	rapidxml::xml_node<>* channelsNode = root->first_node("effects");
	//loop through sample node children and creat a sound obj
	for (rapidxml::xml_node<>* child = channelsNode->first_node(); child; child = child->next_sibling())
	{
		// initilize stream sounds	
		loadGroupDSPs(child);
	}

	// cleanup
	delete doc;
	delete xmlFile;

	return true;
}


//creat a stream sound
FMOD::Sound* cSoundManager::LoadRemoteSound(std::string url) {

	//for error handling
	FMOD_RESULT _result = FMOD_OK;

	FMOD::Sound* tempsound;

	// create stream sound
	if(HandleFmodError(_system->createSound(url.c_str(), FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &tempsound),
		"Create Stream Sound")) return NULL;

	return tempsound;
}


//creat a channel group and calls LoadSound
bool cSoundManager::loadChannelGroups_Stream(rapidxml::xml_node<>* node) {
	
	for (rapidxml::xml_node<>* child = node->first_node(); child; child = child->next_sibling())
	{
		//Get Index and name need to construct a SoundResource
		int index = stream_Sounds.size();
		std::string soundName = child->value();
		cSoundResource* newSound = new cSoundResource(index, soundName);
		//Retrieve resource ID to create a group for it to be able to apply dsps aftwards for each stream
		newSound->SetId(child->first_attribute("id")->value());
		//create channel group with the node name
		FMOD::ChannelGroup* group = NULL;
		if (HandleFmodError(_system->createChannelGroup(newSound->GetId().c_str(), &group), "create stream Channel group"))
			return false;
		group->setVolume(0.1);
		newSound->SetGroup(group);
		//Resource address
		newSound->SetUrl(child->value());
		newSound->SetSound(LoadRemoteSound(newSound->GetUrl()));
		newSound->SetState(eSoundResourceState::CREATED);
		groups.push_back(group);
		stream_Sounds.push_back(newSound);
	}
	
	float pan1 = 0.0f;
	pan.push_back(pan1);
	return true;
}

sDSP cSoundManager::loadDSP(rapidxml::xml_node<>* dspNode)
{
	const std::string trueCheck = "true";
	sDSP dsp;
	dsp.id = atoi(dspNode->first_attribute("id")->value());
	dsp.name = dspNode->first_attribute("name")->value();
	dsp.enabled = trueCheck._Equal(dspNode->first_attribute("enabled")->value());
	dsp.bypass = trueCheck._Equal(dspNode->first_attribute("bypass")->value());

	for (rapidxml::xml_node<>* paramNode = dspNode->first_node(); paramNode; paramNode = paramNode->next_sibling())
	{
		sParam param;
		if (paramNode->first_attribute("type")->value() == "int")
		{
			param.type = eParamType::INT;
			param.intValue = atoi(paramNode->value());
		}
		else
		{
			param.type = eParamType::FLOAT;
			param.floatValue = std::stof(paramNode->value());
		}
		param.id = atoi(paramNode->first_attribute("id")->value());
		param.name = paramNode->first_attribute("name")->value();
		dsp.params.push_back(param);
	}
	dspFactory.LoadDSP(_system, dsp);
	return dsp;
}

bool cSoundManager::loadGroupDSPs(rapidxml::xml_node<>* channelNode)
{
	sFxGroup fxGroup;
	fxGroup.index = atoi(channelNode->first_attribute("id")->value());
	for (rapidxml::xml_node<>* dspNode = channelNode->first_node(); dspNode; dspNode = dspNode->next_sibling())
	{
		sDSP dsp = loadDSP(dspNode);
		fxGroup._dsps.push_back(dsp);
	}
	dspGroups.push_back(fxGroup);
	return true;
}

//play sound by index
void cSoundManager::PlayStreamSound(int index) {
	cSoundResource* streamSound = stream_Sounds[index];

	//Try to start playing the stream sound (NOT VALIDATING RIGHT AWAY)
	_system->playSound(streamSound->GetSound(), streamSound->GetGroup(), false, &streamSound->channel);
	
	//update current state
	streamSound->SetState(eSoundResourceState::LOADING);

	//indicating that the new sound channel is the active one
	_activeStreamChannel = streamSound->channel;
	//matching the global index to the newest sound played
	stream_index = index;
}

void cSoundManager::cycleStream() {
	//pause the current active sound
	_activeStreamChannel->setPaused(true);
	//get the next sound on the list
	_activeStreamChannel = stream_Sounds[stream_index++]->GetChannel();
	//check that index doesnt go out of bounds
	if (stream_index >= stream_Sounds.size())stream_index = 0;
	//if we havent set the channel for this sound before
	// play the sound
	if (stream_Sounds[stream_index]->GetChannel() == NULL) {
		PlayStreamSound(stream_index);
	}
	// if its set just unpause it
	_activeStreamChannel->setPaused(false);

	
}
void cSoundManager::cycleChannelGroup() {
	//just cycle the channel group index
	group_index++;

	if (group_index > groups.size())group_index = 0;

	if (group_index == groups.size()) {

		_activeChannelGroup = _masterChannelGroup;
	}
	else {
		_activeChannelGroup = groups[group_index];
	}
}


//helper functions to change pan and stuff

void cSoundManager::addpan(float p) {

	pan.at(group_index)+=p;
	setpan(pan.at(group_index));


}
void cSoundManager::subpan(float p) {
	
	pan.at(group_index) -= p;
	setpan(pan.at(group_index));
	

}
float cSoundManager::getpan() {
	return pan.at(group_index);
}
void cSoundManager::setpan(float p) {
	_activeChannelGroup->setPan(pan.at(group_index));
}
void cSoundManager::setpitch(float p) {
	_activeChannelGroup->setPitch(p);
}
void cSoundManager::setvol(float v) {
	_activeChannelGroup->setVolume(v);
}
void cSoundManager::setvolRamp() {
	bool* freq = NULL;
	_activeChannelGroup->getVolumeRamp(freq);
	_activeChannelGroup->setVolumeRamp(!freq);
}
void cSoundManager::setFrequency(float f) {
	_activeStreamChannel->setFrequency(f);
}

void cSoundManager::pause() {
	bool paused;
	_activeChannelGroup->getPaused(&paused);
	_activeChannelGroup->setPaused(!paused);
}


void cSoundManager::changebypass(int x) {
	if (_activeChannelGroup == _masterChannelGroup)return;
	FMOD::DSP* dsp;
	_activeChannelGroup->getDSP(x, &dsp);

	bool current_bypass;
	dsp->getBypass(&current_bypass);
	dsp->setBypass(!current_bypass);

}
void cSoundManager::changebypassall(bool &stat) {

	for (int x = 0; x < dspGroups.size(); x++) {

		for (int i = 0; i < dspGroups[x]._dsps.size(); i++) {
		
			dspGroups[x]._dsps[i].dsp->setBypass(stat);
		}
	}
	
	stat = !stat;
}

void cSoundManager::changeVol(float amount) {
	float vol;
	_activeChannelGroup->getVolume(&vol);
	vol += amount;
	if (vol < 0)vol = 0.0f;
	_activeChannelGroup->setVolume(vol);

}

//constructor 
cSoundManager::cSoundManager( FMOD::System* System) {
	_system = System;
	ReadSoundsFromXml();

	HandleFmodError(_system->getMasterChannelGroup(&_masterChannelGroup), "get master channel group");

	//loop throug the dsp and assign them to the groups 
	//also bypass all the dsp so we can change them using keys later
	for (int x = 0; x < dspGroups.size(); x++) {
		
		for (int i = 0; i < dspGroups[x]._dsps.size(); i++) {
		
			groups[x]->addDSP(i, dspGroups[x]._dsps[i].dsp);
			dspGroups[x]._dsps[i].dsp->setActive(true);
			dspGroups[x]._dsps[i].dsp->setBypass(true);
			std::stringstream ss;
			char name[255] = "";
			unsigned int version;
			int channels, configw, configh;
			dspGroups[x]._dsps[i].dsp->getInfo(name, &version, &channels, &configw, &configh);
		}
	
	}
	
	float mainpan = 0.0f;
	pan.push_back(mainpan);

}

cSoundManager::~cSoundManager() 
{
}


bool cSoundManager::hasCustomizableParam()
{
	return group_index < dspGroups.size() && dspGroups[group_index]._dsps.size() > 0
		&& dspGroups[group_index]._dsps[0].params.size() > 0;
}

sParam& cSoundManager::getCustomizableParam()
{
	return this->dspGroups[group_index]._dsps[0].params[0];
}

void cSoundManager::AddToCustomizableParam()
{
	FMOD::DSP* dsp;
	_activeChannelGroup->getDSP(0, &dsp);

	sParam& param = getCustomizableParam();
	if (param.type == eParamType::INT)
	{
		param.intValue = param.intValue + 1;
		dsp->setParameterInt(0, param.intValue);
	}
	else
	{
		param.floatValue = param.floatValue + 0.5f;
		dsp->setParameterFloat(0, param.floatValue);
	}

}
void cSoundManager::SubToCustomizableParam()
{
	FMOD::DSP* dsp;
	_activeChannelGroup->getDSP(0, &dsp);

	sParam& param = getCustomizableParam();
	if (param.type == eParamType::INT)
	{
		param.intValue -= 1;
		dsp->setParameterInt(0, param.intValue);
	}
	else
	{
		param.floatValue -= 0.5f;
		dsp->setParameterFloat(0, param.floatValue);
	}
}

bool HandleFmodError(int result, const char* taskDescription)
{
	bool hasError = result != FMOD_OK;
	if (hasError) {
		fprintf(stderr, "Fmod could not perform action[%d]: %s\n", result, taskDescription);
	}
	return hasError;
}