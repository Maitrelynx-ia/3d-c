#ifndef CAD_STATIC_ANALYSIS_HPP
#define CAD_STATIC_ANALYSIS_HPP

#include "../mesh/TetMesh.hpp"
#include "../core/geometry/vec3.h"
#include <vector>
#include <memory>

struct FEMaterial {
    std::string name;
    float young_modulus;
    float poisson_ratio;
    float density;
    float yield_strength;
    float thermal_conductivity;
    float thermal_expansion;
};

enum class BoundaryConditionType {
    FIXED, DISPLACEMENT, FORCE, PRESSURE, ACCELERATION
};

class BoundaryCondition {
public:
    BoundaryConditionType type;
    std::vector<int> nodes;
    Vec3 value;
    BoundaryCondition(BoundaryConditionType type, const std::vector<int>& nodes, const Vec3& value)
        : type(type), nodes(nodes), value(value) {}
    virtual ~BoundaryCondition() = default;
    virtual std::unique_ptr<BoundaryCondition> clone() const;
};

class StaticAnalysis {
private:
    std::shared_ptr<TetMesh> mesh;
    FEMaterial material;
    std::vector<std::unique_ptr<BoundaryCondition>> boundary_conditions;
    class SparseMatrix* stiffness_matrix;
    std::vector<Vec3> forces;
    std::vector<Vec3> displacements;
    std::vector<float> von_mises_stresses;
    std::vector<float> strains;
    bool solveSystem();
    void computeStresses();
    void computeStrains();
public:
    StaticAnalysis(std::shared_ptr<TetMesh> mesh, const FEMaterial& material);
    ~StaticAnalysis();
    std::shared_ptr<TetMesh> getMesh() const { return mesh; }
    void setMesh(std::shared_ptr<TetMesh> mesh) { this->mesh = mesh; }
    const FEMaterial& getMaterial() const { return material; }
    void setMaterial(const FEMaterial& material) { this->material = material; }
    void addFixedConstraint(const std::vector<int>& nodes);
    void addDisplacementConstraint(const std::vector<int>& nodes, const Vec3& displacement);
    void addForce(const std::vector<int>& nodes, const Vec3& force);
    void addPressure(const std::vector<int>& faces, float pressure);
    const std::vector<std::unique_ptr<BoundaryCondition>>& getBoundaryConditions() const { return boundary_conditions; }
    bool solve();
    const std::vector<Vec3>& getDisplacements() const { return displacements; }
    const std::vector<float>& getVonMisesStresses() const { return von_mises_stresses; }
    const std::vector<float>& getStrains() const { return strains; }
    float getMaxVonMisesStress() const;
    float getMinVonMisesStress() const;
    Vec3 getMaxDisplacement() const;
    Vec3 getMinDisplacement() const;
    float computeStrainEnergy() const;
    void exportResultsToVTK(const std::string& filename) const;
    std::unique_ptr<StaticAnalysis> clone() const;
};

#endif // CAD_STATIC_ANALYSIS_HPP
