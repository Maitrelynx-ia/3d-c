#ifndef CAD_BREP_H
#define CAD_BREP_H

#include "vec3.h"
#include "aabb.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Topology Types ---
typedef enum {
    TOPO_VERTEX,
    TOPO_EDGE,
    TOPO_FACE,
    TOPO_SOLID,
    TOPO_SHELL,
    TOPO_WIRE
} TopoType;

// --- Forward Declarations ---
typedef struct TopoShape TopoShape;
typedef struct VertexData VertexData;
typedef struct EdgeData EdgeData;
typedef struct FaceData FaceData;
typedef struct SolidData SolidData;
typedef struct ShellData ShellData;
typedef struct WireData WireData;

// --- Base Topology Shape ---
struct TopoShape {
    TopoType type;
    void* data;
    TopoShape* parent;
    TopoShape** children;
    int num_children;
    int capacity_children;
    AABB aabb;
    bool aabb_dirty;
};

// --- Vertex Data ---
struct VertexData {
    Vec3 point;
};

// --- Edge Data ---
typedef enum {
    EDGE_LINE,
    EDGE_CIRCLE,
    EDGE_ELLIPSE,
    EDGE_BEZIER,
    EDGE_NURBS
} EdgeType;

struct EdgeData {
    EdgeType type;
    TopoShape* start_vertex;
    TopoShape* end_vertex;
    void* curve;
    float length;
    bool length_dirty;
};

// --- Face Data ---
typedef enum {
    FACE_PLANE,
    FACE_CYLINDER,
    FACE_CONE,
    FACE_SPHERE,
    FACE_NURBS
} FaceType;

struct FaceData {
    FaceType type;
    void* surface;
    TopoShape* outer_wire;
    TopoShape** holes;
    int num_holes;
    Vec3 normal;
    bool normal_dirty;
};

// --- Solid Data ---
struct SolidData {
    TopoShape** shells;
    int num_shells;
    float volume;
    bool volume_dirty;
};

// --- Shell Data ---
struct ShellData {
    TopoShape** faces;
    int num_faces;
    bool is_closed;
};

// --- Wire Data ---
struct WireData {
    TopoShape** edges;
    int num_edges;
    bool is_closed;
};

// --- Curve Types ---
typedef struct {
    Vec3 start, end;
} Line;

typedef struct {
    Vec3 center;
    float radius;
} Circle;

typedef struct {
    Vec3 center;
    Vec3 major_axis;
    Vec3 minor_axis;
} Ellipse;

typedef struct {
    Vec3* control_points;
    int degree;
    int num_points;
} BezierCurve;

// --- Surface Types ---
typedef struct {
    Vec3 origin;
    Vec3 normal;
} Plane;

typedef struct {
    Vec3 center;
    Vec3 axis;
    float radius;
    float height;
} Cylinder;

typedef struct {
    Vec3 center;
    float radius;
} Sphere;

// --- Creation Functions ---
TopoShape* topo_vertex_create(Vec3 point);
TopoShape* topo_edge_create(TopoShape* start, TopoShape* end, EdgeType type, void* curve_data);
TopoShape* topo_face_create(FaceType type, void* surface_data);
TopoShape* topo_solid_create();
TopoShape* topo_shell_create();
TopoShape* topo_wire_create();

// --- Shape Manipulation ---
void topo_shape_add_child(TopoShape* parent, TopoShape* child);
void topo_shape_remove_child(TopoShape* parent, TopoShape* child);
void topo_shape_free(TopoShape* shape);
void topo_shape_free_recursive(TopoShape* shape);

// --- AABB Management ---
void topo_shape_update_aabb(TopoShape* shape);
AABB topo_shape_get_aabb(const TopoShape* shape);

// --- Utility Functions ---
bool topo_shape_is_valid(const TopoShape* shape);
TopoShape* topo_shape_clone(const TopoShape* shape);
TopoShape* topo_shape_transform(const TopoShape* shape, Mat4 transform);

// --- Primitive Creation ---
TopoShape* topo_create_cube(Vec3 center, float size);
TopoShape* topo_create_sphere(Vec3 center, float radius, int stacks, int sectors);
TopoShape* topo_create_cylinder(Vec3 center, Vec3 axis, float radius, float height);
TopoShape* topo_create_cone(Vec3 center, Vec3 axis, float radius, float height);
TopoShape* topo_create_torus(Vec3 center, float major_radius, float minor_radius, int segments);

// --- Type Checking ---
static inline bool topo_shape_is_vertex(const TopoShape* shape) {
    return shape && shape->type == TOPO_VERTEX;
}

static inline bool topo_shape_is_edge(const TopoShape* shape) {
    return shape && shape->type == TOPO_EDGE;
}

static inline bool topo_shape_is_face(const TopoShape* shape) {
    return shape && shape->type == TOPO_FACE;
}

static inline bool topo_shape_is_solid(const TopoShape* shape) {
    return shape && shape->type == TOPO_SOLID;
}

// --- Data Access ---
static inline VertexData* topo_vertex_data(TopoShape* shape) {
    return (VertexData*)shape->data;
}

static inline EdgeData* topo_edge_data(TopoShape* shape) {
    return (EdgeData*)shape->data;
}

static inline FaceData* topo_face_data(TopoShape* shape) {
    return (FaceData*)shape->data;
}

static inline SolidData* topo_solid_data(TopoShape* shape) {
    return (SolidData*)shape->data;
}

#ifdef __cplusplus
}
#endif

#endif // CAD_BREP_H
