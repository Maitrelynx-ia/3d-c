#ifndef CAD_BINARY_H
#define CAD_BINARY_H

#include "../modeling/assembly/Assembly.hpp"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAD_MAGIC 0x43414400
#define CAD_VERSION 1

bool cad_save_binary(const Assembly* assembly, const char* filename);
Assembly* cad_load_binary(const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_BINARY_H
