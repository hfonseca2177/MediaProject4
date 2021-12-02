#pragma once
#include <FMOD/fmod.hpp>
#include <string>
#include <vector>

enum eParamType
{
	INT, FLOAT
};


struct sParam
{
	int id;
	std::string name;
	int intValue;
	float floatValue;
	enum eParamType type;

};

struct sDSP
{
	int id;
	std::string name;
	FMOD::DSP* dsp;
	std::vector<sParam> params;
	bool enabled;
	bool bypass;
};




//Factory to instantiate a DSP
class cDSPFactory
{
private:
	std::vector<FMOD::DSP*> dpsList;
	void SetValue(FMOD::DSP* dsp, sParam& param);
public:
	cDSPFactory();
	~cDSPFactory();
	void LoadDSP(FMOD::System* _system, sDSP& dsp);
};