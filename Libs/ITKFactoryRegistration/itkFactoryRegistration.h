

#ifndef __itkFactoryRegistration_h
#define __itkFactoryRegistration_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "itkFactoryRegistrationConfigure.h"

namespace itk {

ITKFactoryRegistration_EXPORT void itkFactoryRegistration(void);

}

extern "C" {
void register_itk_factory();
}

#endif

