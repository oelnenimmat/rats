#include "JobQueue.hpp"

void JobQueue::execute() 
{	
	MINIMA_ASSERT(executing == false);

	executing = true;
	// Todo(Leo): too much nesting, is ugly
	work_thread = std::thread([this]()
	{
		for (QueuedJob const & job : jobs)
		{	
			if (job.parallel)
			{
				#pragma omp parallel for
				for (int i = 0; i < job.count; i++)
				{
					job.execute(job.job, i);
				}
			}
			else
			{
				for (int i = 0; i < job.count; i++)
				{
					job.execute(job.job, i);
				}
			}

		}

		executing = false;
	});
}

void JobQueue::wait()
{
	work_thread.join();
	executing = false;
}

void JobQueue::reset()
{
	MINIMA_ASSERT(executing == false);
	
	jobs.reset();
	personal_allocator.reset();
}