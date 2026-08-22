#include "StaticAnalysis.hpp"
#include "../mesh/TetMesh.hpp"
#include "../solver/sparse_matrix.h"
#include "../solver/conjugate_gradient.h"
#include <algorithm>

StaticAnalysis::StaticAnalysis(std::shared_ptr<TetMesh> mesh, const FEMaterial& material)
    : mesh(mesh), material(material), stiffness_matrix(nullptr) {
    if (mesh) {
        forces.resize(mesh->getNodeCount());
        displacements.resize(mesh->getNodeCount());
        von_mises_stresses.resize(mesh->getElementCount());
        strains.resize(mesh->getElementCount());
    }
}

StaticAnalysis::~StaticAnalysis() {
    if (stiffness_matrix) sparse_matrix_destroy(stiffness_matrix);
}

void StaticAnalysis::addFixedConstraint(const std::vector<int>& nodes) {
    boundary_conditions.push_back(std::make_unique<BoundaryCondition>(BoundaryConditionType::FIXED, nodes, VEC3_ZERO));
}

void StaticAnalysis::addDisplacementConstraint(const std::vector<int>& nodes, const Vec3& displacement) {
    boundary_conditions.push_back(std::make_unique<BoundaryCondition>(BoundaryConditionType::DISPLACEMENT, nodes, displacement));
}

void StaticAnalysis::addForce(const std::vector<int>& nodes, const Vec3& force) {
    boundary_conditions.push_back(std::make_unique<BoundaryCondition>(BoundaryConditionType::FORCE, nodes, force));
}

void StaticAnalysis::addPressure(const std::vector<int>& faces, float pressure) {
}

bool StaticAnalysis::solveSystem() {
    if (!mesh || mesh->getNodeCount() == 0) return false;
    size_t num_nodes = mesh->getNodeCount();
    size_t num_dof = num_nodes * 3;
    if (stiffness_matrix) sparse_matrix_destroy(stiffness_matrix);
    stiffness_matrix = sparse_matrix_create(num_dof);
    std::fill(forces.begin(), forces.end(), VEC3_ZERO);
    std::fill(displacements.begin(), displacements.end(), VEC3_ZERO);
    float E = material.young_modulus;
    float nu = material.poisson_ratio;
    for (const auto& element : mesh->getElements()) {
        if (element->type != ElementType::TETRAHEDRON) continue;
        const TetrahedronElement* tet = static_cast<const TetrahedronElement*>(element.get());
        const Vec3& n0 = mesh->getNodes()[tet->node_indices[0]];
        const Vec3& n1 = mesh->getNodes()[tet->node_indices[1]];
        const Vec3& n2 = mesh->getNodes()[tet->node_indices[2]];
        const Vec3& n3 = mesh->getNodes()[tet->node_indices[3]];
        float volume = tet->computeVolume(mesh->getNodes());
        float k = E * volume / (1.0f + nu);
        for (int i = 0; i < 4; i++) {
            int node_idx = tet->node_indices[i];
            for (int j = 0; j < 4; j++) {
                int other_node_idx = tet->node_indices[j];
                for (int d1 = 0; d1 < 3; d1++) {
                    for (int d2 = 0; d2 < 3; d2++) {
                        size_t row = node_idx * 3 + d1;
                        size_t col = other_node_idx * 3 + d2;
                        float value = (i == j) ? k : -k / 3.0f;
                        sparse_matrix_set(stiffness_matrix, row, col, value);
                    }
                }
            }
        }
    }
    for (const auto& bc : boundary_conditions) {
        if (bc->type == BoundaryConditionType::FORCE) {
            for (int node_idx : bc->nodes) {
                if (node_idx >= 0 && node_idx < (int)forces.size()) {
                    forces[node_idx] = vec3_add(forces[node_idx], bc->value);
                }
            }
        }
    }
    Vector* b = vector_create(num_dof);
    for (size_t i = 0; i < num_nodes; i++) {
        vector_set(b, i * 3 + 0, forces[i].x);
        vector_set(b, i * 3 + 1, forces[i].y);
        vector_set(b, i * 3 + 2, forces[i].z);
    }
    Vector* u = conjugate_gradient(stiffness_matrix, b, 1e-6f, 1000);
    for (size_t i = 0; i < num_nodes; i++) {
        displacements[i].x = vector_get(u, i * 3 + 0);
        displacements[i].y = vector_get(u, i * 3 + 1);
        displacements[i].z = vector_get(u, i * 3 + 2);
    }
    vector_destroy(b);
    vector_destroy(u);
    computeStresses();
    computeStrains();
    return true;
}

bool StaticAnalysis::solve() { return solveSystem(); }

void StaticAnalysis::computeStresses() {
    if (!mesh || displacements.empty()) { std::fill(von_mises_stresses.begin(), von_mises_stresses.end(), 0.0f); return; }
    float E = material.young_modulus;
    for (size_t i = 0; i < mesh->getElements().size(); i++) {
        const auto& element = mesh->getElements()[i];
        if (element->type != ElementType::TETRAHEDRON) { von_mises_stresses[i] = 0.0f; continue; }
        const TetrahedronElement* tet = static_cast<const TetrahedronElement*>(element.get());
        Vec3 nodes[4]; Vec3 displs[4];
        for (int j = 0; j < 4; j++) { int node_idx = tet->node_indices[j]; nodes[j] = mesh->getNodes()[node_idx]; displs[j] = displacements[node_idx]; }
        Vec3 centroid = element->computeCentroid(mesh->getNodes());
        float avg_strain = 0.0f;
        for (int j = 0; j < 4; j++) avg_strain += vec3_length(displs[j]);
        avg_strain /= 4.0f;
        von_mises_stresses[i] = E * avg_strain;
    }
}

void StaticAnalysis::computeStrains() {
    if (!mesh || displacements.empty()) { std::fill(strains.begin(), strains.end(), 0.0f); return; }
    for (size_t i = 0; i < mesh->getElements().size(); i++) {
        const auto& element = mesh->getElements()[i];
        if (element->type != ElementType::TETRAHEDRON) { strains[i] = 0.0f; continue; }
        const TetrahedronElement* tet = static_cast<const TetrahedronElement*>(element.get());
        float avg_strain = 0.0f;
        for (int j = 0; j < 4; j++) { int node_idx = tet->node_indices[j]; avg_strain += vec3_length(displacements[node_idx]); }
        avg_strain /= 4.0f; strains[i] = avg_strain;
    }
}

float StaticAnalysis::getMaxVonMisesStress() const { if (von_mises_stresses.empty()) return 0.0f; return *std::max_element(von_mises_stresses.begin(), von_mises_stresses.end()); }
float StaticAnalysis::getMinVonMisesStress() const { if (von_mises_stresses.empty()) return 0.0f; return *std::min_element(von_mises_stresses.begin(), von_mises_stresses.end()); }
Vec3 StaticAnalysis::getMaxDisplacement() const { if (displacements.empty()) return VEC3_ZERO; Vec3 max_disp = displacements[0]; for (const auto& disp : displacements) { float len = vec3_length(disp); if (len > vec3_length(max_disp)) max_disp = disp; } return max_disp; }
Vec3 StaticAnalysis::getMinDisplacement() const { if (displacements.empty()) return VEC3_ZERO; Vec3 min_disp = displacements[0]; for (const auto& disp : displacements) { float len = vec3_length(disp); if (len < vec3_length(min_disp)) min_disp = disp; } return min_disp; }

float StaticAnalysis::computeStrainEnergy() const {
    if (!mesh || von_mises_stresses.empty()) return 0.0f;
    float energy = 0.0f;
    for (size_t i = 0; i < mesh->getElements().size(); i++) {
        const auto& element = mesh->getElements()[i];
        if (element->type == ElementType::TETRAHEDRON) {
            float volume = element->computeVolume(mesh->getNodes());
            energy += 0.5f * von_mises_stresses[i] * strains[i] * volume;
        }
    }
    return energy;
}

void StaticAnalysis::exportResultsToVTK(const std::string& filename) const {
    if (!mesh) return;
    std::ofstream file(filename);
    if (!file.is_open()) return;
    file << "# vtk DataFile Version 2.0\nCAD Engine FEA Results\nASCII\nDATASET UNSTRUCTURED_GRID\n";
    file << "POINTS " << mesh->getNodes().size() << " float\n";
    for (const auto& node : mesh->getNodes()) file << node.x << " " << node.y << " " << node.z << "\n";
    size_t num_tetrahedra = 0; for (const auto& element : mesh->getElements()) if (element->type == ElementType::TETRAHEDRON) num_tetrahedra++;
    file << "\nCELLS " << num_tetrahedra << " " << num_tetrahedra * 5 << "\n";
    for (const auto& element : mesh->getElements()) {
        if (element->type == ElementType::TETRAHEDRON) {
            file << "4 "; for (int index : element->node_indices) file << index << " "; file << "\n";
        }
    }
    file << "\nCELL_TYPES " << num_tetrahedra << "\n";
    for (size_t i = 0; i < num_tetrahedra; i++) file << "10\n";
    file << "\nPOINT_DATA " << mesh->getNodes().size() << "\nSCALARS displacements float 3\nLOOKUP_TABLE default\n";
    for (const auto& disp : displacements) file << disp.x << " " << disp.y << " " << disp.z << "\n";
    file << "\nCELL_DATA " << num_tetrahedra << "\nSCALARS von_mises_stress float 1\nLOOKUP_TABLE default\n";
    for (float stress : von_mises_stresses) file << stress << "\n";
    file.close();
}

std::unique_ptr<StaticAnalysis> StaticAnalysis::clone() const {
    auto new_analysis = std::make_unique<StaticAnalysis>(mesh, material);
    for (const auto& bc : boundary_conditions) new_analysis->boundary_conditions.push_back(bc->clone());
    new_analysis->forces = forces; new_analysis->displacements = displacements;
    new_analysis->von_mises_stresses = von_mises_stresses; new_analysis->strains = strains;
    return new_analysis;
}
