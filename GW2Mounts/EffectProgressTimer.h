#pragma once
#include "Utility.h"

class EffectProgressTimer
{
public:
	EffectProgressTimer();
	~EffectProgressTimer();

	void SetEffectDuration(uint milliseconds);
	void SetEffectSteps(uint steps);
	void Start();
	void Cancel();
	float GetProgress();

private:
	mstime StartTime;
	uint EffectDuration;
	uint EffectSteps;
	bool Enabled = false;

};

