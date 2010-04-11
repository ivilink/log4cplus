// Module:  Log4CPLUS
// File:    loggingevent.cxx
// Created: 6/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/loggingevent.h>
#include <algorithm>


namespace log4cplus {  namespace spi {


static const int LOG4CPLUS_DEFAULT_TYPE = 1;


///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent ctors and dtor
///////////////////////////////////////////////////////////////////////////////

InternalLoggingEvent::InternalLoggingEvent(const log4cplus::tstring& logger,
    LogLevel loglevel, const log4cplus::tstring& message_, const char* filename,
    int line_)
    : message(message_)
    , loggerName(logger)
    , ll(loglevel)
    , ndc()
    , thread()
    , timestamp(log4cplus::helpers::Time::gettimeofday())
    , file(filename
        ? LOG4CPLUS_C_STR_TO_TSTRING(filename) 
        : log4cplus::tstring())
    , line(line_)
    , threadCached(false)
    , ndcCached(false)
{
}


InternalLoggingEvent::InternalLoggingEvent(const log4cplus::tstring& logger,
    LogLevel loglevel, const log4cplus::tstring& ndc_,
    const log4cplus::tstring& message_, const log4cplus::tstring& thread_,
    log4cplus::helpers::Time time, const log4cplus::tstring& file_, int line_)
    : message(message_)
    , loggerName(logger)
    , ll(loglevel)
    , ndc(ndc_)
    , thread(thread_)
    , timestamp(time)
    , file(file_)
    , line(line_)
    , threadCached(true)
    , ndcCached(true)
{
}


InternalLoggingEvent::InternalLoggingEvent ()
    : ll (NOT_SET_LOG_LEVEL)
    , threadCached(false)
    , ndcCached(false)
{ }


InternalLoggingEvent::InternalLoggingEvent(
    const log4cplus::spi::InternalLoggingEvent& rhs)
    : message(rhs.getMessage())
    , loggerName(rhs.getLoggerName())
    , ll(rhs.getLogLevel())
    , ndc(rhs.getNDC())
    , thread(rhs.getThread())
    , timestamp(rhs.getTimestamp())
    , file(rhs.getFile())
    , line(rhs.getLine())
    , threadCached(true)
    , ndcCached(true)
{
}


InternalLoggingEvent::~InternalLoggingEvent()
{
}



///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent static methods
///////////////////////////////////////////////////////////////////////////////

unsigned int
InternalLoggingEvent::getDefaultType()
{
    return LOG4CPLUS_DEFAULT_TYPE;
}



///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent implementation
///////////////////////////////////////////////////////////////////////////////

void
InternalLoggingEvent::setLoggingEvent (const log4cplus::tstring & logger,
    LogLevel loglevel, const log4cplus::tstring & msg, const char * filename,
    int fline)
{
    InternalLoggingEvent (logger, loglevel, msg, filename, fline).swap (*this);
}


const log4cplus::tstring& 
InternalLoggingEvent::getMessage() const
{
    return message;
}


unsigned int
InternalLoggingEvent::getType() const
{
    return LOG4CPLUS_DEFAULT_TYPE;
}



std::auto_ptr<InternalLoggingEvent>
InternalLoggingEvent::clone() const
{
    std::auto_ptr<InternalLoggingEvent> tmp(new InternalLoggingEvent(*this));
    return tmp;
}



InternalLoggingEvent &
InternalLoggingEvent::operator = (const InternalLoggingEvent& rhs)
{
    InternalLoggingEvent (rhs).swap (*this);
    return *this;
}


void
InternalLoggingEvent::swap (InternalLoggingEvent & other)
{
    using std::swap;

    swap (message, other.message);
    swap (loggerName, other.loggerName);
    swap (ll, other.ll);
    swap (ndc, other.ndc);
    swap (thread, other.thread);
    swap (timestamp, other.timestamp);
    swap (file, other.file);
    swap (line, other.line);
    swap (threadCached, other.threadCached);
    assert (threadCached);
    swap (ndcCached, other.ndcCached);
    assert (ndcCached);
}


} } // namespace log4cplus {  namespace spi {
