#ifndef CAD_OBJ_READER_H
#define CAD_OBJ_READER_H

#include "../core/geometry/brep.h"

#ifdef __cplusplus
extern "C" {
#endif

TopoShape* obj_import(const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_OBJ_READER_H
