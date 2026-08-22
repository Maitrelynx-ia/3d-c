#ifndef CAD_DXF_WRITER_H
#define CAD_DXF_WRITER_H

#include "../modeling/sketch/Sketch.hpp"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool dxf_export(const Sketch* sketch, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_DXF_WRITER_H
