/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/


// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkEventSpy.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <map>
#include <sstream>


//----------------------------------------------------------------------------
class vtkEventSpy::vtkInternal
{
public:
  typedef vtkInternal Self;

  vtkInternal(vtkEventSpy* external);

  static void UpdateEvent(vtkEventSpyEntry* event,
                          vtkObject* caller,
                          unsigned long eventId,
                          const void* callData,
                          int callDataType);

  static void SpyCallback(vtkObject * caller,
                          unsigned long eventId,
                          void * clientData,
                          void * callerData);

  vtkSmartPointer<vtkCallbackCommand> Spy;

  vtkSmartPointer<vtkVariantArray> Events;

  std::map<unsigned long, int> EventIdToCallDataType;
  std::map<unsigned long, vtkEventSpy::EventPropertyRecorder*> EventIdToEventPropertiesRecorder;

  // Since vtkVariant increases the reference count of associated VTK object,
  // we will instead associate an "ObjectId" with the "CallData" property, and
  // use a map to link "ObjectId" with an "object weak pointer".
//  static std::map<std::string, vtkWeakPointer<vtkObject> > ObjectIdToWeakPointer;

  vtkEventSpy* External;
};

//----------------------------------------------------------------------------
vtkEventSpy::vtkInternal::vtkInternal(vtkEventSpy *external) : External(external)
{
  this->Spy = vtkSmartPointer<vtkCallbackCommand>::New();
  this->Spy->SetCallback(Self::SpyCallback);
  this->Spy->SetClientData(this->External);

  this->Events = vtkSmartPointer<vtkVariantArray>::New();
}

//----------------------------------------------------------------------------
void vtkEventSpy::vtkInternal::UpdateEvent(vtkEventSpyEntry* event,
                                           vtkObject* caller,
                                           unsigned long eventId,
                                           const void* callData,
                                           int callDataType)
{
  if (!event)
    {
    vtkGenericWarningMacro("UpdateEvent: event is NULL !");
    return;
    }
  event->InsertValue(EventCaller, vtkVariant(caller));
  event->InsertValue(EventId, vtkVariant(eventId));
  vtkVariant callDataVariant;
  switch(callDataType)
    {
  case String:
    callDataVariant =
        vtkVariant(reinterpret_cast<char*>(const_cast<void*>(callData)));
    break;
  case Integer:
    callDataVariant =
        vtkVariant(*reinterpret_cast<int*>(const_cast<void*>(callData)));
    break;
  case VTKObject:
    {
    vtkObjectBase* object =
        reinterpret_cast<vtkObjectBase*>(const_cast<void*>(callData));
    std::ostringstream os;
    os << "(" << object->GetClassName() << ")" << hex << object << dec;
    callDataVariant = vtkVariant(os.str());
    }
    break;
  default:
    callDataVariant = vtkVariant("(unknown)");
    }
  event->InsertValue(EventCallData, callDataVariant);
}

//----------------------------------------------------------------------------
void vtkEventSpy::vtkInternal::SpyCallback(vtkObject * caller,
                                           unsigned long eventId,
                                           void * clientData,
                                           void * callData)
{
  vtkEventSpy* spy = reinterpret_cast<vtkEventSpy*>(clientData);

  vtkNew<vtkEventSpyEntry> event;
  vtkInternal::UpdateEvent(
        event.GetPointer(), caller, eventId,
        callData, spy->GetCallDataType(eventId));

  vtkEventSpy::EventPropertyRecorder* recorder =
      spy->GetEventPropertyRecorder(eventId);
  if (recorder)
    {
    (*recorder)(event.GetPointer());
    }

  spy->RecordEventProperties(event.GetPointer());

  spy->Internal->Events->InsertNextValue(vtkVariant(event.GetPointer()));
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEventSpy);

//----------------------------------------------------------------------------
vtkEventSpy::vtkEventSpy()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkEventSpy::~vtkEventSpy()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
bool vtkEventSpy::IsSupportedCallDataType(int callDataType)
{
  switch(callDataType)
    {
  case Unknown:
  case String:
  case Integer:
  case VTKObject:
    return true;
  default:
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkEventSpy::SetCallDataType(unsigned long eventId, int callDataType)
{
  if (!Self::IsSupportedCallDataType(callDataType))
    {
    callDataType = Unknown;
    }
  this->Internal->EventIdToCallDataType[eventId] = callDataType;
}

//----------------------------------------------------------------------------
int vtkEventSpy::GetCallDataType(unsigned long eventId)
{
  std::map<unsigned long, int>::iterator it;
  it = this->Internal->EventIdToCallDataType.find(eventId);
  if (it != this->Internal->EventIdToCallDataType.end())
    {
    return it->second;
    }
  return Unknown;
}

//----------------------------------------------------------------------------
void vtkEventSpy::ResetCallDataTypes()
{
  this->Internal->EventIdToCallDataType.clear();
}

//----------------------------------------------------------------------------
void vtkEventSpy::SetEventPropertyRecorder(unsigned long eventId,
                                             EventPropertyRecorder* recorder)
{
  this->Internal->EventIdToEventPropertiesRecorder[eventId] = recorder;
}

//----------------------------------------------------------------------------
vtkEventSpy::EventPropertyRecorder*
vtkEventSpy::GetEventPropertyRecorder(unsigned long eventId)
{
  std::map<unsigned long, EventPropertyRecorder*>::iterator it;
  it = this->Internal->EventIdToEventPropertiesRecorder.find(eventId);
  if (it != this->Internal->EventIdToEventPropertiesRecorder.end())
    {
    return it->second;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkEventSpy::ResetEventPropertyRecorders()
{
  this->Internal->EventIdToEventPropertiesRecorder.clear();
}

//----------------------------------------------------------------------------
vtkCommand* vtkEventSpy::GetSpy()
{
  return this->Internal->Spy;
}

//----------------------------------------------------------------------------
vtkIdType vtkEventSpy::GetCount()
{
  return this->Internal->Events->GetNumberOfValues();
}

//----------------------------------------------------------------------------
vtkIdType vtkEventSpy::GetTotalNumberOfEvents()
{
  return this->GetCount();
}

//----------------------------------------------------------------------------
vtkIdType vtkEventSpy::GetCountByEventId(unsigned long eventId)
{
  vtkIdType count = 0;
  for(vtkIdType index = 0; index < this->GetCount(); ++index)
    {
    if (Self::GetEventId(this->GetNthEvent(index)) == eventId)
      {
      ++count;
      }
    }
  return count;
}

//----------------------------------------------------------------------------
vtkIdType vtkEventSpy::GetNumberOfEvents(unsigned long eventId)
{
  return this->GetCountByEventId(eventId);
}

//----------------------------------------------------------------------------
vtkEventSpyEntry* vtkEventSpy::GetNthEvent(vtkIdType index)
{
  if (index >= this->GetCount())
    {
    return 0;
    }
  vtkVariant value = this->Internal->Events->GetValue(index);
  if (!value.IsVTKObject())
    {
    return 0;
    }
  return vtkEventSpyEntry::SafeDownCast(value.ToVTKObject());
}

//----------------------------------------------------------------------------
void vtkEventSpy::ResetEvents()
{
  this->Internal->Events->Reset();
}

//----------------------------------------------------------------------------
void vtkEventSpy::ResetNumberOfEvents()
{
  this->ResetEvents();
}

//----------------------------------------------------------------------------
vtkObject* vtkEventSpy::GetEventCaller(vtkEventSpyEntry* event)
{
  vtkVariant value = event->GetValue(EventCaller);
  if (!value.IsVTKObject())
    {
    return 0;
    }
  return vtkObject::SafeDownCast(value.ToVTKObject());
}

//----------------------------------------------------------------------------
unsigned long vtkEventSpy::GetEventId(vtkEventSpyEntry *event)
{
  vtkVariant value = event->GetValue(EventId);
  if (!value.IsUnsignedLong())
    {
    return vtkCommand::NoEvent;
    }
  return value.ToUnsignedLong();
}

//----------------------------------------------------------------------------
std::string vtkEventSpy::GetEventCallData(vtkEventSpyEntry *event)
{
  vtkVariant value = event->GetValue(EventCallData);
  if (!value.IsString())
    {
    return std::string();
    }
  return event->GetValue(EventCallData).ToString();
}

//----------------------------------------------------------------------------
void* vtkEventSpy::GetEventCallDataAsVoid(vtkEventSpyEntry* event)
{
  std::string callData = Self::GetEventCallData(event);
  if (callData.empty())
    {
    return 0;
    }

  std::string::size_type startAddress;
  startAddress = callData.find(")") + 1;

  std::string callDataAddress =
      std::string(callData.begin() + startAddress, callData.end());

  void* callDataPointer;
  sscanf(callDataAddress.c_str(), "%p", &callDataPointer);

  return callDataPointer;
}

//----------------------------------------------------------------------------
void vtkEventSpy::RecordEventProperties(vtkEventSpyEntry* vtkNotUsed(event))
{
}

//----------------------------------------------------------------------------
void vtkEventSpy::UpdateEvent(vtkEventSpyEntry* event,
                              vtkObject* caller,
                              unsigned long eventId)
{
  vtkInternal::UpdateEvent(event, caller, eventId, 0, Unknown);
}

//----------------------------------------------------------------------------
void vtkEventSpy::UpdateEvent(vtkEventSpyEntry* event,
                              vtkObject* caller,
                              unsigned long eventId,
                              const char* callData)
{
  vtkInternal::UpdateEvent(event, caller, eventId, callData, String);
}

//----------------------------------------------------------------------------
void vtkEventSpy::UpdateEvent(vtkEventSpyEntry* event,
                              vtkObject* caller,
                              unsigned long eventId,
                              const int* callData)
{
  vtkInternal::UpdateEvent(event, caller, eventId, callData, Integer);
}

//----------------------------------------------------------------------------
void vtkEventSpy::UpdateEvent(vtkEventSpyEntry* event,
                              vtkObject* caller,
                              unsigned long eventId,
                              const vtkObjectBase* callData)
{
  vtkInternal::UpdateEvent(event, caller, eventId, callData, VTKObject);
}

//----------------------------------------------------------------------------
bool vtkEventSpy::AreEventEqual(vtkEventSpyEntry* event1,
                                vtkEventSpyEntry* event2,
                                int options)
{
  return AreEventEqual(event1, event2, "1", "2", options);
}

//----------------------------------------------------------------------------
bool vtkEventSpy::AreEventEqual(vtkEventSpyEntry* event1,
                                vtkEventSpyEntry* event2,
                                const std::string& descriptionEvent1,
                                const std::string& descriptionEvent2,
                                int options)
{
  bool verbose = !(options & Quiet);
  bool ignoreCustomProperties = options & IgnoreCustomProperties;

  // NULL values are always equal to one another and
  // unequal to anything else.
  if (!(event1 && event2))
    {
    return event1 == event2;
    }

  if (ignoreCustomProperties)
    {
    // events are expected to have at least three values: EventCaller, EventId
    // and EventCallData.
    if (event1->GetNumberOfValues() < 3)
      {
      if (verbose)
        {
        vtkGenericWarningMacro(
              << "Event " << descriptionEvent1 << " has size < 3"
              << "\n\tsize " << descriptionEvent1 << ": " << event1->GetNumberOfValues()
              << "\n\tcontent " << descriptionEvent1 << ": " << Self::ToString(event1));
        }
      return false;
      }
    if (event2->GetNumberOfValues() < 3)
      {
      if (verbose)
        {
        vtkGenericWarningMacro(
              << "Event " << descriptionEvent2 << " has size < 3"
              << "\n\tsize " << descriptionEvent2 << ": " << event2->GetNumberOfValues()
              << "\n\tcontent " << descriptionEvent2 << ": " << Self::ToString(event2));
        }
      return false;
      }
    }
  else
    {
    if (event1->GetNumberOfValues() != event2->GetNumberOfValues())
      {
      if (verbose)
        {
        vtkGenericWarningMacro(
              << "Arrays have different size"
              << "\n\tsize " << descriptionEvent1 << ": " << event1->GetNumberOfValues()
              << "\n\tsize " << descriptionEvent2 << ": " << event2->GetNumberOfValues()
              << "\n\tcontent " << descriptionEvent1 << ": " << Self::ToString(event1)
              << "\n\tcontent " << descriptionEvent2 << ": " << Self::ToString(event2));
        }
      return false;
      }
    }

  unsigned long eventId1 = event1->GetValue(vtkEventSpy::EventId).ToInt();
  unsigned long eventId2 = event2->GetValue(vtkEventSpy::EventId).ToInt();
  if (eventId1 != eventId2)
    {
    if (verbose)
      {
      vtkGenericWarningMacro(
            << "Arrays have different eventId"
            << "\n\teventId " << descriptionEvent1 << ": " << eventId1
            << "\n\teventId " << descriptionEvent2 << ": " << eventId2
            << "\n\tcontent " << descriptionEvent1 << ": " << Self::ToString(event1)
            << "\n\tcontent " << descriptionEvent2 << ": " << Self::ToString(event2));
      }
    return false;
    }

  vtkObjectBase* caller1 = event1->GetValue(vtkEventSpy::EventCaller).ToVTKObject();
  vtkObjectBase* caller2 = event2->GetValue(vtkEventSpy::EventCaller).ToVTKObject();
  if (caller1 != caller2)
    {
    if (verbose)
      {
      vtkGenericWarningMacro(
            << "Arrays have different caller"
            << "\n\tcaller " << descriptionEvent1 << ": " << caller1
            << "\n\tcaller " << descriptionEvent2 << ": " << caller2
            << "\n\tcontent " << descriptionEvent1 << ": " << Self::ToString(event1)
            << "\n\tcontent " << descriptionEvent2 << ": " << Self::ToString(event2));
      }
    return false;
    }

  vtkVariant callData1 = event1->GetValue(vtkEventSpy::EventCallData);
  vtkVariant callData2 = event2->GetValue(vtkEventSpy::EventCallData);
  if ((callData2 != "(unknown)") && (callData1 != callData2))
    {
    if (verbose)
      {
      vtkGenericWarningMacro(
            << "Arrays have different callData"
            << "\n\tcallData " << descriptionEvent1 << ": " << callData1
            << "\n\tcallData " << descriptionEvent2 << ": " << callData2
            << "\n\tcontent " << descriptionEvent1 << ": " << Self::ToString(event1)
            << "\n\tcontent " << descriptionEvent2 << ": " << Self::ToString(event2));
      }
    return false;
    }

  if (!ignoreCustomProperties)
    {

    for(vtkIdType index = EventDefaultPropertyCount;
        index < event1->GetNumberOfValues();
        ++index)
      {
      vtkVariant value1 = event1->GetValue(index);
      vtkVariant value2 = event2->GetValue(index);
      if (value1 != value2)
        {
        if (verbose)
          {
          vtkGenericWarningMacro(
                << "Arrays have different variants at index "
                << index
                << "\n\tvariants[" << index << "] " << descriptionEvent1 << ": "
                  << value1 << " " << value1.GetTypeAsString()
                << "\n\tvariants[" << index << "] " << descriptionEvent2 << ": "
                  << value2 << " " << value2.GetTypeAsString()
                << "\n\tcontent " << descriptionEvent1 << ": " << Self::ToString(event1)
                << "\n\tcontent " << descriptionEvent2 << ": " << Self::ToString(event2));
          }
        return false;
        }
      }
    }

  return true;
}

//----------------------------------------------------------------------------
std::string vtkEventSpy::ToString(vtkEventSpyEntry* event)
{
  std::ostringstream os;
  if (!event)
    {
    os << "(null)";
    return os.str();
    }
  if (event->GetNumberOfValues() == 0)
    {
    os << "(empty)";
    return os.str();
    }
  for (vtkIdType index = 0; index < event->GetNumberOfValues(); ++index)
    {
    if (index > 0)
      {
      os << " ";
      }
    os << event->GetValue(index);
    }
  return os.str();
}

//----------------------------------------------------------------------------
// XXX Commented this code. It gives the following warning:
//   warning: ISO C++ says that these are ambiguous, even though
//   the worst conversion for the first is better than the worst
//   conversion for the second:
//ostream& operator << (ostream& os, vtkEventSpyEntry* event)
//{
//  os << vtkEventSpy::ToString(event);
//  return os;
//}
