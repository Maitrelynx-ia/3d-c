#ifndef CAD_ASSEMBLY_HPP
#define CAD_ASSEMBLY_HPP

#include "../part/Part.hpp"
#include "../core/geometry/brep.h"
#include <vector>
#include <memory>
#include <string>

enum class AssemblyConstraintType {
    COINCIDENT,
    PARALLEL,
    PERPENDICULAR,
    TANGENT,
    DISTANCE,
    ANGLE,
    INSERT
};

enum class AssemblyConstraintStatus {
    UNSOLVED,
    SATISFIED,
    CONFLICT
};

class AssemblyConstraint {
public:
    AssemblyConstraintType type;
    AssemblyConstraintStatus status;
    Part* part1;
    Part* part2;
    TopoShape* shape1;
    TopoShape* shape2;
    float value;
    AssemblyConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2, AssemblyConstraintType type, float value = 0.0f)
        : type(type), status(AssemblyConstraintStatus::UNSOLVED), part1(part1), part2(part2), shape1(shape1), shape2(shape2), value(value) {}
    ~AssemblyConstraint() = default;
    bool solve();
    std::unique_ptr<AssemblyConstraint> clone() const;
    void serialize(cJSON* json) const;
};

class Assembly {
private:
    std::string name;
    std::vector<Part*> parts;
    std::vector<std::unique_ptr<AssemblyConstraint>> constraints;
    bool is_solved;
    void updateOctree();
public:
    Assembly(const std::string& name = "Assembly");
    ~Assembly();
    const std::string& getName() const { return name; }
    void setName(const std::string& name) { this->name = name; }
    void addPart(Part* part);
    void removePart(Part* part);
    const std::vector<Part*>& getParts() const { return parts; }
    Part* getPart(const std::string& name) const;
    AssemblyConstraint* addCoincidentConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2);
    AssemblyConstraint* addParallelConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2);
    AssemblyConstraint* addPerpendicularConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2);
    AssemblyConstraint* addDistanceConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2, float distance);
    AssemblyConstraint* addAngleConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2, float angle);
    AssemblyConstraint* addInsertConstraint(Part* part1, TopoShape* shape1, Part* part2, TopoShape* shape2);
    void removeConstraint(AssemblyConstraint* constraint);
    const std::vector<std::unique_ptr<AssemblyConstraint>>& getConstraints() const { return constraints; }
    bool solveConstraints();
    bool isSolved() const { return is_solved; }
    std::vector<std::pair<Part*, Part*>> detectCollisions();
    bool hasCollisions() const;
    void explode(float distance);
    void translate(const Vec3& delta);
    void rotate(const Vec3& axis, float angle);
    std::unique_ptr<Assembly> clone() const;
    void serialize(cJSON* json) const;
    static std::unique_ptr<Assembly> deserialize(cJSON* json);
};

#endif