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

// Framework include files
#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Printout.h>
#include <DD4hep/InstanceCount.h>
#include <DDAlign/GlobalAlignmentStack.h>

using namespace dd4hep::align;

static dd4hep::dd4hep_ptr<GlobalAlignmentStack>& _stack()  {
  static dd4hep::dd4hep_ptr<GlobalAlignmentStack> s;
  return s;
}
static dd4hep::dd4hep_ptr<GlobalAlignmentStack>& _stack(GlobalAlignmentStack* obj)  {
  dd4hep::dd4hep_ptr<GlobalAlignmentStack>& stk = _stack();
  stk.adopt(obj);
  return stk;
}

/// Constructor with partial initialization
GlobalAlignmentStack::StackEntry::StackEntry(DetElement element, const std::string& p, const Delta& del, double ov)
  : detector(element), delta(del), path(p), overlap(ov)
{
  InstanceCount::increment(this);
}

/// Copy constructor
GlobalAlignmentStack::StackEntry::StackEntry(const StackEntry& e)
  : detector(e.detector), delta(e.delta), path(e.path), overlap(e.overlap)
{
  InstanceCount::increment(this);
}

/// Default destructor
GlobalAlignmentStack::StackEntry::~StackEntry() {
  InstanceCount::decrement(this);
}
#if 0
/// Set flag to reset the entry to its ideal geometrical position
GlobalAlignmentStack::StackEntry& GlobalAlignmentStack::StackEntry::setReset(bool new_value)   {
  new_value ? (delta.flags |= RESET_VALUE) : (delta.flags &= ~RESET_VALUE);
  return *this;
}


/// Set flag to reset the entry's children to their ideal geometrical position
GlobalAlignmentStack::StackEntry& GlobalAlignmentStack::StackEntry::setResetChildren(bool new_value)   {
  new_value ? (delta.flags |= RESET_CHILDREN) : (delta.flags &= ~RESET_CHILDREN);
  return *this;
}


/// Set flag to check overlaps
GlobalAlignmentStack::StackEntry& GlobalAlignmentStack::StackEntry::setOverlapCheck(bool new_value)   {
  new_value ? (delta.flags |= CHECKOVL_DEFINED) : (delta.flags &= ~CHECKOVL_DEFINED);
  return *this;
}


/// Set the precision for the overlap check (otherwise the default is 0.001 cm)
GlobalAlignmentStack::StackEntry& GlobalAlignmentStack::StackEntry::setOverlapPrecision(double precision)   {
  delta.flags |= CHECKOVL_DEFINED;
  delta.flags |= CHECKOVL_VALUE;
  overlap = precision;
  return *this;
}
#endif

/// Default constructor
GlobalAlignmentStack::GlobalAlignmentStack()
{
  InstanceCount::increment(this);
}

/// Default destructor
GlobalAlignmentStack::~GlobalAlignmentStack()   {
  detail::destroyObjects(m_stack);
  InstanceCount::decrement(this);
}

/// Static client accessor
GlobalAlignmentStack& GlobalAlignmentStack::get()  {
  if ( _stack().get() ) return *_stack();
  except("GlobalAlignmentStack", "Stack not allocated -- may not be retrieved!");
  throw std::runtime_error("Stack not allocated");
}

/// Create an alignment stack instance. The creation of a second instance will be refused.
void GlobalAlignmentStack::create()   {
  if ( _stack().get() )   {
    except("GlobalAlignmentStack", "Stack already allocated. Multiple copies are not allowed!");
  }
  _stack(new GlobalAlignmentStack());
}

/// Check existence of alignment stack
bool GlobalAlignmentStack::exists()   {
  return _stack().get() != 0;
}

/// Clear data content and remove the slignment stack
void GlobalAlignmentStack::release()    {
  if ( _stack().get() )  {
    _stack(0);
    return;
  }
  except("GlobalAlignmentStack", "Attempt to delete non existing stack.");
}

/// Add a new entry to the cache. The key is the placement path
bool GlobalAlignmentStack::insert(const std::string& full_path, dd4hep_ptr<StackEntry>& entry)  {
  if ( entry.get() && !full_path.empty() )  {
    entry->path = full_path;
    return add(entry);
  }
  except("GlobalAlignmentStack", "Attempt to apply an invalid alignment entry.");
  return false;
}

/// Add a new entry to the cache. The key is the placement path
bool GlobalAlignmentStack::insert(dd4hep_ptr<StackEntry>& entry)  {
  return add(entry);
}

/// Add a new entry to the cache. The key is the placement path
bool GlobalAlignmentStack::add(dd4hep_ptr<StackEntry>& entry)  {
  if ( entry.get() && !entry->path.empty() )  {
    Stack::const_iterator i = m_stack.find(entry->path);
    if ( i == m_stack.end() )   {
      StackEntry* e = entry.get();
      // Need to make some checks BEFORE insertion
      if ( !e->detector.isValid() )   {
        except("GlobalAlignmentStack", "Invalid alignment entry [No such detector]");
      }
      printout(INFO,"GlobalAlignmentStack","Add node:%s",e->path.c_str());
      m_stack.emplace(e->path,entry.release());
      return true;
    }
    except("GlobalAlignmentStack", "The entry with path "+entry->path+
           " cannot be re-aligned twice in one transaction.");
  }
  except("GlobalAlignmentStack", "Attempt to apply an invalid alignment entry.");
  return false;
}

/// Retrieve an alignment entry of the current stack
dd4hep::dd4hep_ptr<GlobalAlignmentStack::StackEntry> GlobalAlignmentStack::pop()   {
  Stack::iterator i = m_stack.begin();
  if ( i != m_stack.end() )   {
    dd4hep_ptr<StackEntry> e((*i).second);
    m_stack.erase(i);
    return e;
  }
  except("GlobalAlignmentStack", "Alignment stack is empty. "
         "Cannot pop entries - check size first!");
  return {};
}

/// Get all pathes to be aligned
std::vector<const GlobalAlignmentStack::StackEntry*> GlobalAlignmentStack::entries() const    {
  std::vector<const StackEntry*> result;
  result.reserve(m_stack.size());
  transform(begin(m_stack),end(m_stack),back_inserter(result),detail::select2nd(m_stack));
  return result;
}

