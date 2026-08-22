#include "cad_json.h"
#include "../modeling/assembly/Assembly.hpp"
#include "../modeling/part/Part.hpp"
#include "../modeling/sketch/Sketch.hpp"
#include "cjson/cJSON.h"

static cJSON* vec3_to_json(Vec3 v) {
    cJSON* json = cJSON_CreateArray();
    cJSON_AddItemToArray(json, cJSON_CreateNumber(v.x));
    cJSON_AddItemToArray(json, cJSON_CreateNumber(v.y));
    cJSON_AddItemToArray(json, cJSON_CreateNumber(v.z));
    return json;
}

static Vec3 vec3_from_json(cJSON* json) {
    if (!json || !cJSON_IsArray(json) || cJSON_GetArraySize(json) < 3) {
        return VEC3_ZERO;
    }
    return (Vec3){
        (float)cJSON_GetArrayItem(json, 0)->valuedouble,
        (float)cJSON_GetArrayItem(json, 1)->valuedouble,
        (float)cJSON_GetArrayItem(json, 2)->valuedouble
    };
}

static cJSON* sketch_to_json(const Sketch* sketch) {
    if (!sketch) return NULL;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", sketch->getName().c_str());
    const Plane& plane = sketch->getPlane();
    cJSON_AddItemToObject(json, "plane_origin", vec3_to_json(plane.origin));
    cJSON_AddItemToObject(json, "plane_normal", vec3_to_json(plane.normal));
    cJSON* entities = cJSON_AddArrayToObject(json, "entities");
    for (const auto& entity : sketch->getEntities()) {
        cJSON* entity_json = cJSON_CreateObject();
        switch (entity->type) {
            case SketchEntityType::LINE: cJSON_AddStringToObject(entity_json, "type", "line"); break;
            case SketchEntityType::CIRCLE: cJSON_AddStringToObject(entity_json, "type", "circle"); break;
            case SketchEntityType::ARC: cJSON_AddStringToObject(entity_json, "type", "arc"); break;
            case SketchEntityType::POINT: cJSON_AddStringToObject(entity_json, "type", "point"); break;
            default: cJSON_AddStringToObject(entity_json, "type", "unknown"); break;
        }
        switch (entity->type) {
            case SketchEntityType::LINE: {
                SketchLine* line = static_cast<SketchLine*>(entity.get());
                cJSON_AddItemToObject(entity_json, "start", vec3_to_json(line->start));
                cJSON_AddItemToObject(entity_json, "end", vec3_to_json(line->end));
                break;
            }
            case SketchEntityType::CIRCLE: {
                SketchCircle* circle = static_cast<SketchCircle*>(entity.get());
                cJSON_AddItemToObject(entity_json, "center", vec3_to_json(circle->center));
                cJSON_AddNumberToObject(entity_json, "radius", circle->radius);
                break;
            }
            case SketchEntityType::ARC: {
                SketchArc* arc = static_cast<SketchArc*>(entity.get());
                cJSON_AddItemToObject(entity_json, "center", vec3_to_json(arc->center));
                cJSON_AddNumberToObject(entity_json, "radius", arc->radius);
                cJSON_AddNumberToObject(entity_json, "start_angle", arc->start_angle);
                cJSON_AddNumberToObject(entity_json, "end_angle", arc->end_angle);
                break;
            }
            case SketchEntityType::POINT: {
                SketchPoint* point = static_cast<SketchPoint*>(entity.get());
                cJSON_AddItemToObject(entity_json, "position", vec3_to_json(point->position));
                break;
            }
        }
        cJSON_AddItemToArray(entities, entity_json);
    }
    return json;
}

static Sketch* sketch_from_json(cJSON* json) {
    if (!json) return NULL;
    cJSON* name_json = cJSON_GetObjectItem(json, "name");
    std::string name = name_json ? name_json->valuestring : "Unnamed";
    cJSON* plane_origin_json = cJSON_GetObjectItem(json, "plane_origin");
    cJSON* plane_normal_json = cJSON_GetObjectItem(json, "plane_normal");
    Plane plane;
    if (plane_origin_json) plane.origin = vec3_from_json(plane_origin_json);
    if (plane_normal_json) plane.normal = vec3_from_json(plane_normal_json);
    else plane.normal = VEC3_Z;
    Sketch* sketch = new Sketch(name, plane);
    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    if (entities && cJSON_IsArray(entities)) {
        cJSON* entity_json = NULL;
        cJSON_ArrayForEach(entity_json, entities) {
            cJSON* type_json = cJSON_GetObjectItem(entity_json, "type");
            if (!type_json) continue;
            std::string type = type_json->valuestring;
            if (type == "line") {
                Vec3 start = vec3_from_json(cJSON_GetObjectItem(entity_json, "start"));
                Vec3 end = vec3_from_json(cJSON_GetObjectItem(entity_json, "end"));
                sketch->addLine(start, end);
            } else if (type == "circle") {
                Vec3 center = vec3_from_json(cJSON_GetObjectItem(entity_json, "center"));
                float radius = cJSON_GetObjectItem(entity_json, "radius")->valuedouble;
                sketch->addCircle(center, radius);
            } else if (type == "arc") {
                Vec3 center = vec3_from_json(cJSON_GetObjectItem(entity_json, "center"));
                float radius = cJSON_GetObjectItem(entity_json, "radius")->valuedouble;
                float start_angle = cJSON_GetObjectItem(entity_json, "start_angle")->valuedouble;
                float end_angle = cJSON_GetObjectItem(entity_json, "end_angle")->valuedouble;
                sketch->addArc(center, radius, start_angle, end_angle);
            } else if (type == "point") {
                Vec3 position = vec3_from_json(cJSON_GetObjectItem(entity_json, "position"));
                sketch->addPoint(position);
            }
        }
    }
    return sketch;
}

static cJSON* part_to_json(const Part* part) {
    if (!part) return NULL;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", part->getName().c_str());
    const Material& material = part->getMaterial();
    cJSON* material_json = cJSON_AddObjectToObject(json, "material");
    cJSON_AddStringToObject(material_json, "name", material.name.c_str());
    cJSON_AddNumberToObject(material_json, "density", material.density);
    cJSON_AddNumberToObject(material_json, "young_modulus", material.young_modulus);
    cJSON_AddNumberToObject(material_json, "poisson_ratio", material.poisson_ratio);
    cJSON* sketches = cJSON_AddArrayToObject(json, "sketches");
    for (const auto& sketch : part->getSketches()) {
        cJSON_AddItemToArray(sketches, sketch_to_json(sketch.get()));
    }
    cJSON* features = cJSON_AddArrayToObject(json, "features");
    for (const auto& feature : part->getFeatures()) {
        cJSON* feature_json = cJSON_CreateObject();
        switch (feature->type) {
            case FeatureType::EXTRUDE: cJSON_AddStringToObject(feature_json, "type", "extrude"); break;
            case FeatureType::REVOLVE: cJSON_AddStringToObject(feature_json, "type", "revolve"); break;
            case FeatureType::FILLET: cJSON_AddStringToObject(feature_json, "type", "fillet"); break;
            case FeatureType::CHAMFER: cJSON_AddStringToObject(feature_json, "type", "chamfer"); break;
            case FeatureType::HOLE: cJSON_AddStringToObject(feature_json, "type", "hole"); break;
            default: cJSON_AddStringToObject(feature_json, "type", "unknown"); break;
        }
        cJSON* params = cJSON_AddArrayToObject(feature_json, "parameters");
        for (const auto& param : feature->parameters) {
            cJSON* param_json = cJSON_CreateObject();
            cJSON_AddStringToObject(param_json, "name", param.name.c_str());
            switch (param.type) {
                case Parameter::FLOAT: cJSON_AddNumberToObject(param_json, "value", param.float_value); break;
                case Parameter::INT: cJSON_AddNumberToObject(param_json, "value", param.int_value); break;
                case Parameter::BOOL: cJSON_AddBoolToObject(param_json, "value", param.bool_value); break;
            }
            cJSON_AddItemToArray(params, param_json);
        }
        cJSON_AddItemToArray(features, feature_json);
    }
    return json;
}

static Part* part_from_json(cJSON* json) {
    if (!json) return NULL;
    cJSON* name_json = cJSON_GetObjectItem(json, "name");
    std::string name = name_json ? name_json->valuestring : "Unnamed";
    Part* part = new Part(name);
    cJSON* material_json = cJSON_GetObjectItem(json, "material");
    if (material_json) {
        Material material;
        cJSON* name = cJSON_GetObjectItem(material_json, "name");
        if (name) material.name = name->valuestring;
        cJSON* density = cJSON_GetObjectItem(material_json, "density");
        if (density) material.density = density->valuedouble;
        cJSON* young = cJSON_GetObjectItem(material_json, "young_modulus");
        if (young) material.young_modulus = young->valuedouble;
        cJSON* poisson = cJSON_GetObjectItem(material_json, "poisson_ratio");
        if (poisson) material.poisson_ratio = poisson->valuedouble;
        part->setMaterial(material);
    }
    cJSON* sketches = cJSON_GetObjectItem(json, "sketches");
    if (sketches && cJSON_IsArray(sketches)) {
        cJSON* sketch_json = NULL;
        cJSON_ArrayForEach(sketch_json, sketches) {
            Sketch* sketch = sketch_from_json(sketch_json);
            if (sketch) part->getSketches().push_back(std::unique_ptr<Sketch>(sketch));
        }
    }
    cJSON* features = cJSON_GetObjectItem(json, "features");
    if (features && cJSON_IsArray(features)) {
        cJSON* feature_json = NULL;
        cJSON_ArrayForEach(feature_json, features) {
            cJSON* type = cJSON_GetObjectItem(feature_json, "type");
            if (!type) continue;
            std::string type_str = type->valuestring;
            if (type_str == "extrude") {
                cJSON* params = cJSON_GetObjectItem(feature_json, "parameters");
                float depth = 10.0f;
                bool is_additive = true;
                if (params && cJSON_IsArray(params)) {
                    cJSON* param = NULL;
                    cJSON_ArrayForEach(param, params) {
                        cJSON* name = cJSON_GetObjectItem(param, "name");
                        if (!name) continue;
                        if (name->valuestring == std::string("Depth")) {
                            depth = cJSON_GetObjectItem(param, "value")->valuedouble;
                        } else if (name->valuestring == std::string("Additive")) {
                            is_additive = cJSON_GetObjectItem(param, "value")->valueint;
                        }
                    }
                }
                Sketch* sketch = part->createSketch(SketchPlane::XY);
                part->addExtrude(sketch, depth, is_additive);
            }
        }
    }
    return part;
}

bool cad_save_json(const Assembly* assembly, const char* filename) {
    if (!assembly) return false;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "version", "1.0");
    cJSON_AddStringToObject(json, "name", assembly->getName().c_str());
    cJSON* parts = cJSON_AddArrayToObject(json, "parts");
    for (Part* part : assembly->getParts()) {
        cJSON_AddItemToArray(parts, part_to_json(part));
    }
    cJSON* constraints = cJSON_AddArrayToObject(json, "constraints");
    for (const auto& constraint : assembly->getConstraints()) {
        cJSON* constraint_json = cJSON_CreateObject();
        switch (constraint->type) {
            case AssemblyConstraintType::COINCIDENT: cJSON_AddStringToObject(constraint_json, "type", "coincident"); break;
            case AssemblyConstraintType::PARALLEL: cJSON_AddStringToObject(constraint_json, "type", "parallel"); break;
            case AssemblyConstraintType::PERPENDICULAR: cJSON_AddStringToObject(constraint_json, "type", "perpendicular"); break;
            case AssemblyConstraintType::DISTANCE: cJSON_AddStringToObject(constraint_json, "type", "distance"); break;
            case AssemblyConstraintType::ANGLE: cJSON_AddStringToObject(constraint_json, "type", "angle"); break;
            case AssemblyConstraintType::INSERT: cJSON_AddStringToObject(constraint_json, "type", "insert"); break;
        }
        cJSON_AddNumberToObject(constraint_json, "value", constraint->value);
        cJSON_AddItemToArray(constraints, constraint_json);
    }
    char* json_str = cJSON_Print(json);
    FILE* file = fopen(filename, "w");
    if (!file) {
        cJSON_free(json_str);
        cJSON_Delete(json);
        return false;
    }
    fwrite(json_str, 1, strlen(json_str), file);
    fclose(file);
    cJSON_free(json_str);
    cJSON_Delete(json);
    return true;
}

Assembly* cad_load_json(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) { fclose(file); return NULL; }
    size_t read = fread(buffer, 1, file_size, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON* json = cJSON_Parse(buffer);
    free(buffer);
    if (!json) return NULL;
    cJSON* version = cJSON_GetObjectItem(json, "version");
    if (!version || strcmp(version->valuestring, "1.0") != 0) {
        cJSON_Delete(json);
        return NULL;
    }
    cJSON* name = cJSON_GetObjectItem(json, "name");
    std::string assembly_name = name ? name->valuestring : "Unnamed";
    Assembly* assembly = new Assembly(assembly_name);
    cJSON* parts = cJSON_GetObjectItem(json, "parts");
    if (parts && cJSON_IsArray(parts)) {
        cJSON* part_json = NULL;
        cJSON_ArrayForEach(part_json, parts) {
            Part* part = part_from_json(part_json);
            if (part) assembly->addPart(part);
        }
    }
    cJSON* constraints = cJSON_GetObjectItem(json, "constraints");
    if (constraints && cJSON_IsArray(constraints)) {
        cJSON* constraint_json = NULL;
        cJSON_ArrayForEach(constraint_json, constraints) {
        }
    }
    cJSON_Delete(json);
    return assembly;
}