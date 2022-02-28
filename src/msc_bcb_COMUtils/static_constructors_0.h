#pragma once
#include "utils/static_constructors_0.h"

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#define __CONDITION_CALL(a)   __if_exists(a) {a() ;}
#else 
#define __CONDITION_CALL(a) a()
#endif




