///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


#include <EASTL/internal/config.h>
#include <EASTL/internal/thread_support.h>
#include <EASTL/type_traits.h>
#include <EASTL/memory.h>

#if defined(EA_PLATFORM_MICROSOFT)
	#pragma warning(push, 0)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <ntddk.h>
	#pragma warning(pop)    
#endif


namespace eastl
{
	namespace Internal
	{
		#if EASTL_CPP11_MUTEX_ENABLED
			// We use the C++11 Standard Library mutex as-is.
		#else
			/////////////////////////////////////////////////////////////////
			// mutex
			/////////////////////////////////////////////////////////////////

			mutex::mutex()
			{
				mMutex = new KMUTEX;
				KeInitializeMutex(mMutex, 0);
			}

			mutex::~mutex()
			{
				delete mMutex;
			}

			void mutex::lock()
			{
				NTSTATUS st;
				KIRQL kIrql = KeGetCurrentIrql();

				if (kIrql < DISPATCH_LEVEL)
				{
					st = KeWaitForSingleObject(mMutex, Executive, KernelMode, FALSE, NULL);
					if (!NT_SUCCESS(st))
						ExRaiseStatus(st);
				}
				else if (kIrql == DISPATCH_LEVEL)
				{
					LARGE_INTEGER Timeout;

					Timeout.QuadPart = 0;
					do 
					{
						st = KeWaitForSingleObject(mMutex, Executive, KernelMode, FALSE, &Timeout);
						if (st == STATUS_SUCCESS)
						{
							break;
						}
						else if (st == STATUS_TIMEOUT)
						{
							// continue;
						}
						else
						{
							ExRaiseStatus(st);
						}
					} while (true);
				}
				else
				{
					KeBugCheck(IRQL_NOT_LESS_OR_EQUAL);
				}
			}

			void mutex::unlock()
			{
				KeReleaseMutex(mMutex, FALSE);
			}
		#endif


		/////////////////////////////////////////////////////////////////
		// shared_ptr_auto_mutex
		/////////////////////////////////////////////////////////////////

		// We could solve this by having single global mutex for all shared_ptrs, a set of mutexes for shared_ptrs, 
		// a single mutex for every shared_ptr, or have a template parameter that enables mutexes for just some shared_ptrs.
		eastl::late_constructed<mutex, true> gSharedPtrMutex;

		shared_ptr_auto_mutex::shared_ptr_auto_mutex(const void* /*pSharedPtr*/)
			: auto_mutex(*gSharedPtrMutex.get())
		{
		}


	} // namespace Internal

} // namespace eastl















