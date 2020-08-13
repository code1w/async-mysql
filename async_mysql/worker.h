#pragma once
#include <functional>
#include "dll_export.h"

namespace gamesh
{
	class WorkerImpl;
	class GAMESH_MYSQL_IOC_DLL_CLASS_DECL Worker
	{
	private:
		WorkerImpl* impl_;
	public:
		Worker(void* loop);
		Worker();
		virtual ~Worker();
		void execute(const std::function<void()>& function);
	};
}