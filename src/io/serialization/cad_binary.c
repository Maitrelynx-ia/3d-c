#include "cad_binary.h"
#include "../modeling/assembly/Assembly.hpp"
#include "../modeling/part/Part.hpp"
#include "../modeling/sketch/Sketch.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t timestamp;
    uint32_t num_parts;
    uint32_t num_constraints;
} CADHeader;
#pragma pack(pop)

static void write_vec3(FILE* file, Vec3 v) {
    fwrite(&v.x, sizeof(float), 1, file);
    fwrite(&v.y, sizeof(float), 1, file);
    fwrite(&v.z, sizeof(float), 1, file);
}

static Vec3 read_vec3(FILE* file) {
    Vec3 v;
    fread(&v.x, sizeof(float), 1, file);
    fread(&v.y, sizeof(float), 1, file);
    fread(&v.z, sizeof(float), 1, file);
    return v;
}

static void write_string(FILE* file, const char* str) {
    uint32_t len = str ? strlen(str) : 0;
    fwrite(&len, sizeof(uint32_t), 1, file);
    if (len > 0) fwrite(str, sizeof(char), len, file);
}

static char* read_string(FILE* file) {
    uint32_t len;
    fread(&len, sizeof(uint32_t), 1, file);
    if (len == 0) return NULL;
    char* str = (char*)malloc(len + 1);
    fread(str, sizeof(char), len, file);
    str[len] = '\0';
    return str;
}

static bool write_sketch(FILE* file, const Sketch* sketch) {
    if (!sketch) return false;
    write_string(file, sketch->getName().c_str());
    const Plane& plane = sketch->getPlane();
    write_vec3(file, plane.origin);
    write_vec3(file, plane.normal);
    uint32_t num_entities = sketch->getEntities().size();
    fwrite(&num_entities, sizeof(uint32_t), 1, file);
    for (const auto& entity : sketch->getEntities()) {
        uint32_t type = (uint32_t)entity->type;
        fwrite(&type, sizeof(uint32_t), 1, file);
        switch (entity->type) {
            case SketchEntityType::LINE: {
                SketchLine* line = static_cast<SketchLine*>(entity.get());
                write_vec3(file, line->start);
                write_vec3(file, line->end);
                break;
            }
            case SketchEntityType::CIRCLE: {
                SketchCircle* circle = static_cast<SketchCircle*>(entity.get());
                write_vec3(file, circle->center);
                fwrite(&circle->radius, sizeof(float), 1, file);
                break;
            }
            case SketchEntityType::ARC: {
                SketchArc* arc = static_cast<SketchArc*>(entity.get());
                write_vec3(file, arc->center);
                fwrite(&arc->radius, sizeof(float), 1, file);
                fwrite(&arc->start_angle, sizeof(float), 1, file);
                fwrite(&arc->end_angle, sizeof(float), 1, file);
                break;
            }
            case SketchEntityType::POINT: {
                SketchPoint* point = static_cast<SketchPoint*>(entity.get());
                write_vec3(file, point->position);
                break;
            }
            default: return false;
        }
    }
    return true;
}

static Sketch* read_sketch(FILE* file) {
    char* name = read_string(file);
    std::string name_str = name ? name : "Unnamed";
    if (name) free(name);
    Vec3 origin = read_vec3(file);
    Vec3 normal = read_vec3(file);
    Plane plane(origin, normal);
    Sketch* sketch = new Sketch(name_str, plane);
    uint32_t num_entities;
    fread(&num_entities, sizeof(uint32_t), 1, file);
    for (uint32_t i = 0; i < num_entities; i++) {
        uint32_t type;
        fread(&type, sizeof(uint32_t), 1, file);
        switch ((SketchEntityType)type) {
            case SketchEntityType::LINE: {
                Vec3 start = read_vec3(file);
                Vec3 end = read_vec3(file);
                sketch->addLine(start, end);
                break;
            }
            case SketchEntityType::CIRCLE: {
                Vec3 center = read_vec3(file);
                float radius;
                fread(&radius, sizeof(float), 1, file);
                sketch->addCircle(center, radius);
                break;
            }
            case SketchEntityType::ARC: {
                Vec3 center = read_vec3(file);
                float radius, start_angle, end_angle;
                fread(&radius, sizeof(float), 1, file);
                fread(&start_angle, sizeof(float), 1, file);
                fread(&end_angle, sizeof(float), 1, file);
                sketch->addArc(center, radius, start_angle, end_angle);
                break;
            }
            case SketchEntityType::POINT: {
                Vec3 position = read_vec3(file);
                sketch->addPoint(position);
                break;
            }
            default: return NULL;
        }
    }
    return sketch;
}

static bool write_part(FILE* file, const Part* part) {
    if (!part) return false;
    write_string(file, part->getName().c_str());
    const Material& material = part->getMaterial();
    write_string(file, material.name.c_str());
    fwrite(&material.density, sizeof(float), 1, file);
    fwrite(&material.young_modulus, sizeof(float), 1, file);
    fwrite(&material.poisson_ratio, sizeof(float), 1, file);
    uint32_t num_sketches = part->getSketches().size();
    fwrite(&num_sketches, sizeof(uint32_t), 1, file);
    for (const auto& sketch : part->getSketches()) {
        if (!write_sketch(file, sketch.get())) return false;
    }
    uint32_t num_features = part->getFeatures().size();
    fwrite(&num_features, sizeof(uint32_t), 1, file);
    for (const auto& feature : part->getFeatures()) {
        uint32_t type = (uint32_t)feature->type;
        fwrite(&type, sizeof(uint32_t), 1, file);
        uint32_t num_params = feature->parameters.size();
        fwrite(&num_params, sizeof(uint32_t), 1, file);
        for (const auto& param : feature->parameters) {
            write_string(file, param.name.c_str());
            uint32_t param_type = (uint32_t)param.type;
            fwrite(&param_type, sizeof(uint32_t), 1, file);
            switch (param.type) {
                case Parameter::FLOAT: fwrite(&param.float_value, sizeof(float), 1, file); break;
                case Parameter::INT: fwrite(&param.int_value, sizeof(int), 1, file); break;
                case Parameter::BOOL: fwrite(&param.bool_value, sizeof(bool), 1, file); break;
            }
        }
    }
    return true;
}

static Part* read_part(FILE* file) {
    char* name = read_string(file);
    std::string name_str = name ? name : "Unnamed";
    if (name) free(name);
    Part* part = new Part(name_str);
    Material material;
    char* material_name = read_string(file);
    if (material_name) { material.name = material_name; free(material_name); }
    fread(&material.density, sizeof(float), 1, file);
    fread(&material.young_modulus, sizeof(float), 1, file);
    fread(&material.poisson_ratio, sizeof(float), 1, file);
    part->setMaterial(material);
    uint32_t num_sketches;
    fread(&num_sketches, sizeof(uint32_t), 1, file);
    for (uint32_t i = 0; i < num_sketches; i++) {
        Sketch* sketch = read_sketch(file);
        if (sketch) part->getSketches().push_back(std::unique_ptr<Sketch>(sketch));
    }
    uint32_t num_features;
    fread(&num_features, sizeof(uint32_t), 1, file);
    for (uint32_t i = 0; i < num_features; i++) {
        uint32_t type;
        fread(&type, sizeof(uint32_t), 1, file);
        uint32_t num_params;
        fread(&num_params, sizeof(uint32_t), 1, file);
        if ((FeatureType)type == FeatureType::EXTRUDE && num_params > 0) {
            float depth = 10.0f;
            bool is_additive = true;
            for (uint32_t j = 0; j < num_params; j++) {
                char* param_name = read_string(file);
                if (param_name) {
                    uint32_t param_type;
                    fread(&param_type, sizeof(uint32_t), 1, file);
                    if (strcmp(param_name, "Depth") == 0) {
                        fread(&depth, sizeof(float), 1, file);
                    } else if (strcmp(param_name, "Additive") == 0) {
                        fread(&is_additive, sizeof(bool), 1, file);
                    } else {
                        if ((Parameter::Type)param_type == Parameter::FLOAT) {
                            float dummy; fread(&dummy, sizeof(float), 1, file);
                        } else if ((Parameter::Type)param_type == Parameter::INT) {
                            int dummy; fread(&dummy, sizeof(int), 1, file);
                        } else if ((Parameter::Type)param_type == Parameter::BOOL) {
                            bool dummy; fread(&dummy, sizeof(bool), 1, file);
                        }
                    }
                    free(param_name);
                }
            }
            Sketch* sketch = part->createSketch(SketchPlane::XY);
            part->addExtrude(sketch, depth, is_additive);
        } else {
            for (uint32_t j = 0; j < num_params; j++) {
                char* param_name = read_string(file);
                if (param_name) free(param_name);
                uint32_t param_type;
                fread(&param_type, sizeof(uint32_t), 1, file);
                if ((Parameter::Type)param_type == Parameter::FLOAT) {
                    float dummy; fread(&dummy, sizeof(float), 1, file);
                } else if ((Parameter::Type)param_type == Parameter::INT) {
                    int dummy; fread(&dummy, sizeof(int), 1, file);
                } else if ((Parameter::Type)param_type == Parameter::BOOL) {
                    bool dummy; fread(&dummy, sizeof(bool), 1, file);
                }
            }
        }
    }
    return part;
}

bool cad_save_binary(const Assembly* assembly, const char* filename) {
    if (!assembly) return false;
    FILE* file = fopen(filename, "wb");
    if (!file) return false;
    CADHeader header;
    header.magic = CAD_MAGIC;
    header.version = CAD_VERSION;
    header.timestamp = 0;
    header.num_parts = assembly->getParts().size();
    header.num_constraints = assembly->getConstraints().size();
    fwrite(&header, sizeof(CADHeader), 1, file);
    write_string(file, assembly->getName().c_str());
    for (Part* part : assembly->getParts()) {
        if (!write_part(file, part)) { fclose(file); return false; }
    }
    for (const auto& constraint : assembly->getConstraints()) {
        uint32_t type = (uint32_t)constraint->type;
        fwrite(&type, sizeof(uint32_t), 1, file);
        fwrite(&constraint->value, sizeof(float), 1, file);
    }
    fclose(file);
    return true;
}

Assembly* cad_load_binary(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    CADHeader header;
    size_t read = fread(&header, sizeof(CADHeader), 1, file);
    if (read != 1 || header.magic != CAD_MAGIC) { fclose(file); return NULL; }
    if (header.version != CAD_VERSION) { fclose(file); return NULL; }
    char* name = read_string(file);
    std::string assembly_name = name ? name : "Unnamed";
    if (name) free(name);
    Assembly* assembly = new Assembly(assembly_name);
    for (uint32_t i = 0; i < header.num_parts; i++) {
        Part* part = read_part(file);
        if (part) assembly->addPart(part);
    }
    for (uint32_t i = 0; i < header.num_constraints; i++) {
        uint32_t type;
        fread(&type, sizeof(uint32_t), 1, file);
        float value;
        fread(&value, sizeof(float), 1, file);
    }
    fclose(file);
    return assembly;
}