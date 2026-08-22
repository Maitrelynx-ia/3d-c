#ifndef CAD_STL_READER_H
#define CAD_STL_READER_H

#include "../core/geometry/brep.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- STL Triangle (binaire) ---
#pragma pack(push, 1)
typedef struct {
    float normal[3];
    float vertex1[3];
    float vertex2[3];
    float vertex3[3];
    uint16_t attribute_byte_count;
} STLTriangle;
#pragma pack(pop)

// --- STL Header (80 octets) ---
typedef struct {
    uint8_t header[80];
    uint32_t num_triangles;
} STLHeader;

// --- Lecture STL ---
TopoShape* stl_import(const char* filename);
TopoShape* stl_import_binary(const char* filename);
TopoShape* stl_import_ascii(const char* filename);

// --- Verification du format ---
bool stl_is_binary(const char* filename);
bool stl_is_ascii(const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_STL_READER_H
