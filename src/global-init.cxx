// Module:  Log4CPLUS
// File:    global-init.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/config.hxx>
#include <log4cplus/logger.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/internal/internal.h>


// Forward Declarations
namespace log4cplus
{

#ifdef UNICODE
LOG4CPLUS_EXPORT tostream & tcout = std::wcout;
LOG4CPLUS_EXPORT tostream & tcerr = std::wcerr;

#else
LOG4CPLUS_EXPORT tostream & tcout = std::cout;
LOG4CPLUS_EXPORT tostream & tcerr = std::cerr;

#endif // UNICODE


namespace internal
{


gft_scratch_pad::gft_scratch_pad ()
    : uc_q_str_valid (false)
    , q_str_valid (false)
{ }


gft_scratch_pad::~gft_scratch_pad ()
{ }


appender_sratch_pad::appender_sratch_pad ()
{ }


appender_sratch_pad::~appender_sratch_pad ()
{ }


per_thread_data::per_thread_data ()
{ }


per_thread_data::~per_thread_data ()
{ }


LOG4CPLUS_THREAD_LOCAL_TYPE tls_storage_key;


#if ! defined (LOG4CPLUS_SINGLE_THREADED)

# if defined (LOG4CPLUS_THREAD_LOCAL_VAR)

LOG4CPLUS_THREAD_LOCAL_VAR per_thread_data * ptd = 0;


per_thread_data *
alloc_ptd ()
{
    per_thread_data * tmp = new per_thread_data;
    set_ptd (tmp);
    // This is a special hack. We set the keys' value to non-NULL to
    // get the ptd_cleanup_func to execute when this thread ends. The
    // cast is safe; the associated value will never be used if read
    // again using the key.
    LOG4CPLUS_SET_THREAD_LOCAL_VALUE (tls_storage_key,
        reinterpret_cast<void *>(1));

    return tmp;
}

#  else


per_thread_data *
alloc_ptd ()
{
    per_thread_data * tmp = new per_thread_data;
    set_ptd (tmp);
    return tmp;
}

#  endif

#else

std::auto_ptr<per_thread_data> ptd;


per_thread_data *
alloc_ptd ()
{
    per_thread_data * tmp = new per_thread_data;
    set_ptd (tmp);
    return tmp;
}

#endif

} // namespace internal


void initializeFactoryRegistry();


#if ! defined (LOG4CPLUS_SINGLE_THREADED)

//! Thread local storage clean up function for POSIX threads.
static 
void 
ptd_cleanup_func (void * arg)
{
    // Either it is a dummy value or it should be the per thread data
    // pointer we get from internal::get_ptd().
    assert (arg == reinterpret_cast<void *>(1)
        || arg == internal::get_ptd ());
    (void)arg;

    threadCleanup ();

    // Setting the value through the key here is necessary in case we
    // are using TLS using __thread or __declspec(thread) or similar
    // constructs with POSIX threads. Otherwise POSIX calls this cleanup
    // routine more than once if the value stays non-NULL after it returns.
    LOG4CPLUS_SET_THREAD_LOCAL_VALUE (internal::tls_storage_key, 0);
}

#endif


void initializeLog4cplus()
{
    static bool initialized = false;
    if (initialized)
        return;

#if ! defined (LOG4CPLUS_SINGLE_THREADED)

    internal::tls_storage_key = LOG4CPLUS_THREAD_LOCAL_INIT (ptd_cleanup_func);

#endif

    helpers::LogLog::getLogLog();
    getLogLevelManager ();
    getNDC();
    Logger::getRoot();
    initializeFactoryRegistry();

    initialized = true;
}


} // namespace log4cplus


#if defined (_WIN32) && (defined (LOG4CPLUS_BUILD_DLL) || defined (log4cplus_EXPORTS))

BOOL WINAPI DllMain(LOG4CPLUS_DLLMAIN_HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved )  // reserved
{
    using namespace log4cplus;

    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
    case DLL_PROCESS_ATTACH:
    {
        log4cplus::initializeLog4cplus();
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific initialization for the main thread.
        internal::per_thread_data * ptd = new internal::per_thread_data;
        internal::set_ptd (ptd);
#endif
        break;
    }

    case DLL_THREAD_ATTACH:
    {
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific initialization.
        internal::per_thread_data * ptd = new internal::per_thread_data;
        internal::set_ptd (ptd);
#endif
        break;
    }

    case DLL_THREAD_DETACH:
    {
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific cleanup.
        threadCleanup ();
#endif
        break;
    }

    case DLL_PROCESS_DETACH:
    {
        // Perform any necessary cleanup.
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific cleanup.
        threadCleanup ();
#  if ! defined (LOG4CPLUS_THREAD_LOCAL_VAR)
        LOG4CPLUS_THREAD_LOCAL_CLEANUP (internal::tls_storage_key);
#  endif
#endif
        break;
    }

    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
 
#else

namespace {

    struct _static_log4cplus_initializer
    {
        _static_log4cplus_initializer ()
        {
            log4cplus::initializeLog4cplus();
        }

        ~_static_log4cplus_initializer ()
        {
            // Last thread cleanup.
            log4cplus::threadCleanup ();
        }
    } static initializer;
}


#endif


namespace log4cplus
{


void
threadCleanup ()
{
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    // Do thread-specific cleanup.
    internal::per_thread_data * ptd = internal::get_ptd (false);
    delete ptd;
    internal::set_ptd (0);
#endif
}


} // namespace log4cplus
