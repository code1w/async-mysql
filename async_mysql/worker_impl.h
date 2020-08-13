#pragma once
#include <functional>
namespace gamesh
{
	class WorkerImpl
	{
	public:
		virtual ~WorkerImpl(){}
		virtual void execute(const std::function<void()>& function) = 0;
	};
}
