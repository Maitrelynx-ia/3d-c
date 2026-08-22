#ifndef CAD_JSON_H
#define CAD_JSON_H

#include "../modeling/assembly/Assembly.hpp"

#ifdef __cplusplus
extern "C" {
#endif

bool cad_save_json(const Assembly* assembly, const char* filename);
Assembly* cad_load_json(const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_JSON_H
