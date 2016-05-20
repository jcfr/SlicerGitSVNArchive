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

==============================================================================*/

// vtkAddon includes
#include "vtkAddonMathUtilities.h"
#include "vtkAddonTestingUtilities.h"

// vtk includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

using namespace vtkAddonTestingUtilities;

//----------------------------------------------------------------------------
int vtkAddonMathUtilitiesTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{

  vtkSmartPointer<vtkMatrix4x4> m1 =
      vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> m2 =
      vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> m3 =
      vtkSmartPointer<vtkMatrix4x4>::New();
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {
      m1->SetElement(i, j, i * j);
      m2->SetElement(i, j, i * j);
      m3->SetElement(i, j, i);
      }
    }

  if (!CheckInt(__LINE__, "TestCheckInt", 1, vtkAddonMathUtilities::Matrix4x4AreEqual(m1, m2))
      || CheckInt(__LINE__, "TestCheckInt Expected Failure", 1, vtkAddonMathUtilities::Matrix4x4AreEqual(m1, m3)))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckInt failed" << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
