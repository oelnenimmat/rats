#pragma once

#include "JobQueue.hpp"

template <typename TJob>
void run_job(TJob & job, int count)
{
	for(int i = 0; i < count; i++)
	{
		job.execute(i);
	}
}