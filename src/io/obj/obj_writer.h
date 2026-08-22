#ifndef CAD_OBJ_WRITER_H
#define CAD_OBJ_WRITER_H

#include "../core/geometry/brep.h"

#ifdef __cplusplus
extern "C" {
#endif

bool obj_export(const TopoShape* shape, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_OBJ_WRITER_H
