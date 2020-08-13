#include "worker.h"
#include "worker_impl.h"
#include "thread_worker_impl.h"

namespace gamesh
{
	Worker::Worker(void* loop)
	{
		impl_ = nullptr;
	}

	Worker::Worker()
	{
		impl_ = new ThreadWorkerImpl();
	}

	Worker::~Worker()
	{
		delete impl_;
	}

	void Worker::execute(const std::function<void()>& function)
	{
		impl_->execute(function);
	}

}