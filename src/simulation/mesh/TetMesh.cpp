#include "TetMesh.hpp"
#include "../core/geometry/brep.h"
#include "../core/geometry/vec3.h"
#include <algorithm>
#include <fstream>

float MeshElement::computeVolume(const std::vector<Vec3>& nodes) const {
    switch (type) {
        case ElementType::TETRAHEDRON: {
            if (node_indices.size() < 4) return 0.0f;
            Vec3 v0 = nodes[node_indices[0]];
            Vec3 v1 = nodes[node_indices[1]];
            Vec3 v2 = nodes[node_indices[2]];
            Vec3 v3 = nodes[node_indices[3]];
            Vec3 edge1 = vec3_sub(v1, v0);
            Vec3 edge2 = vec3_sub(v2, v0);
            Vec3 edge3 = vec3_sub(v3, v0);
            float volume = fabsf(vec3_dot(edge1, vec3_cross(edge2, edge3))) / 6.0f;
            return volume;
        }
        case ElementType::HEXAHEDRON: {
            if (node_indices.size() < 8) return 0.0f;
            return 0.0f;
        }
        default: return 0.0f;
    }
}

Vec3 MeshElement::computeCentroid(const std::vector<Vec3>& nodes) const {
    Vec3 centroid = VEC3_ZERO;
    for (int index : node_indices) centroid = vec3_add(centroid, nodes[index]);
    return vec3_div(centroid, (float)node_indices.size());
}

float TetrahedronElement::computeVolume(const std::vector<Vec3>& nodes) const {
    if (node_indices.size() < 4) return 0.0f;
    Vec3 v0 = nodes[node_indices[0]];
    Vec3 v1 = nodes[node_indices[1]];
    Vec3 v2 = nodes[node_indices[2]];
    Vec3 v3 = nodes[node_indices[3]];
    Vec3 edge1 = vec3_sub(v1, v0);
    Vec3 edge2 = vec3_sub(v2, v0);
    Vec3 edge3 = vec3_sub(v3, v0);
    float volume = fabsf(vec3_dot(edge1, vec3_cross(edge2, edge3))) / 6.0f;
    return volume;
}

TetMesh::TetMesh(const TopoShape* shape, float element_size)
    : bounds(AABB_EMPTY), element_size(element_size), is_generated(false) {
    if (shape) bounds = topo_shape_get_aabb(shape);
}

TetMesh::TetMesh(const AABB& bounds, float element_size)
    : bounds(bounds), element_size(element_size), is_generated(false) {}

TetMesh::~TetMesh() { clear(); }

void TetMesh::clear() { nodes.clear(); elements.clear(); is_generated = false; }

void TetMesh::generate() {
    if (bounds.min.x == INFINITY) { is_generated = true; return; }
    Vec3 size = aabb_size(bounds);
    Vec3 min = bounds.min;
    int nx = (int)ceilf(size.x / element_size);
    int ny = (int)ceilf(size.y / element_size);
    int nz = (int)ceilf(size.z / element_size);
    nodes.reserve((nx + 1) * (ny + 1) * (nz + 1));
    for (int z = 0; z <= nz; z++) {
        for (int y = 0; y <= ny; y++) {
            for (int x = 0; x <= nx; x++) {
                float px = min.x + (float)x * size.x / nx;
                float py = min.y + (float)y * size.y / ny;
                float pz = min.z + (float)z * size.z / nz;
                nodes.push_back((Vec3){px, py, pz});
            }
        }
    }
    for (int z = 0; z < nz; z++) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                int i000 = x + y * (nx + 1) + z * (nx + 1) * (ny + 1);
                int i100 = (x + 1) + y * (nx + 1) + z * (nx + 1) * (ny + 1);
                int i010 = x + (y + 1) * (nx + 1) + z * (nx + 1) * (ny + 1);
                int i110 = (x + 1) + (y + 1) * (nx + 1) + z * (nx + 1) * (ny + 1);
                int i001 = x + y * (nx + 1) + (z + 1) * (nx + 1) * (ny + 1);
                int i101 = (x + 1) + y * (nx + 1) + (z + 1) * (nx + 1) * (ny + 1);
                int i011 = x + (y + 1) * (nx + 1) + (z + 1) * (nx + 1) * (ny + 1);
                int i111 = (x + 1) + (y + 1) * (nx + 1) + (z + 1) * (nx + 1) * (ny + 1);
                elements.push_back(std::make_unique<TetrahedronElement>(i000, i100, i010, i001));
                elements.push_back(std::make_unique<TetrahedronElement>(i100, i110, i010, i001));
                elements.push_back(std::make_unique<TetrahedronElement>(i100, i110, i101, i001));
                elements.push_back(std::make_unique<TetrahedronElement>(i110, i010, i001, i101));
                elements.push_back(std::make_unique<TetrahedronElement>(i110, i101, i001, i111));
                elements.push_back(std::make_unique<TetrahedronElement>(i110, i111, i101, i001));
            }
        }
    }
    is_generated = true;
}

const MeshElement* TetMesh::findElementContainingPoint(const Vec3& point) const {
    for (const auto& element : elements) {
        if (element->type == ElementType::TETRAHEDRON && element->node_indices.size() >= 4) {
            Vec3 v0 = nodes[element->node_indices[0]];
            Vec3 v1 = nodes[element->node_indices[1]];
            Vec3 v2 = nodes[element->node_indices[2]];
            Vec3 v3 = nodes[element->node_indices[3]];
            Vec3 edge1 = vec3_sub(v1, v0);
            Vec3 edge2 = vec3_sub(v2, v0);
            Vec3 edge3 = vec3_sub(v3, v0);
            Vec3 vec = vec3_sub(point, v0);
            float det = vec3_dot(edge1, vec3_cross(edge2, edge3));
            if (fabsf(det) < 1e-10f) continue;
            float u = vec3_dot(vec, vec3_cross(edge2, edge3)) / det;
            float v = vec3_dot(edge1, vec3_cross(vec, edge3)) / det;
            float w = vec3_dot(edge1, vec3_cross(edge2, vec)) / det;
            float t = 1.0f - u - v - w;
            if (u >= 0 && v >= 0 && w >= 0 && t >= 0) return element.get();
        }
    }
    return nullptr;
}

const std::vector<int> TetMesh::findElementsInRegion(const AABB& region) const {
    std::vector<int> result;
    for (size_t i = 0; i < elements.size(); i++) {
        Vec3 centroid = elements[i]->computeCentroid(nodes);
        if (aabb_contains_point(region, centroid)) result.push_back((int)i);
    }
    return result;
}

float TetMesh::computeAverageElementQuality() const {
    if (elements.empty()) return 0.0f;
    float total_quality = 0.0f;
    for (const auto& element : elements) {
        if (element->type == ElementType::TETRAHEDRON) total_quality += element->computeVolume(nodes);
    }
    return total_quality / elements.size();
}

float TetMesh::computeMinElementQuality() const {
    if (elements.empty()) return 0.0f;
    float min_quality = INFINITY;
    for (const auto& element : elements) {
        if (element->type == ElementType::TETRAHEDRON) {
            float volume = element->computeVolume(nodes);
            if (volume < min_quality) min_quality = volume;
        }
    }
    return min_quality;
}

float TetMesh::computeMaxElementQuality() const {
    if (elements.empty()) return 0.0f;
    float max_quality = 0.0f;
    for (const auto& element : elements) {
        if (element->type == ElementType::TETRAHEDRON) {
            float volume = element->computeVolume(nodes);
            if (volume > max_quality) max_quality = volume;
        }
    }
    return max_quality;
}

void TetMesh::exportToVTK(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    file << "# vtk DataFile Version 2.0\nCAD Engine TetMesh Export\nASCII\nDATASET UNSTRUCTURED_GRID\n";
    file << "POINTS " << nodes.size() << " float\n";
    for (const auto& node : nodes) file << node.x << " " << node.y << " " << node.z << "\n";
    size_t num_tetrahedra = 0;
    for (const auto& element : elements) if (element->type == ElementType::TETRAHEDRON) num_tetrahedra++;
    file << "\nCELLS " << num_tetrahedra << " " << num_tetrahedra * 5 << "\n";
    for (const auto& element : elements) {
        if (element->type == ElementType::TETRAHEDRON) {
            file << "4 ";
            for (int index : element->node_indices) file << index << " ";
            file << "\n";
        }
    }
    file << "\nCELL_TYPES " << num_tetrahedra << "\n";
    for (size_t i = 0; i < num_tetrahedra; i++) file << "10\n";
    file.close();
}

std::unique_ptr<TetMesh> TetMesh::clone() const {
    auto new_mesh = std::make_unique<TetMesh>(bounds, element_size);
    new_mesh->nodes = nodes;
    for (const auto& element : elements) {
        if (element->type == ElementType::TETRAHEDRON && element->node_indices.size() >= 4) {
            new_mesh->elements.push_back(std::make_unique<TetrahedronElement>(
                element->node_indices[0], element->node_indices[1],
                element->node_indices[2], element->node_indices[3]
            ));
        }
    }
    new_mesh->is_generated = is_generated;
    return new_mesh;
}