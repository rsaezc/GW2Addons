#pragma once
#include "Utility.h"

class EffectProgressTimer
{
public:
	EffectProgressTimer();
	~EffectProgressTimer();

	void SetEffectDuration(uint milliseconds);
	void Start();
	void Cancel();
	float GetProgress();

private:
	mstime StartTime;
	uint EffectDuration;
	bool Enabled = false;

};

