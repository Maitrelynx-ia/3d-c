#ifndef CAD_TET_MESH_HPP
#define CAD_TET_MESH_HPP

#include "../core/geometry/brep.h"
#include "../core/geometry/vec3.h"
#include "../core/geometry/aabb.h"
#include <vector>
#include <memory>

enum class ElementType {
    TETRAHEDRON,
    HEXAHEDRON,
    TRIANGLE,
    QUAD,
    LINE,
    EDGE
};

class MeshElement {
public:
    ElementType type;
    std::vector<int> node_indices;
    MeshElement(ElementType type, const std::vector<int>& node_indices)
        : type(type), node_indices(node_indices) {}
    virtual ~MeshElement() = default;
    virtual float computeVolume(const std::vector<Vec3>& nodes) const;
    Vec3 computeCentroid(const std::vector<Vec3>& nodes) const;
};

class TetrahedronElement : public MeshElement {
public:
    TetrahedronElement(int n0, int n1, int n2, int n3)
        : MeshElement(ElementType::TETRAHEDRON, {n0, n1, n2, n3}) {}
    float computeVolume(const std::vector<Vec3>& nodes) const override;
};

class HexahedronElement : public MeshElement {
public:
    HexahedronElement(int n0, int n1, int n2, int n3, int n4, int n5, int n6, int n7)
        : MeshElement(ElementType::HEXAHEDRON, {n0, n1, n2, n3, n4, n5, n6, n7}) {}
    float computeVolume(const std::vector<Vec3>& nodes) const override;
};

class TetMesh {
private:
    std::vector<Vec3> nodes;
    std::vector<std::unique_ptr<MeshElement>> elements;
    AABB bounds;
    float element_size;
    bool is_generated;
    void generateFromBRep(const TopoShape* shape);
    void generateFromAABB(const AABB& aabb);
public:
    TetMesh(const TopoShape* shape, float element_size = 1.0f);
    TetMesh(const AABB& bounds, float element_size = 1.0f);
    ~TetMesh();
    void generate();
    void clear();
    const std::vector<Vec3>& getNodes() const { return nodes; }
    const std::vector<std::unique_ptr<MeshElement>>& getElements() const { return elements; }
    const AABB& getBounds() const { return bounds; }
    float getElementSize() const { return element_size; }
    bool isGenerated() const { return is_generated; }
    size_t getNodeCount() const { return nodes.size(); }
    size_t getElementCount() const { return elements.size(); }
    const MeshElement* findElementContainingPoint(const Vec3& point) const;
    const std::vector<int> findElementsInRegion(const AABB& region) const;
    float computeAverageElementQuality() const;
    float computeMinElementQuality() const;
    float computeMaxElementQuality() const;
    void exportToVTK(const std::string& filename) const;
    void exportToMSH(const std::string& filename) const;
    std::unique_ptr<TetMesh> clone() const;
};

#endif // CAD_TET_MESH_HPP
