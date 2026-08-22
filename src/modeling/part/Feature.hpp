#ifndef CAD_FEATURE_HPP
#define CAD_FEATURE_HPP

#include "Sketch.hpp"
#include "../core/geometry/brep.h"
#include <vector>
#include <memory>
#include <string>

class Part;

enum class FeatureType {
    EXTRUDE,
    REVOLVE,
    LOFT,
    SWEEP,
    HOLE,
    FILLET,
    CHAMFER,
    POCKET,
    BOOLEAN_UNION,
    BOOLEAN_DIFFERENCE,
    BOOLEAN_INTERSECTION,
    SHELL,
    THICKEN,
    MIRROR,
    PATTERN_LINEAR,
    PATTERN_CIRCULAR
};

class Parameter {
public:
    std::string name;
    enum Type { FLOAT, INT, BOOL, STRING, VEC3 } type;
    union {
        float float_value;
        int int_value;
        bool bool_value;
        char* string_value;
        Vec3 vec3_value;
    };
    Parameter(const std::string& name, float value) : name(name), type(FLOAT) { float_value = value; }
    Parameter(const std::string& name, int value) : name(name), type(INT) { int_value = value; }
    Parameter(const std::string& name, bool value) : name(name), type(BOOL) { bool_value = value; }
    Parameter(const std::string& name, const std::string& value) : name(name), type(STRING) { 
        string_value = new char[value.size() + 1]; strcpy(string_value, value.c_str());
    }
    Parameter(const std::string& name, Vec3 value) : name(name), type(VEC3) { vec3_value = value; }
    ~Parameter() { if (type == STRING) delete[] string_value; }
    std::unique_ptr<Parameter> clone() const;
};

class Feature {
public:
    FeatureType type;
    std::string name;
    bool is_suppressed;
    bool is_visible;
    std::vector<Parameter> parameters;
    TopoShape* shape;
    Feature* parent;
    Feature(FeatureType type, const std::string& name = "")
        : type(type), name(name), is_suppressed(false), is_visible(true), shape(nullptr), parent(nullptr) {}
    virtual ~Feature() { if (shape) topo_shape_free(shape); }
    virtual TopoShape* compute() = 0;
    virtual void regenerate() = 0;
    virtual void translate(const Vec3& delta) = 0;
    virtual void rotate(const Vec3& axis, float angle) = 0;
    virtual void scale(const Vec3& scale) = 0;
    virtual std::unique_ptr<Feature> clone() const = 0;
    virtual void serialize(cJSON* json) const = 0;
    static std::unique_ptr<Feature> deserialize(cJSON* json);
};

class ExtrudeFeature : public Feature {
private:
    Sketch* sketch;
    float depth;
    bool is_additive;
public:
    ExtrudeFeature(Sketch* sketch, float depth, bool is_additive = true)
        : Feature(FeatureType::EXTRUDE, "Extrusion"), sketch(sketch), depth(depth), is_additive(is_additive) {
        parameters.push_back(Parameter("Depth", depth));
        parameters.push_back(Parameter("Additive", is_additive));
    }
    TopoShape* compute() override {
        if (shape) topo_shape_free(shape);
        shape = topo_create_cube(VEC3_ZERO, 10.0f);
        return shape;
    }
    void regenerate() override { compute(); }
    void translate(const Vec3& delta) override { if (shape) shape = topo_shape_transform(shape, mat4_translation(delta)); }
    void rotate(const Vec3& axis, float angle) override { if (shape) shape = topo_shape_transform(shape, mat4_rotation_axis(axis, angle)); }
    void scale(const Vec3& scale) override { if (shape) shape = topo_shape_transform(shape, mat4_scale(scale)); }
    Sketch* getSketch() const { return sketch; }
    float getDepth() const { return depth; }
    bool getIsAdditive() const { return is_additive; }
    std::unique_ptr<Feature> clone() const override { return std::make_unique<ExtrudeFeature>(sketch, depth, is_additive); }
    void serialize(cJSON* json) const override;
};

class RevolveFeature : public Feature {
private:
    Sketch* sketch;
    float angle;
    Vec3 axis;
public:
    RevolveFeature(Sketch* sketch, float angle, const Vec3& axis)
        : Feature(FeatureType::REVOLVE, "Revolution"), sketch(sketch), angle(angle), axis(axis) {
        parameters.push_back(Parameter("Angle", angle));
        parameters.push_back(Parameter("Axis", axis));
    }
    TopoShape* compute() override {
        if (shape) topo_shape_free(shape);
        shape = topo_create_cylinder(VEC3_ZERO, VEC3_Z, 5.0f, 10.0f);
        return shape;
    }
    void regenerate() override { compute(); }
    void translate(const Vec3& delta) override { if (shape) shape = topo_shape_transform(shape, mat4_translation(delta)); }
    void rotate(const Vec3& axis, float angle) override { if (shape) shape = topo_shape_transform(shape, mat4_rotation_axis(axis, angle)); }
    void scale(const Vec3& scale) override { if (shape) shape = topo_shape_transform(shape, mat4_scale(scale)); }
    std::unique_ptr<Feature> clone() const override { return std::make_unique<RevolveFeature>(sketch, angle, axis); }
    void serialize(cJSON* json) const override;
};

class FilletFeature : public Feature {
private:
    std::vector<TopoShape*> edges;
    float radius;
public:
    FilletFeature(const std::vector<TopoShape*>& edges, float radius)
        : Feature(FeatureType::FILLET, "Fillet"), edges(edges), radius(radius) {
        parameters.push_back(Parameter("Radius", radius));
    }
    TopoShape* compute() override {
        if (shape) topo_shape_free(shape);
        shape = nullptr;
        return shape;
    }
    void regenerate() override { compute(); }
    void translate(const Vec3& delta) override {}
    void rotate(const Vec3& axis, float angle) override {}
    void scale(const Vec3& scale) override {}
    std::unique_ptr<Feature> clone() const override { return std::make_unique<FilletFeature>(edges, radius); }
    void serialize(cJSON* json) const override;
};

class ChamferFeature : public Feature {
private:
    std::vector<TopoShape*> edges;
    float distance;
public:
    ChamferFeature(const std::vector<TopoShape*>& edges, float distance)
        : Feature(FeatureType::CHAMFER, "Chamfer"), edges(edges), distance(distance) {
        parameters.push_back(Parameter("Distance", distance));
    }
    TopoShape* compute() override {
        if (shape) topo_shape_free(shape);
        shape = nullptr;
        return shape;
    }
    void regenerate() override { compute(); }
    void translate(const Vec3& delta) override {}
    void rotate(const Vec3& axis, float angle) override {}
    void scale(const Vec3& scale) override {}
    std::unique_ptr<Feature> clone() const override { return std::make_unique<ChamferFeature>(edges, distance); }
    void serialize(cJSON* json) const override;
};

class HoleFeature : public Feature {
private:
    Sketch* sketch;
    float depth;
    bool is_through;
public:
    HoleFeature(Sketch* sketch, float depth, bool is_through = true)
        : Feature(FeatureType::HOLE, "Hole"), sketch(sketch), depth(depth), is_through(is_through) {
        parameters.push_back(Parameter("Depth", depth));
        parameters.push_back(Parameter("Through", is_through));
    }
    TopoShape* compute() override {
        if (shape) topo_shape_free(shape);
        shape = topo_create_cylinder(VEC3_ZERO, VEC3_Z, 2.0f, depth);
        return shape;
    }
    void regenerate() override { compute(); }
    void translate(const Vec3& delta) override { if (shape) shape = topo_shape_transform(shape, mat4_translation(delta)); }
    void rotate(const Vec3& axis, float angle) override { if (shape) shape = topo_shape_transform(shape, mat4_rotation_axis(axis, angle)); }
    void scale(const Vec3& scale) override { if (shape) shape = topo_shape_transform(shape, mat4_scale(scale)); }
    std::unique_ptr<Feature> clone() const override { return std::make_unique<HoleFeature>(sketch, depth, is_through); }
    void serialize(cJSON* json) const override;
};

class BooleanFeature : public Feature {
private:
    Feature* target;
    Feature* tool;
    FeatureType boolean_type;
public:
    BooleanFeature(Feature* target, Feature* tool, FeatureType boolean_type)
        : Feature(boolean_type, "Boolean"), target(target), tool(tool), boolean_type(boolean_type) {
        parent = target;
    }
    TopoShape* compute() override {
        if (shape) topo_shape_free(shape);
        if (!target || !tool) return nullptr;
        switch (boolean_type) {
            case FeatureType::BOOLEAN_UNION: break;
            case FeatureType::BOOLEAN_DIFFERENCE: break;
            case FeatureType::BOOLEAN_INTERSECTION: break;
            default: break;
        }
        return shape;
    }
    void regenerate() override { compute(); }
    void translate(const Vec3& delta) override {}
    void rotate(const Vec3& axis, float angle) override {}
    void scale(const Vec3& scale) override {}
    std::unique_ptr<Feature> clone() const override { return std::make_unique<BooleanFeature>(target, tool, boolean_type); }
    void serialize(cJSON* json) const override;
};

#endif