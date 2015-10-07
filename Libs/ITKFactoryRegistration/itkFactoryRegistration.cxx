
#include "itkFactoryRegistration.h"

// ITK includes
#include <itkImageFileReader.h>
#include <itkTransformFileReader.h>

// The following code is required to ensure that the
// mechanism allowing the ITK factory to be registered is not
// optimized out by the compiler.
void itk::itkFactoryRegistration(void)
{
  return;
}

void register_itk_factory()
{
  std::cout << "ITK Factory registered" << std::endl;
  itk::itkFactoryRegistration();
}

