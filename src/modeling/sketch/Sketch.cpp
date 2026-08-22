#include "Sketch.hpp"
#include "SketchEntity.hpp"
#include "constraint.hpp"
#include "../modeling/part/Solid.hpp"
#include <algorithm>

// --- Plane ---
Vec3 Plane::toPlane(Vec3 point) const {
    Vec3 offset = vec3_sub(point, origin);
    float distance = vec3_dot(offset, normal);
    return vec3_sub(point, vec3_mul(normal, distance));
}

Vec3 Plane::fromPlane(Vec3 point) const {
    if (vec3_equal(normal, VEC3_Z, 1e-6f)) {
        return vec3_add(origin, (Vec3){point.x, point.y, 0});
    } else if (vec3_equal(normal, VEC3_Y, 1e-6f)) {
        return vec3_add(origin, (Vec3){point.x, 0, point.y});
    } else if (vec3_equal(normal, VEC3_X, 1e-6f)) {
        return vec3_add(origin, (Vec3){0, point.x, point.y});
    }
    return vec3_add(origin, point);
}

// --- Sketch ---
Sketch::Sketch(const std::string& name, SketchPlane plane_type) 
    : name(name), is_solved(false) {
    switch (plane_type) {
        case SketchPlane::XY: plane = Plane::XY(); break;
        case SketchPlane::XZ: plane = Plane::XZ(); break;
        case SketchPlane::YZ: plane = Plane::YZ(); break;
        default: plane = Plane::XY(); break;
    }
}

Sketch::Sketch(const std::string& name, const Plane& plane) 
    : name(name), plane(plane), is_solved(false) {}

Sketch::~Sketch() = default;

SketchLine* Sketch::addLine(const Vec3& start, const Vec3& end, bool is_construction) {
    auto line = std::make_unique<SketchLine>(start, end, is_construction);
    SketchLine* ptr = line.get();
    entities.push_back(std::move(line));
    is_solved = false;
    return ptr;
}

SketchCircle* Sketch::addCircle(const Vec3& center, float radius, bool is_construction) {
    auto circle = std::make_unique<SketchCircle>(center, radius, is_construction);
    SketchCircle* ptr = circle.get();
    entities.push_back(std::move(circle));
    is_solved = false;
    return ptr;
}

SketchArc* Sketch::addArc(const Vec3& center, float radius, float start_angle, float end_angle, bool is_construction) {
    auto arc = std::make_unique<SketchArc>(center, radius, start_angle, end_angle, is_construction);
    SketchArc* ptr = arc.get();
    entities.push_back(std::move(arc));
    is_solved = false;
    return ptr;
}

SketchPoint* Sketch::addPoint(const Vec3& position, bool is_construction) {
    auto point = std::make_unique<SketchPoint>(position, is_construction);
    SketchPoint* ptr = point.get();
    entities.push_back(std::move(point));
    is_solved = false;
    return ptr;
}

void Sketch::removeEntity(SketchEntity* entity) {
    auto it = std::remove_if(entities.begin(), entities.end(), 
        [entity](const std::unique_ptr<SketchEntity>& e) { return e.get() == entity; });
    entities.erase(it, entities.end());
    is_solved = false;
}

CoincidentConstraint* Sketch::addCoincidentConstraint(SketchEntity* entity1, SketchEntity* entity2) {
    auto constraint = std::make_unique<CoincidentConstraint>(entity1, entity2);
    CoincidentConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

DistanceConstraint* Sketch::addDistanceConstraint(SketchEntity* entity1, SketchEntity* entity2, float distance) {
    auto constraint = std::make_unique<DistanceConstraint>(entity1, entity2, distance);
    DistanceConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

ParallelConstraint* Sketch::addParallelConstraint(SketchEntity* entity1, SketchEntity* entity2) {
    auto constraint = std::make_unique<ParallelConstraint>(entity1, entity2);
    ParallelConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

PerpendicularConstraint* Sketch::addPerpendicularConstraint(SketchEntity* entity1, SketchEntity* entity2) {
    auto constraint = std::make_unique<PerpendicularConstraint>(entity1, entity2);
    PerpendicularConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

TangentConstraint* Sketch::addTangentConstraint(SketchEntity* entity1, SketchEntity* entity2) {
    auto constraint = std::make_unique<TangentConstraint>(entity1, entity2);
    TangentConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

RadiusConstraint* Sketch::addRadiusConstraint(SketchEntity* entity, float radius) {
    auto constraint = std::make_unique<RadiusConstraint>(entity, radius);
    RadiusConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

AngleConstraint* Sketch::addAngleConstraint(SketchEntity* entity1, SketchEntity* entity2, float angle) {
    auto constraint = std::make_unique<AngleConstraint>(entity1, entity2, angle);
    AngleConstraint* ptr = constraint.get();
    constraints.push_back(std::move(constraint));
    is_solved = false;
    return ptr;
}

void Sketch::removeConstraint(Constraint* constraint) {
    auto it = std::remove_if(constraints.begin(), constraints.end(), 
        [constraint](const std::unique_ptr<Constraint>& c) { return c.get() == constraint; });
    constraints.erase(it, constraints.end());
    is_solved = false;
}

bool Sketch::solveConstraints() {
    bool all_satisfied = true;
    for (auto& constraint : constraints) {
        if (!constraint->solve()) {
            all_satisfied = false;
            constraint->status = ConstraintStatus::CONFLICT;
        } else {
            constraint->status = ConstraintStatus::SATISFIED;
        }
    }
    return all_satisfied;
}

bool Sketch::solve() {
    is_solved = solveConstraints();
    return is_solved;
}

void Sketch::translate(const Vec3& delta) {
    for (auto& entity : entities) entity->translate(delta);
    is_solved = false;
}

void Sketch::rotate(const Vec3& axis, float angle) {
    for (auto& entity : entities) entity->rotate(axis, angle);
    is_solved = false;
}

void Sketch::scale(const Vec3& scale) {
    for (auto& entity : entities) entity->scale(scale);
    is_solved = false;
}

void Sketch::mirror(const Vec3& normal) {
    for (auto& entity : entities) entity->mirror(normal);
    is_solved = false;
}

class Solid* Sketch::extrude(float depth, bool is_additive) {
    Solid* solid = new Solid("Extruded_" + name);
    return solid;
}

class Solid* Sketch::revolve(float angle, const Vec3& axis) {
    Solid* solid = new Solid("Revolved_" + name);
    return solid;
}

std::unique_ptr<Sketch> Sketch::clone() const {
    auto new_sketch = std::make_unique<Sketch>(name, plane);
    for (const auto& entity : entities) new_sketch->entities.push_back(entity->clone());
    new_sketch->is_solved = is_solved;
    return new_sketch;
}

