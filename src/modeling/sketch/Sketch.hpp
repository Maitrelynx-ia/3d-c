#ifndef CAD_SKETCH_HPP
#define CAD_SKETCH_HPP

#include "SketchEntity.hpp"
#include "constraint.hpp"
#include "../core/geometry/vec3.h"
#include "../core/geometry/mat4.h"
#include <vector>
#include <memory>
#include <string>

// --- Plan d'esquisse ---
enum class SketchPlane {
    XY,
    XZ,
    YZ,
    CUSTOM
};

class Plane {
public:
    Vec3 origin;
    Vec3 normal;
    
    Plane() : origin(VEC3_ZERO), normal(VEC3_Z) {}
    Plane(Vec3 origin, Vec3 normal) : origin(origin), normal(vec3_normalize(normal)) {}
    
    static Plane XY() { return Plane(VEC3_ZERO, VEC3_Z); }
    static Plane XZ() { return Plane(VEC3_ZERO, VEC3_Y); }
    static Plane YZ() { return Plane(VEC3_ZERO, VEC3_X); }
    
    Vec3 toPlane(Vec3 point) const;
    Vec3 fromPlane(Vec3 point) const;
};

// --- Esquisse 2D ---
class Sketch {
private:
    std::string name;
    Plane plane;
    std::vector<std::unique_ptr<SketchEntity>> entities;
    std::vector<std::unique_ptr<Constraint>> constraints;
    bool is_solved;
    
    bool solveConstraints();
    
public:
    Sketch(const std::string& name = "Sketch", SketchPlane plane_type = SketchPlane::XY);
    Sketch(const std::string& name, const Plane& plane);
    ~Sketch();
    
    const std::string& getName() const { return name; }
    void setName(const std::string& name) { this->name = name; }
    
    const Plane& getPlane() const { return plane; }
    void setPlane(const Plane& plane) { this->plane = plane; }
    
    SketchLine* addLine(const Vec3& start, const Vec3& end, bool is_construction = false);
    SketchCircle* addCircle(const Vec3& center, float radius, bool is_construction = false);
    SketchArc* addArc(const Vec3& center, float radius, float start_angle, float end_angle, bool is_construction = false);
    SketchPoint* addPoint(const Vec3& position, bool is_construction = false);
    
    void removeEntity(SketchEntity* entity);
    const std::vector<std::unique_ptr<SketchEntity>>& getEntities() const { return entities; }
    
    CoincidentConstraint* addCoincidentConstraint(SketchEntity* entity1, SketchEntity* entity2);
    DistanceConstraint* addDistanceConstraint(SketchEntity* entity1, SketchEntity* entity2, float distance);
    ParallelConstraint* addParallelConstraint(SketchEntity* entity1, SketchEntity* entity2);
    PerpendicularConstraint* addPerpendicularConstraint(SketchEntity* entity1, SketchEntity* entity2);
    TangentConstraint* addTangentConstraint(SketchEntity* entity1, SketchEntity* entity2);
    RadiusConstraint* addRadiusConstraint(SketchEntity* entity, float radius);
    AngleConstraint* addAngleConstraint(SketchEntity* entity1, SketchEntity* entity2, float angle);
    
    void removeConstraint(Constraint* constraint);
    const std::vector<std::unique_ptr<Constraint>>& getConstraints() const { return constraints; }
    
    bool solve();
    bool isSolved() const { return is_solved; }
    
    void translate(const Vec3& delta);
    void rotate(const Vec3& axis, float angle);
    void scale(const Vec3& scale);
    void mirror(const Vec3& normal);
    
    class Solid* extrude(float depth, bool is_additive = true);
    class Solid* revolve(float angle, const Vec3& axis);
    
    std::unique_ptr<Sketch> clone() const;
    
    void serialize(cJSON* json) const;
    static std::unique_ptr<Sketch> deserialize(cJSON* json);
};

#endif