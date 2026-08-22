#ifndef CAD_PART_HPP
#define CAD_PART_HPP

#include "Feature.hpp"
#include "Sketch.hpp"
#include "../core/geometry/brep.h"
#include <vector>
#include <memory>
#include <string>

class Assembly;

class Material {
public:
    std::string name;
    float density;
    float young_modulus;
    float poisson_ratio;
    float yield_strength;
    float thermal_conductivity;
    float thermal_expansion;
    Material() : name("Default"), density(1000.0f), young_modulus(1e9f), poisson_ratio(0.3f), yield_strength(1e8f), thermal_conductivity(50.0f), thermal_expansion(1e-5f) {}
    Material(const std::string& name, float density, float young_modulus, float poisson_ratio) : name(name), density(density), young_modulus(young_modulus), poisson_ratio(poisson_ratio), yield_strength(0), thermal_conductivity(0), thermal_expansion(0) {}
    static Material Steel() { return Material("Steel", 7850.0f, 210e9f, 0.3f); }
    static Material Aluminum() { return Material("Aluminum", 2700.0f, 70e9f, 0.33f); }
    static Material Plastic() { return Material("Plastic", 1200.0f, 3e9f, 0.35f); }
};

class Part {
private:
    std::string name;
    std::vector<std::unique_ptr<Feature>> features;
    std::vector<std::unique_ptr<Sketch>> sketches;
    TopoShape* body;
    Material material;
    bool is_suppressed;
    bool is_visible;
    void regenerateBody();
public:
    Part(const std::string& name = "Part");
    ~Part();
    const std::string& getName() const { return name; }
    void setName(const std::string& name) { this->name = name; }
    const Material& getMaterial() const { return material; }
    void setMaterial(const Material& material) { this->material = material; }
    bool getIsVisible() const { return is_visible; }
    void setIsVisible(bool visible) { is_visible = visible; }
    bool getIsSuppressed() const { return is_suppressed; }
    void setIsSuppressed(bool suppressed) { is_suppressed = suppressed; }
    Sketch* createSketch(const Plane& plane);
    Sketch* createSketch(SketchPlane plane_type = SketchPlane::XY);
    void removeSketch(Sketch* sketch);
    const std::vector<std::unique_ptr<Sketch>>& getSketches() const { return sketches; }
    ExtrudeFeature* addExtrude(Sketch* sketch, float depth, bool is_additive = true);
    RevolveFeature* addRevolve(Sketch* sketch, float angle, const Vec3& axis);
    FilletFeature* addFillet(const std::vector<TopoShape*>& edges, float radius);
    ChamferFeature* addChamfer(const std::vector<TopoShape*>& edges, float distance);
    HoleFeature* addHole(Sketch* sketch, float depth, bool is_through = true);
    BooleanFeature* addBooleanUnion(Feature* target, Feature* tool);
    BooleanFeature* addBooleanDifference(Feature* target, Feature* tool);
    BooleanFeature* addBooleanIntersection(Feature* target, Feature* tool);
    void removeFeature(Feature* feature);
    const std::vector<std::unique_ptr<Feature>>& getFeatures() const { return features; }
    void regenerate();
    TopoShape* getBody() const { return body; }
    void updateBody();
    void translate(const Vec3& delta);
    void rotate(const Vec3& axis, float angle);
    void scale(const Vec3& scale);
    float computeMass() const;
    Vec3 computeCenterOfMass() const;
    float computeVolume() const;
    AABB computeAABB() const;
    std::unique_ptr<Part> clone() const;
    void serialize(cJSON* json) const;
    static std::unique_ptr<Part> deserialize(cJSON* json);
};

#endif