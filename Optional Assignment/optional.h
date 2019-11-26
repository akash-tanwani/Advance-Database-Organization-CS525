#ifndef OPTIONAL_H
#define OPTIONAL_H

#include "dberror.h"

extern RC initiateOptional (int numPages);
extern RC shutOptional (void);
extern long getOptionalInputOutput (void);

#endif
