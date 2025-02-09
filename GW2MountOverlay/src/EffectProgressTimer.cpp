#include "EffectProgressTimer.h"

EffectProgressTimer::EffectProgressTimer()
{
}


EffectProgressTimer::~EffectProgressTimer()
{
}

void EffectProgressTimer::SetEffectDuration(uint milliseconds)
{
	EffectDuration = milliseconds;
}

void EffectProgressTimer::Start()
{
	StartTime = timeInMS();
	Enabled = true;
}

void EffectProgressTimer::Cancel()
{
	Enabled = false;
}

float EffectProgressTimer::GetProgress()
{
	if (Enabled)
	{
		float progress = (timeInMS() - StartTime) / (float)EffectDuration;
		if (progress < 1.f)
		{
			return progress;
		}
		Enabled = false;
	}
	return 1.f;
}
