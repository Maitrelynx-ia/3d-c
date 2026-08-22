#include "Assembly.hpp"
#include "../part/Part.hpp"
#include "../core/geometry/brep.h"
#include <algorithm>

Assembly::Assembly(const std::string& name) : name(name), octree(nullptr), is_solved(false) {}
Assembly::~Assembly() { if (octree) {} }

void Assembly::addPart(Part* part) { parts.push_back(part); updateOctree(); is_solved = false; }
void Assembly::removePart(Part* part) { auto it = std::remove(parts.begin(), parts.end(), part); parts.erase(it, parts.end()); updateOctree(); is_solved = false; }

Part* Assembly::getPart(const std::string& name) const {
    for (Part* part : parts) if (part->getName() == name) return part;
    return nullptr;
}

AssemblyConstraint* Assembly::addCoincidentConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2) {
    auto constraint = std::make_unique<AssemblyConstraint>(part1, shape1, part2, shape2, AssemblyConstraintType::COINCIDENT);
    AssemblyConstraint* ptr = constraint.get(); constraints.push_back(std::move(constraint)); is_solved = false; return ptr;
}

AssemblyConstraint* Assembly::addParallelConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2) {
    auto constraint = std::make_unique<AssemblyConstraint>(part1, shape1, part2, shape2, AssemblyConstraintType::PARALLEL);
    AssemblyConstraint* ptr = constraint.get(); constraints.push_back(std::move(constraint)); is_solved = false; return ptr;
}

AssemblyConstraint* Assembly::addPerpendicularConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2) {
    auto constraint = std::make_unique<AssemblyConstraint>(part1, shape1, part2, shape2, AssemblyConstraintType::PERPENDICULAR);
    AssemblyConstraint* ptr = constraint.get(); constraints.push_back(std::move(constraint)); is_solved = false; return ptr;
}

AssemblyConstraint* Assembly::addDistanceConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2, float distance) {
    auto constraint = std::make_unique<AssemblyConstraint>(part1, shape1, part2, shape2, AssemblyConstraintType::DISTANCE, distance);
    AssemblyConstraint* ptr = constraint.get(); constraints.push_back(std::move(constraint)); is_solved = false; return ptr;
}

AssemblyConstraint* Assembly::addAngleConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2, float angle) {
    auto constraint = std::make_unique<AssemblyConstraint>(part1, shape1, part2, shape2, AssemblyConstraintType::ANGLE, angle);
    AssemblyConstraint* ptr = constraint.get(); constraints.push_back(std::move(constraint)); is_solved = false; return ptr;
}

AssemblyConstraint* Assembly::addInsertConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2) {
    auto constraint = std::make_unique<AssemblyConstraint>(part1, shape1, part2, shape2, AssemblyConstraintType::INSERT);
    AssemblyConstraint* ptr = constraint.get(); constraints.push_back(std::move(constraint)); is_solved = false; return ptr;
}

void Assembly::removeConstraint(AssemblyConstraint* constraint) {
    auto it = std::remove_if(constraints.begin(), constraints.end(), [constraint](const std::unique_ptr<AssemblyConstraint>& c) { return c.get() == constraint; });
    constraints.erase(it, constraints.end()); is_solved = false;
}

bool Assembly::solveConstraints() {
    bool all_satisfied = true;
    for (auto& constraint : constraints) {
        if (!constraint->solve()) { all_satisfied = false; constraint->status = AssemblyConstraintStatus::CONFLICT; }
        else { constraint->status = AssemblyConstraintStatus::SATISFIED; }
    }
    is_solved = all_satisfied; return all_satisfied;
}

std::vector<std::pair<Part*, Part*>> Assembly::detectCollisions() {
    std::vector<std::pair<Part*, Part*>> collisions;
    if (!octree) updateOctree();
    for (size_t i = 0; i < parts.size(); i++) {
        for (size_t j = i + 1; j < parts.size(); j++) {
            AABB aabb1 = parts[i]->computeAABB(); AABB aabb2 = parts[j]->computeAABB();
            if (aabb_intersect(aabb1, aabb2)) collisions.emplace_back(parts[i], parts[j]);
        }
    }
    return collisions;
}

bool Assembly::hasCollisions() const { return !detectCollisions().empty(); }

void Assembly::explode(float distance) {
    Vec3 center = VEC3_ZERO; int count = 0;
    for (Part* part : parts) { center = vec3_add(center, part->computeAABB().min); center = vec3_add(center, part->computeAABB().max); count += 2; }
    if (count > 0) center = vec3_div(center, (float)count);
    for (size_t i = 0; i < parts.size(); i++) {
        Vec3 direction = vec3_normalize(vec3_sub(parts[i]->computeCenterOfMass(), center));
        parts[i]->translate(vec3_mul(direction, distance * (i + 1)));
    }
}

void Assembly::translate(const Vec3& delta) { for (Part* part : parts) part->translate(delta); updateOctree(); }
void Assembly::rotate(const Vec3& axis, float angle) { for (Part* part : parts) part->rotate(axis, angle); updateOctree(); }

void Assembly::updateOctree() {
    if (octree) {}
    AABB bounds = AABB_EMPTY;
    for (Part* part : parts) {
        AABB aabb = part->computeAABB();
        if (aabb.min.x < bounds.min.x) bounds.min.x = aabb.min.x;
        if (aabb.min.y < bounds.min.y) bounds.min.y = aabb.min.y;
        if (aabb.min.z < bounds.min.z) bounds.min.z = aabb.min.z;
        if (aabb.max.x > bounds.max.x) bounds.max.x = aabb.max.x;
        if (aabb.max.y > bounds.max.y) bounds.max.y = aabb.max.y;
        if (aabb.max.z > bounds.max.z) bounds.max.z = aabb.max.z;
    }
}

std::unique_ptr<Assembly> Assembly::clone() const {
    auto new_assembly = std::make_unique<Assembly>(name);
    for (Part* part : parts) new_assembly->parts.push_back(part->clone().release());
    for (const auto& constraint : constraints) new_assembly->constraints.push_back(constraint->clone());
    new_assembly->is_solved = is_solved; return new_assembly;
}

