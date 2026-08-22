#include "Part.hpp"
#include "Feature.hpp"
#include "Sketch.hpp"
#include <algorithm>

Part::Part(const std::string& name) 
    : name(name), body(nullptr), material(Material::Steel()), is_suppressed(false), is_visible(true) {}

Part::~Part() {
    if (body) topo_shape_free(body);
}

Sketch* Part::createSketch(const Plane& plane) {
    auto sketch = std::make_unique<Sketch>("Sketch_" + std::to_string(sketches.size() + 1), plane);
    Sketch* ptr = sketch.get();
    sketches.push_back(std::move(sketch));
    return ptr;
}

Sketch* Part::createSketch(SketchPlane plane_type) {
    auto sketch = std::make_unique<Sketch>("Sketch_" + std::to_string(sketches.size() + 1), plane_type);
    Sketch* ptr = sketch.get();
    sketches.push_back(std::move(sketch));
    return ptr;
}

void Part::removeSketch(Sketch* sketch) {
    auto it = std::remove_if(sketches.begin(), sketches.end(), 
        [sketch](const std::unique_ptr<Sketch>& s) { return s.get() == sketch; });
    sketches.erase(it, sketches.end());
}

ExtrudeFeature* Part::addExtrude(Sketch* sketch, float depth, bool is_additive) {
    auto feature = std::make_unique<ExtrudeFeature>(sketch, depth, is_additive);
    ExtrudeFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

RevolveFeature* Part::addRevolve(Sketch* sketch, float angle, const Vec3& axis) {
    auto feature = std::make_unique<RevolveFeature>(sketch, angle, axis);
    RevolveFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

FilletFeature* Part::addFillet(const std::vector<TopoShape*>& edges, float radius) {
    auto feature = std::make_unique<FilletFeature>(edges, radius);
    FilletFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

ChamferFeature* Part::addChamfer(const std::vector<TopoShape*>& edges, float distance) {
    auto feature = std::make_unique<ChamferFeature>(edges, distance);
    ChamferFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

HoleFeature* Part::addHole(Sketch* sketch, float depth, bool is_through) {
    auto feature = std::make_unique<HoleFeature>(sketch, depth, is_through);
    HoleFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

BooleanFeature* Part::addBooleanUnion(Feature* target, Feature* tool) {
    auto feature = std::make_unique<BooleanFeature>(target, tool, FeatureType::BOOLEAN_UNION);
    BooleanFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

BooleanFeature* Part::addBooleanDifference(Feature* target, Feature* tool) {
    auto feature = std::make_unique<BooleanFeature>(target, tool, FeatureType::BOOLEAN_DIFFERENCE);
    BooleanFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

BooleanFeature* Part::addBooleanIntersection(Feature* target, Feature* tool) {
    auto feature = std::make_unique<BooleanFeature>(target, tool, FeatureType::BOOLEAN_INTERSECTION);
    BooleanFeature* ptr = feature.get();
    features.push_back(std::move(feature));
    regenerate();
    return ptr;
}

void Part::removeFeature(Feature* feature) {
    auto it = std::remove_if(features.begin(), features.end(), 
        [feature](const std::unique_ptr<Feature>& f) { return f.get() == feature; });
    features.erase(it, features.end());
    regenerate();
}

void Part::regenerateBody() {
    if (body) { topo_shape_free(body); body = nullptr; }
    if (!features.empty()) {
        body = features[0]->compute();
        for (size_t i = 1; i < features.size(); i++) {
        }
    }
}

void Part::regenerate() {
    for (auto& feature : features) feature->regenerate();
    regenerateBody();
}

void Part::updateBody() { regenerateBody(); }

void Part::translate(const Vec3& delta) {
    for (auto& feature : features) feature->translate(delta);
    regenerateBody();
}

void Part::rotate(const Vec3& axis, float angle) {
    for (auto& feature : features) feature->rotate(axis, angle);
    regenerateBody();
}

void Part::scale(const Vec3& scale) {
    for (auto& feature : features) feature->scale(scale);
    regenerateBody();
}

float Part::computeMass() const {
    if (!body) return 0.0f;
    return material.density * computeVolume();
}

Vec3 Part::computeCenterOfMass() const {
    if (!body) return VEC3_ZERO;
    return VEC3_ZERO;
}

float Part::computeVolume() const {
    if (!body) return 0.0f;
    if (topo_shape_is_solid(body)) {
        AABB aabb = topo_shape_get_aabb(body);
        Vec3 size = aabb_size(aabb);
        return size.x * size.y * size.z;
    }
    return 0.0f;
}

AABB Part::computeAABB() const {
    if (!body) return AABB_EMPTY;
    return topo_shape_get_aabb(body);
}

std::unique_ptr<Part> Part::clone() const {
    auto new_part = std::make_unique<Part>(name);
    new_part->material = material;
    new_part->is_suppressed = is_suppressed;
    new_part->is_visible = is_visible;
    for (const auto& sketch : sketches) new_part->sketches.push_back(sketch->clone());
    for (const auto& feature : features) new_part->features.push_back(feature->clone());
    return new_part;
}

