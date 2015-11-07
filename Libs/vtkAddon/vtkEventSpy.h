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

#ifndef vtkEventSpy_h
#define vtkEventSpy_h

#include "vtkAddon.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkVariantArray.h>
typedef vtkVariantArray vtkEventSpyEntry;

/// The vtkEventSpy class enables introspection of event invocation.
///
/// vtkEventSpy can observe any event of any object and records its invocation.
/// vtkEventSpy itself is an array of vtkVariant arrays. Each emission of the
/// event will append one item to the array, containing the \a eventId,
/// \a caller and \a callData.
///
/// \sa vtkCommand
/// \sa vtkObject::AddObserver
///

//VTK_ADDON_EXPORT ostream& operator << (ostream& os, vtkEventSpyEntry* event);

class VTK_ADDON_EXPORT vtkEventSpy : public vtkObject
{
public:
  static vtkEventSpy *New();
  typedef vtkEventSpy Self;

  vtkTypeMacro(vtkEventSpy, vtkObject);

  enum EventProperties
  {
    EventCaller = 0,
    EventId,
    EventCallData,
    EventDefaultPropertyCount = EventCallData + 1
  };

  enum EventCallDataTypes
  {
    Unknown = 0,
    String,
    Integer,
    VTKObject
  };

  /// Return \a true if \a callDataType can be converted to a vtkVariant.
  ///
  /// \sa SetCallDataType()
  static bool IsSupportedCallDataType(int callDataType);

  /// Associate an \a eventId with \a callDataType.
  ///
  /// \sa GetCallDataType(), IsSupportedCallDataType()
  void SetCallDataType(unsigned long eventId, int callDataType);

  /// Get \a callDataType associated with \a eventId.
  ///
  /// \sa SetCallDataType()
  int GetCallDataType(unsigned long eventId);

  /// Remove all associations of \a eventId with \a callDataType.
  ///
  /// \sa SetCallDataType(), GetCallDataType()
  void ResetCallDataTypes();

  /// Base functor for recording additional event properties.
  ///
  /// \sa SetEventPropertyRecorder(), GetEventPropertyRecorder()
  struct EventPropertyRecorder
  {
    virtual void operator () (vtkEventSpyEntry* event) = 0;
    virtual ~EventPropertyRecorder() {}
  };

  /// Associate an \a eventId with an instance of \a recorder functor.
  ///
  /// \sa GetEventPropertyRecorder()
  void SetEventPropertyRecorder(unsigned long eventId,
                                EventPropertyRecorder* recorder);

  /// Get the recorder functor associated with an \a eventId.
  ///
  /// \sa SetEventPropertyRecorder()
  EventPropertyRecorder* GetEventPropertyRecorder(unsigned long eventId);

  /// Remove all associations of \a eventId with \a recorder.
  ///
  /// \sa SetEventPropertyRecorder(), GetEventPropertyRecorder()
  void ResetEventPropertyRecorders();

  /// Get the spy command.
  ///
  /// This is the callback command that should be associated
  /// with the object to spy.
  vtkCommand* GetSpy();

  /// Get the number of collected events.
  vtkIdType GetCount();

  ///\copydoc GetCount()
  ///
  /// \deprecated Use GetCount() instead.
  vtkIdType GetTotalNumberOfEvents();

  /// Get the number of collected events for a given \a eventId.
  vtkIdType GetCountByEventId(unsigned long eventId);

  /// \copybrief GetCountByEventId()
  ///
  /// \deprecated Use GetCountByEventId() instead.
  vtkIdType GetNumberOfEvents(unsigned long eventId);

  /// Get Nth event if any.
  vtkEventSpyEntry* GetNthEvent(vtkIdType index);

  /// Clear all collected events.
  virtual void ResetEvents();

  /// \copybrief ResetEvents()
  ///
  /// \deprecated Use ResetEvents() instead.
  void ResetNumberOfEvents();

  /// Get \a caller stored in \a event.
  static vtkObject* GetEventCaller(vtkEventSpyEntry* event);

  /// Get eventId stored in \a event.
  static unsigned long GetEventId(vtkEventSpyEntry* event);

  /// Get \a callData stored in \a event.
  ///
  /// \a callData is a string of the form "(vtkClassName)0x12345"
  static std::string GetEventCallData(vtkEventSpyEntry* event);

  /// Get \a callData stored in \a event as a pointer.
  ///
  /// \sa GetEventCallData()
  static void* GetEventCallDataAsVoid(vtkEventSpyEntry* event);

  /// Convenience to set \a event properties.
  static void UpdateEvent(vtkEventSpyEntry* event,
                          vtkObject* caller,
                          unsigned long eventId);
  static void UpdateEvent(vtkEventSpyEntry* event,
                          vtkObject* caller,
                          unsigned long eventId,
                          const char* callData);
  static void UpdateEvent(vtkEventSpyEntry* event,
                          vtkObject* caller,
                          unsigned long eventId,
                          const int* callData);
  static void UpdateEvent(vtkEventSpyEntry* event,
                          vtkObject* caller,
                          unsigned long eventId,
                          const vtkObjectBase* callData);

  enum AreEventEqualOptions
  {
    Default = 0x0,
    Quiet = 0x1,
    IgnoreCustomProperties = 0x2
  };

  /// \brief Return \a true if \a eventId, \a caller, \a callData and
  /// \a callDataType match.
  ///
  /// Options:
  ///
  /// * IgnoreCustomProperties: By default, the function will check if
  /// \a all custom properties match. Setting this option changes this.
  /// In that case, events are only expected to have at least 3 values
  /// (EventCaller, EventId, and EventCallData).
  ///
  static bool AreEventEqual(vtkEventSpyEntry* event1,
                            vtkEventSpyEntry* event2,
                            int options = Quiet);

  /// \copybrief AreEventEqual(vtkEventSpyEntry*, vtkEventSpyEntry*, int options)
  ///
  /// Specifying \a descriptionEvent1 and \a descriptionEvent2 will allow to
  /// customize the name of event displayed in error message.
  ///
  /// Options:
  ///
  /// * Quiet: By default, the function will display information. Setting this
  /// option changes this.
  ///
  /// * IgnoreCustomProperties: By default, the function will check if
  /// \a all custom properties match. Setting this option changes this.
  /// In that case, events are only expected to have at least 3 values
  /// (EventCaller, EventId, and EventCallData).
  ///
  static bool AreEventEqual(vtkEventSpyEntry* event1,
                            vtkEventSpyEntry* event2,
                            const std::string& descriptionEvent1,
                            const std::string& descriptionEvent2,
                            int options = Default);

  static std::string ToString(vtkEventSpyEntry* event);

  //friend VTK_ADDON_EXPORT ostream& operator << (ostream& os, vtkEventSpyEntry* event);

protected:

  vtkEventSpy();
  ~vtkEventSpy();

  virtual void RecordEventProperties(vtkEventSpyEntry* event);

private:

  class vtkInternal;
  friend class vtkInternal;
  vtkInternal* Internal;
};

#endif
