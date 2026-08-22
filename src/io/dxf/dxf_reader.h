#ifndef CAD_DXF_READER_H
#define CAD_DXF_READER_H

#include "../modeling/sketch/Sketch.hpp"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

Sketch* dxf_import(const char* filename);
bool dxf_import_to_sketch(const char* filename, Sketch* sketch);

#ifdef __cplusplus
}
#endif

#endif // CAD_DXF_READER_H
