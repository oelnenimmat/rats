#pragma once

#include "memory.hpp"

#include <thread>
#include <atomic>

struct JobQueue
{
	JobQueue(Allocator & allocator, int max_jobs) :
		jobs(List<QueuedJob>(max_jobs, allocator)),
		allocator(&allocator),
		personal_allocator(ArenaAllocator(1024, allocator.typeless_allocate(1024, 1)))
	{}

	~JobQueue()
	{
		allocator->typeless_deallocate(personal_allocator.return_memory_back_to_where_it_was_received());
	}

	template<typename T>
	void enqueue(T job, int count);

	template<typename T>
	void enqueue_parallel(T job, int count);

	void execute();
	void wait();
	void reset();

private:
	struct QueuedJob
	{
		void (*execute)(void*, int);
		void * 	job;
		int 	count;
		bool 	parallel;
	};

	List<QueuedJob> jobs;
	Allocator * allocator;
	ArenaAllocator personal_allocator;

	std::thread work_thread;
	std::atomic_bool executing;
};

template<typename T>
void JobQueue::enqueue(T job, int count)
{
	QueuedJob queued_job;

	// Note(Leo): I tried explicit execute function per type and a proper template
	// function instead of lambda, but they performed the same. Maybe its optimized away??
	queued_job.execute = [](void * job, int i)
	{
		reinterpret_cast<T*>(job)->execute(i);
	};
	
	T * job_ptr 	= personal_allocator.allocate<T>(1);
	*job_ptr 		= std::move(job);
	queued_job.job 	= job_ptr;

	queued_job.count 	= count;
	queued_job.parallel = false;

	jobs.add(queued_job);
}

template<typename T>
void JobQueue::enqueue_parallel(T job, int count)
{
	QueuedJob queued_job;

	// Note(Leo): I tried explicit execute function per type and a proper template
	// function instead of lambda, but they performed the same. Maybe its optimized away??
	queued_job.execute = [](void * job, int i)
	{
		reinterpret_cast<T*>(job)->execute(i);
	};
	
	T * job_ptr 	= personal_allocator.allocate<T>(1);
	*job_ptr 		= std::move(job);
	queued_job.job 	= job_ptr;

	queued_job.count 	= count;
	queued_job.parallel = true;

	jobs.add(queued_job);
}
