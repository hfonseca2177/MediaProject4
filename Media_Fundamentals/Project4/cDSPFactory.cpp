#include "cDSPFactory.h"


cDSPFactory::cDSPFactory()
{
}

cDSPFactory::~cDSPFactory()
{	
}

void cDSPFactory::LoadDSP(FMOD::System* _system, sDSP& dsp)
{
	dsp.dsp = nullptr;
	FMOD_RESULT _result = _system->createDSPByType((FMOD_DSP_TYPE)dsp.id, &dsp.dsp);
	dsp.dsp->setActive(dsp.enabled);
	dsp.dsp->setBypass(dsp.bypass);
	for (size_t i = 0; i < dsp.params.size(); i++)
	{
		SetValue(dsp.dsp, dsp.params.at(i));
	}
	
		
	if (FMOD_OK != _result) {
		fprintf(stderr, "Unable to create dsp\n");
	}
	else
	{
		this->dpsList.push_back(dsp.dsp);
	}

}

void cDSPFactory::SetValue(FMOD::DSP* dsp, sParam& param)
{
	if (param.type == INT)
	{
		dsp->setParameterInt(param.id, param.intValue);
	}
	else
	{
		dsp->setParameterFloat(FMOD_DSP_FLANGE_MIX, param.floatValue);
	}
	
}
