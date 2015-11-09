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
#include <vtkCommand.h>
#include <vtkEventSpy.h>
#include <vtkNew.h>
#include <vtkVariantArray.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
bool TestDefaultValues();
bool TestEnum();
bool TestSetGetCallDataType();
bool TestSetGetEventPropertyRecorders();
bool TestGetEventCallDataAsVoid();
bool TestUpdateEventWithEmptyArray();
bool TestUpdateEventWithInitializedArray();
bool TestEventRecording();
bool TestEventRecordingWithCallData();
bool TestEventPropertyRecorderUsingDerivation();
bool TestEventPropertyRecorderUsingFunctor();

//----------------------------------------------------------------------------
int vtkEventSpyTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  bool res = true;
  res = res && TestDefaultValues();
  res = res && TestEnum();
  res = res && TestSetGetCallDataType();
  res = res && TestSetGetEventPropertyRecorders();
  res = res && TestGetEventCallDataAsVoid();
  res = res && TestUpdateEventWithEmptyArray();
  res = res && TestUpdateEventWithInitializedArray();
  res = res && TestEventRecording();
  res = res && TestEventRecordingWithCallData();
  res = res && TestEventPropertyRecorderUsingDerivation();
  res = res && TestEventPropertyRecorderUsingFunctor();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
template<typename Type>
std::string ToString(Type value)
{
  std::ostringstream stream;
  stream << value;
  return stream.str();
}

//----------------------------------------------------------------------------
bool CheckNotNull(int line, const std::string& description, void* pointer)
{
  if (!pointer)
    {
    std::cerr << "\nLine " << line << " - " << description
              << " : CheckNotNull failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
template<typename Type>
bool Check(int line, const std::string& description,
              Type current, Type expected)
{
  if(current != expected)
    {
    std::cerr << "\nLine " << line << " - " << description << " : Check failed"
              << "\n\tcurrent:" << current
              << "\n\texpected:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckEvent(int line, const std::string& description,
                vtkEventSpyEntry* current, vtkEventSpyEntry* expected)
{
  bool equal = vtkEventSpy::AreEventEqual(
        current, expected, "current", "expected");
  if (!equal)
    {
    std::cerr << "\nLine " << line
              << " - Problem with " << description << std::endl;
    }
  return equal;
}

//----------------------------------------------------------------------------
bool TestDefaultValues()
{
  vtkNew<vtkEventSpy> spy;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), 0))
    {
    return false;
    }

  if (!Check<vtkIdType>(
        __LINE__, "GetCountByEventId",
        spy->GetCountByEventId(0), 0))
    {
    return false;
    }

  if (!Check<int>(
        __LINE__, "GetCallDataType",
        spy->GetCallDataType(vtkCommand::ModifiedEvent), vtkEventSpy::Unknown))
    {
    return false;
    }

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder",
        spy->GetEventPropertyRecorder(vtkCommand::ModifiedEvent),
        0))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestEnum()
{
  if (!Check<int>(
        __LINE__, "vtkEventSpy-EventCustomProperty",
        vtkEventSpy::EventDefaultPropertyCount, vtkEventSpy::EventCallData + 1))
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestSetGetCallDataType()
{
  vtkNew<vtkEventSpy> spy;

  enum
  {
    CustomEvent1 = vtkCommand::UserEvent,
    CustomEvent2,
  };

  spy->SetCallDataType(CustomEvent1, vtkEventSpy::VTKObject);

  if (!Check<int>(
        __LINE__, "GetEventCallDataType-CustomEvent1-VTKObject",
        spy->GetCallDataType(CustomEvent1), vtkEventSpy::VTKObject))
    {
    return false;
    }

  spy->SetCallDataType(CustomEvent2, vtkEventSpy::String);

  if (!Check<int>(
        __LINE__, "GetEventCallDataType-CustomEvent2-String",
        spy->GetCallDataType(CustomEvent2), vtkEventSpy::String))
    {
    return false;
    }

  spy->SetCallDataType(CustomEvent2, vtkEventSpy::Integer);

  if (!Check<int>(
        __LINE__, "GetEventCallDataType-CustomEvent2-Integer",
        spy->GetCallDataType(CustomEvent2), vtkEventSpy::Integer))
    {
    return false;
    }

  spy->SetCallDataType(CustomEvent2, -1);

  if (!Check<int>(
        __LINE__, "GetEventCallDataType-CustomEvent2-Unknown",
        spy->GetCallDataType(CustomEvent2), vtkEventSpy::Unknown))
    {
    return false;
    }

  spy->ResetCallDataTypes();

  if (!Check<int>(
        __LINE__, "GetEventCallDataType-CustomEvent1-Unknown",
        spy->GetCallDataType(CustomEvent1), vtkEventSpy::Unknown))
    {
    return false;
    }

  if (!Check<int>(
        __LINE__, "GetEventCallDataType-CustomEvent2-Unknown",
        spy->GetCallDataType(CustomEvent2), vtkEventSpy::Unknown))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestSetGetEventPropertyRecorders()
{
  vtkNew<vtkEventSpy> spy;

  enum
  {
    CustomEvent1 = vtkCommand::UserEvent,
    CustomEvent2
  };

  struct Recorder1 : vtkEventSpy::EventPropertyRecorder
  {
    virtual void operator () (vtkEventSpyEntry* vtkNotUsed(event))
    {
    }
  };

  struct Recorder2 : vtkEventSpy::EventPropertyRecorder
  {
    virtual void operator () (vtkEventSpyEntry* vtkNotUsed(event))
    {
    }
  };

  Recorder1 recorder1;

  spy->SetEventPropertyRecorder(CustomEvent1, &recorder1);

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder-CustomEvent1-Recorder1",
        spy->GetEventPropertyRecorder(CustomEvent1), &recorder1))
    {
    return false;
    }

  Recorder2 recorder2;

  spy->SetEventPropertyRecorder(CustomEvent2, &recorder2);

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder-CustomEvent2-Recorder2",
        spy->GetEventPropertyRecorder(CustomEvent2), &recorder2))
    {
    return false;
    }

  spy->SetEventPropertyRecorder(CustomEvent2, &recorder1);

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder-CustomEvent2-Recorder1",
        spy->GetEventPropertyRecorder(CustomEvent2), &recorder1))
    {
    return false;
    }

  spy->SetEventPropertyRecorder(CustomEvent2, 0);

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder-CustomEvent2-Null",
        spy->GetEventPropertyRecorder(CustomEvent2), 0))
    {
    return false;
    }

  spy->ResetEventPropertyRecorders();

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder-CustomEvent1-Null",
        spy->GetEventPropertyRecorder(CustomEvent1), 0))
    {
    return false;
    }

  if (!Check<vtkEventSpy::EventPropertyRecorder*>(
        __LINE__, "GetEventPropertyRecorder-CustomEvent2-Null",
        spy->GetEventPropertyRecorder(CustomEvent2), 0))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestGetEventCallDataAsVoid()
{
  vtkNew<vtkObject> caller;

  vtkNew<vtkObject> callData;

  vtkNew<vtkEventSpyEntry> event;
  vtkEventSpy::UpdateEvent(
        event.GetPointer(), caller.GetPointer(),
        42, callData.GetPointer());

  void* eventCallData =
      vtkEventSpy::GetEventCallDataAsVoid(event.GetPointer());

  if (!Check<void*>(
        __LINE__, "GetEventCallDataAsVoid",
        eventCallData, callData.GetPointer()))
    {
    return false;
    }

  vtkObject* objectCallData = reinterpret_cast<vtkObject*>(eventCallData);

  if (!Check<vtkObject*>(
        __LINE__, "GetEventCallDataAsVoid-ConvertedToVTKObject",
        objectCallData, callData.GetPointer()))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestUpdateEventWithEmptyArray()
{
  vtkNew<vtkObject> foo;
  vtkNew<vtkEventSpyEntry> event;
  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-GetNumberOfValues",
        event->GetNumberOfValues(), 0))
    {
    return false;
    }
  vtkEventSpy::UpdateEvent(
        event.GetPointer(), foo.GetPointer(), vtkCommand::ModifiedEvent);

  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-GetNumberOfValues",
        event->GetNumberOfValues(), 3))
    {
    return false;
    }
  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-EventId",
        event->GetValue(vtkEventSpy::EventId).ToInt(), vtkCommand::ModifiedEvent))
    {
    return false;
    }
  if (!Check<vtkObjectBase*>(
        __LINE__, "vtkEventSpyEntry-EventCaller",
        event->GetValue(vtkEventSpy::EventCaller).ToVTKObject(),
        foo.GetPointer()))
    {
    return false;
    }
  if (!Check<std::string>(
        __LINE__, "vtkEventSpyEntry-EventCallData-ToString",
        event->GetValue(vtkEventSpy::EventCallData).ToString(),
        "(unknown)"))
    {
    return false;
    }

  std::string callData("hello");

  vtkEventSpy::UpdateEvent(
        event.GetPointer(), foo.GetPointer(), vtkCommand::UserEvent,
        callData.c_str());

  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-GetNumberOfValues",
        event->GetNumberOfValues(), 3))
    {
    return false;
    }
  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-EventId",
        event->GetValue(vtkEventSpy::EventId).ToInt(), vtkCommand::UserEvent))
    {
    return false;
    }
  if (!Check<vtkObjectBase*>(
        __LINE__, "vtkEventSpyEntry-EventCaller",
        event->GetValue(vtkEventSpy::EventCaller).ToVTKObject(),
        foo.GetPointer()))
    {
    return false;
    }
  if (!Check<bool>(
        __LINE__, "vtkEventSpyEntry-EventCallData-IsValid",
        event->GetValue(vtkEventSpy::EventCallData).IsValid(),
        true))
    {
    return false;
    }
  if (!Check<std::string>(
        __LINE__, "vtkEventSpyEntry-EventCallData-IsValid",
        event->GetValue(vtkEventSpy::EventCallData).ToString(),
        callData))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestUpdateEventWithInitializedArray()
{
  // Initialize a vtkEventSpyEntry with 10 integers.
  // Only the first three values are expected to be updated.

  vtkNew<vtkObject> foo;
  vtkNew<vtkEventSpyEntry> event;
  vtkIdType eventPropertyCount = 10;
  event->SetNumberOfValues(eventPropertyCount);
  for(int index = 0; index < eventPropertyCount; ++index)
    {
    event->SetValue(index, vtkVariant(index));
    }

  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-GetNumberOfValues",
        event->GetNumberOfValues(), eventPropertyCount))
    {
    return false;
    }

  vtkEventSpy::UpdateEvent(
        event.GetPointer(), foo.GetPointer(), vtkCommand::ModifiedEvent);

  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-GetNumberOfValues",
        event->GetNumberOfValues(), eventPropertyCount))
    {
    return false;
    }

  if (!Check<vtkIdType>(
        __LINE__, "vtkEventSpyEntry-EventId",
        event->GetValue(vtkEventSpy::EventId).ToInt(), vtkCommand::ModifiedEvent))
    {
    return false;
    }
  if (!Check<vtkObjectBase*>(
        __LINE__, "vtkEventSpyEntry-EventCaller",
        event->GetValue(vtkEventSpy::EventCaller).ToVTKObject(),
        foo.GetPointer()))
    {
    return false;
    }

  if (!Check<std::string>(
        __LINE__, "vtkEventSpyEntry-EventCallData-ToString",
        event->GetValue(vtkEventSpy::EventCallData).ToString(),
        "(unknown)"))
    {
    return false;
    }

  for (vtkIdType index = vtkEventSpy::EventDefaultPropertyCount;
       index < eventPropertyCount; ++index)
    {
    if (!Check<int>(
          __LINE__, "vtkEventSpyEntry-Property-" + ToString<vtkIdType>(index),
          event->GetValue(index).ToInt(),
          index))
      {
      return false;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestEventRecording()
{
  vtkNew<vtkEventSpy> spy;

  vtkNew<vtkObject> foo;
  foo->AddObserver(vtkCommand::ModifiedEvent, spy->GetSpy());

  int count = 0;

  // Event 0
  foo->Modified();
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  if (!Check<vtkIdType>(
        __LINE__, "GetCountByEventId",
        spy->GetCountByEventId(vtkCommand::ModifiedEvent), count))
    {
    return false;
    }

  // Event 1
  foo->Modified();
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  if (!Check<vtkIdType>(
        __LINE__, "GetCountByEventId",
        spy->GetCountByEventId(vtkCommand::ModifiedEvent), count))
    {
    return false;
    }

  // Check events
  vtkNew<vtkEventSpyEntry> expected;
  vtkEventSpy::UpdateEvent(
        expected.GetPointer(), foo.GetPointer(), vtkCommand::ModifiedEvent);

  for (vtkIdType index = 0; index < spy->GetCount(); ++index)
    {
    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expected.GetPointer()))
      {
      return false;
      }
    }

  spy->ResetEvents();

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), 0))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestEventRecordingWithCallData()
{

  // CallData of type VTKObject is tested in:
  //  * TestEventPropertyRecorderUsingDerivation
  //  * TestEventPropertyRecorderUsingFunctor

  enum
  {
    IntEvent = vtkCommand::UserEvent,
    StringEvent,
    OtherEvent,
  };

  vtkNew<vtkEventSpy> spy;
  spy->SetCallDataType(IntEvent, vtkEventSpy::Integer);
  spy->SetCallDataType(StringEvent, vtkEventSpy::String);
  // Do not set 'OtherEvent'. We will test the case of missing association.

  vtkNew<vtkObject> foo;
  foo->AddObserver(vtkCommand::ModifiedEvent, spy->GetSpy());
  foo->AddObserver(IntEvent, spy->GetSpy());
  foo->AddObserver(StringEvent, spy->GetSpy());
  foo->AddObserver(OtherEvent, spy->GetSpy());

  int count = 0;

  // Event 0
  foo->Modified();
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  // Event 1
  int intEventCallData = 42;
  foo->InvokeEvent(IntEvent, &intEventCallData);
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  // Event 2
  foo->Modified();
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  // Event 3

  std::string stringEventCallData = "Hello";
  foo->InvokeEvent(StringEvent, const_cast<char*>(stringEventCallData.c_str()));
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  // Event 4

  int otherEventCallData = 42;
  foo->InvokeEvent(OtherEvent, &otherEventCallData);
  ++count;

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), count))
    {
    return false;
    }

  // Check events

  int index = 0;
  {
    vtkNew<vtkEventSpyEntry> expected;
    vtkEventSpy::UpdateEvent(
          expected.GetPointer(), foo.GetPointer(),
          vtkCommand::ModifiedEvent);

    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expected.GetPointer()))
      {
      return false;
      }
  }

  index = 1;
  {
    vtkNew<vtkEventSpyEntry> expectedEvent;
    vtkEventSpy::UpdateEvent(
          expectedEvent.GetPointer(), foo.GetPointer(),
          IntEvent, &intEventCallData);

    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expectedEvent.GetPointer()))
      {
      return false;
      }
  }

  index = 2;
  {
    vtkNew<vtkEventSpyEntry> expected;
    vtkEventSpy::UpdateEvent(
          expected.GetPointer(), foo.GetPointer(), vtkCommand::ModifiedEvent);

    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expected.GetPointer()))
      {
      return false;
      }
  }

  index = 3;
  {
    vtkNew<vtkEventSpyEntry> expectedEvent;
    vtkEventSpy::UpdateEvent(
          expectedEvent.GetPointer(), foo.GetPointer(),
          StringEvent, stringEventCallData.c_str());

    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expectedEvent.GetPointer()))
      {
      return false;
      }

    // Modify the original string
    stringEventCallData[0] = 'B';

    // Check that the information recorded by the Spy is not modified
    vtkEventSpy::UpdateEvent(
          expectedEvent.GetPointer(), foo.GetPointer(),
          StringEvent, "Hello");

    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expectedEvent.GetPointer()))
      {
      return false;
      }

  }

  index = 4; // Since callDataType hasn't been registered, expected
             // callDataType is 'Unknown'
  {
    vtkNew<vtkEventSpyEntry> expectedEvent;
    vtkEventSpy::UpdateEvent(
          expectedEvent.GetPointer(), foo.GetPointer(),
          OtherEvent);

    if (!Check<vtkVariant>(
          __LINE__, "event-" + ToString<vtkIdType>(index),
          expectedEvent->GetValue(vtkEventSpy::EventCallData).ToString(),
          vtkVariant("(unknown)")))
      {
      return false;
      }

    if (!CheckEvent(__LINE__, "event-" + ToString<vtkIdType>(index),
                    spy->GetNthEvent(index), expectedEvent.GetPointer()))
      {
      return false;
      }
  }

  //
  // Test GetCountByEventId
  //

  if (!Check<vtkIdType>(
        __LINE__, "GetCountByEventId-ModifiedEvent",
        spy->GetCountByEventId(vtkCommand::ModifiedEvent), 2))
    {
    return false;
    }

  if (!Check<vtkIdType>(
        __LINE__, "GetCountByEventId-IntEvent",
        spy->GetCountByEventId(IntEvent), 1))
    {
    return false;
    }

  if (!Check<vtkIdType>(
        __LINE__, "GetCountByEventId-StringEvent",
        spy->GetCountByEventId(StringEvent), 1))
    {
    return false;
    }

  //
  // Test Reset
  //

  spy->ResetEvents();

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), 0))
    {
    return false;
    }

  return true;
}

namespace
{

enum CustomEventIds
{
  ObjectAddedEvent = vtkCommand::UserEvent
};

//----------------------------------------------------------------------------
class vtkCustomEventSpy : public vtkEventSpy
{
public:
  static vtkCustomEventSpy *New();
  typedef vtkCustomEventSpy Self;

  vtkTypeMacro(vtkCustomEventSpy, vtkEventSpy);

  enum
  {
    ModifiedTime = EventDefaultPropertyCount,
    ReferenceCount,
    // Add new property before this line
    CustomEventPropertyCount = ReferenceCount + 1
  };

protected:
  vtkCustomEventSpy()
  {
    this->SetCallDataType(ObjectAddedEvent, Self::VTKObject);
  }

  virtual void RecordEventProperties(vtkEventSpyEntry* event)
  {
    unsigned long eventId = vtkEventSpy::GetEventId(event);
    vtkObject* objectAdded =
        reinterpret_cast<vtkObject*>(vtkEventSpy::GetEventCallDataAsVoid(event));
    if (eventId == ObjectAddedEvent)
      {
      event->InsertValue(ModifiedTime, objectAdded->GetMTime());
      event->InsertValue(ReferenceCount, objectAdded->GetReferenceCount());
      }
  }

};
vtkStandardNewMacro(vtkCustomEventSpy);

}

//----------------------------------------------------------------------------
bool TestEventPropertyRecorderUsingDerivation()
{
  vtkNew<vtkCustomEventSpy> spy;

  vtkNew<vtkObject> foo;
  foo->AddObserver(ObjectAddedEvent, spy->GetSpy());

  vtkNew<vtkObject> objectToAdd;
  objectToAdd->Register(foo.GetPointer());

  foo->InvokeEvent(ObjectAddedEvent, objectToAdd.GetPointer());

  objectToAdd->UnRegister(foo.GetPointer());

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), 1))
    {
    return false;
    }

  vtkNew<vtkEventSpyEntry> expected;
  vtkEventSpy::UpdateEvent(
        expected.GetPointer(), foo.GetPointer(),
        ObjectAddedEvent, objectToAdd.GetPointer());
  expected->InsertValue(vtkCustomEventSpy::ModifiedTime,
                        vtkVariant(objectToAdd->GetMTime()));
  expected->InsertValue(vtkCustomEventSpy::ReferenceCount,
                        vtkVariant(2));

  if (!CheckEvent(__LINE__, "event-ObjectAddedEvent",
                  spy->GetNthEvent(0), expected.GetPointer()))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestEventPropertyRecorderUsingFunctor()
{
  vtkNew<vtkEventSpy> spy;

  struct ObjectAddedEventPropertyRecorder : vtkEventSpy::EventPropertyRecorder
  {
    enum
    {
      ModifiedTime = vtkEventSpy::EventDefaultPropertyCount,
      ReferenceCount,
    };

    virtual void operator () (vtkEventSpyEntry* event)
    {
      vtkObject* objectAdded =
          reinterpret_cast<vtkObject*>(vtkEventSpy::GetEventCallDataAsVoid(event));
      event->InsertValue(ModifiedTime, objectAdded->GetMTime());
      event->InsertValue(ReferenceCount, objectAdded->GetReferenceCount());
    }
  };

  ObjectAddedEventPropertyRecorder recorder;

  spy->SetCallDataType(ObjectAddedEvent, vtkEventSpy::VTKObject);
  spy->SetEventPropertyRecorder(ObjectAddedEvent, &recorder);

  vtkNew<vtkObject> foo;
  foo->AddObserver(ObjectAddedEvent, spy->GetSpy());

  vtkNew<vtkObject> objectToAdd;
  objectToAdd->Register(foo.GetPointer());

  foo->InvokeEvent(ObjectAddedEvent, objectToAdd.GetPointer());

  objectToAdd->UnRegister(foo.GetPointer());

  if (!Check<vtkIdType>(__LINE__, "GetCount", spy->GetCount(), 1))
    {
    return false;
    }

  vtkNew<vtkEventSpyEntry> expected;
  vtkEventSpy::UpdateEvent(
        expected.GetPointer(), foo.GetPointer(),
        ObjectAddedEvent, objectToAdd.GetPointer());
  expected->InsertValue(vtkCustomEventSpy::ModifiedTime,
                        vtkVariant(objectToAdd->GetMTime()));
  expected->InsertValue(vtkCustomEventSpy::ReferenceCount,
                        vtkVariant(2));

  if (!CheckEvent(__LINE__, "event-ObjectAddedEvent",
                  spy->GetNthEvent(0), expected.GetPointer()))
    {
    return false;
    }

  return true;
}
