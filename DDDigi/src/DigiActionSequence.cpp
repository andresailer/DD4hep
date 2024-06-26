//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

/// Framework include files
#include <DD4hep/InstanceCount.h>
#include <DDDigi/DigiActionSequence.h>

// C/C++ include files
#include <stdexcept>

using namespace dd4hep::digi;

/// Standard constructor
DigiActionSequence::DigiActionSequence(const DigiKernel& kernel, const std::string& nam)
  : DigiSynchronize(kernel, nam)
{
  InstanceCount::increment(this);
}

/// Default destructor
DigiActionSequence::~DigiActionSequence() {
  m_begin.clear();
  m_end.clear();
  InstanceCount::decrement(this);
}

/// Adopt a new action as part of the sequence. Sequence takes ownership.
void DigiActionSequence::adopt(DigiEventAction* action)    {
  this->DigiSynchronize::adopt(action);
}

/// Pre-track action callback
void DigiActionSequence::execute(DigiContext& context)  const   {
  m_begin(&context);
  this->DigiSynchronize::execute(context);
  debug("+++ Event: %8d (DigiActionSequence) Parallel: %s Done.",
       context.event->eventNumber, yes_no(m_parallel));
  m_end(&context);
}

/// Standard constructor
DigiSequentialActionSequence::DigiSequentialActionSequence(const DigiKernel& kernel, const std::string& nam)
  : DigiActionSequence(kernel, nam)
{
  this->m_parallel = false;
  InstanceCount::increment(this);
}

/// Default destructor
DigiSequentialActionSequence::~DigiSequentialActionSequence() {
  InstanceCount::decrement(this);
}

/// Standard constructor
DigiParallelActionSequence::DigiParallelActionSequence(const DigiKernel& kernel, const std::string& nam)
  : DigiActionSequence(kernel, nam)
{
  this->m_parallel = true;
  InstanceCount::increment(this);
}

/// Default destructor
DigiParallelActionSequence::~DigiParallelActionSequence() {
  InstanceCount::decrement(this);
}
