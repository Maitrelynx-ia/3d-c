#ifndef CAD_STL_WRITER_H
#define CAD_STL_WRITER_H

#include "../core/geometry/brep.h"

#ifdef __cplusplus
extern "C" {
#endif

bool stl_export(const TopoShape* shape, const char* filename);
bool stl_export_ascii(const TopoShape* shape, const char* filename);
bool stl_export_binary(const TopoShape* shape, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_STL_WRITER_H
