// Module:  Log4CPLUS
// File:    loggerimpl.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/spi/rootlogger.h>
#include <stdexcept>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;



//////////////////////////////////////////////////////////////////////////////
// Logger Constructors and Destructor
//////////////////////////////////////////////////////////////////////////////
LoggerImpl::LoggerImpl(const log4cplus::tstring& name, Hierarchy& h)
  : name(name),
    ll(NOT_SET_LOG_LEVEL),
    parent(NULL),
    additive(true), 
    hierarchy(h)
{
}


LoggerImpl::~LoggerImpl() 
{ 
}


//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

void 
LoggerImpl::callAppenders(const InternalLoggingEvent& event)
{
    int writes = 0;
    for(const LoggerImpl* c = this; c != NULL; c=c->parent.get()) {
        writes += c->appendLoopOnAppenders(event);
        if(!c->additive) {
            break;
        }
    }

    // No appenders in hierarchy, warn user only once.
    if(!hierarchy.emittedNoAppenderWarning && writes == 0) {
        getLogLog().error(  LOG4CPLUS_TEXT("No appenders could be found for logger (") 
                          + getName() 
                          + LOG4CPLUS_TEXT(")."));
        getLogLog().error(LOG4CPLUS_TEXT("Please initialize the log4cplus system properly."));
        hierarchy.emittedNoAppenderWarning = true;
    }
}


void 
LoggerImpl::closeNestedAppenders()
{
    SharedAppenderPtrList appenders = getAllAppenders();
    for(SharedAppenderPtrList::iterator it=appenders.begin(); it!=appenders.end(); ++it)
    {
        (*it)->close();
    }
}


bool 
LoggerImpl::isEnabledFor(LogLevel _ll) const
{
    if(hierarchy.disableValue >= _ll) {
        return false;
    }
    return _ll >= getChainedLogLevel();
}


void 
LoggerImpl::log(LogLevel _ll, 
                const log4cplus::tstring& message,
                const char* file, 
                int line)
{
    if(isEnabledFor(_ll)) {
        forcedLog(_ll, message, file, line);
    }
}



LogLevel 
LoggerImpl::getChainedLogLevel() const
{
    for(const LoggerImpl *c=this; c != NULL; c=c->parent.get()) 
	{
        if(c->ll != NOT_SET_LOG_LEVEL) 
		{
            return c->ll;
        }
    }

    getLogLog().error(LOG4CPLUS_TEXT("LoggerImpl::getChainedLogLevel()- No valid LogLevel found") );
    throw std::runtime_error("No valid LogLevel found");

	// Will never return this. Avoids warnings in some compilers
	return NOT_SET_LOG_LEVEL;
}


Hierarchy& 
LoggerImpl::getHierarchy() const
{ 
    return hierarchy; 
}


bool 
LoggerImpl::getAdditivity() const
{ 
    return additive; 
}


void 
LoggerImpl::setAdditivity(bool _additive)
{ 
    this->additive = _additive;
}


void 
LoggerImpl::forcedLog(LogLevel _ll,
                      const log4cplus::tstring& message,
                      const char* file, 
                      int line)
{
    callAppenders(spi::InternalLoggingEvent(this->getName(), _ll, message, file, line));
}



