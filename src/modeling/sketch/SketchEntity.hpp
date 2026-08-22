#ifndef CAD_SKETCH_ENTITY_HPP
#define CAD_SKETCH_ENTITY_HPP

#include <vector>
#include <memory>
#include "../core/geometry/vec3.h"
#include "../core/geometry/aabb.h"

class Sketch;

// --- Types d'entités d'esquisse ---
enum class SketchEntityType {
    LINE,
    CIRCLE,
    ARC,
    SPLINE,
    POINT,
    ELLIPSE
};

// --- Classe de base pour une entité d'esquisse ---
class SketchEntity {
public:
    SketchEntityType type;
    bool is_construction;
    bool is_fixed;
    
    SketchEntity(SketchEntityType type, bool is_construction = false, bool is_fixed = false)
        : type(type), is_construction(is_construction), is_fixed(is_fixed) {}
    
    virtual ~SketchEntity() = default;
    
    virtual void translate(const Vec3& delta) = 0;
    virtual void rotate(const Vec3& axis, float angle) = 0;
    virtual void scale(const Vec3& scale) = 0;
    virtual void mirror(const Vec3& normal) = 0;
    
    virtual AABB getAABB() const = 0;
    virtual Vec3 getStartPoint() const = 0;
    virtual Vec3 getEndPoint() const = 0;
    virtual Vec3 getCenter() const = 0;
    
    virtual bool canBeConstrained() const { return true; }
    virtual std::unique_ptr<SketchEntity> clone() const = 0;
    virtual void serialize(class cJSON* json) const = 0;
    static std::unique_ptr<SketchEntity> deserialize(class cJSON* json);
};

class SketchLine : public SketchEntity {
public:
    Vec3 start, end;
    SketchLine(const Vec3& start, const Vec3& end, bool is_construction = false)
        : SketchEntity(SketchEntityType::LINE, is_construction), start(start), end(end) {}
    void translate(const Vec3& delta) override {
        if (!is_fixed) { start = vec3_add(start, delta); end = vec3_add(end, delta); }
    }
    void rotate(const Vec3& axis, float angle) override {
        if (!is_fixed) {
            Vec3 center = vec3_mul(vec3_add(start, end), 0.5f);
            Vec3 offset_start = vec3_sub(start, center);
            Vec3 offset_end = vec3_sub(end, center);
            float c = cosf(angle); float s = sinf(angle);
            Vec3 axis_norm = vec3_normalize(axis);
            Vec3 cross_start = vec3_cross(axis_norm, offset_start);
            Vec3 rotated_start = vec3_add(vec3_add(vec3_mul(offset_start, c), vec3_mul(cross_start, s)), vec3_mul(vec3_cross(axis_norm, cross_start), 1.0f - c));
            Vec3 cross_end = vec3_cross(axis_norm, offset_end);
            Vec3 rotated_end = vec3_add(vec3_add(vec3_mul(offset_end, c), vec3_mul(cross_end, s)), vec3_mul(vec3_cross(axis_norm, cross_end), 1.0f - c));
            start = vec3_add(center, rotated_start); end = vec3_add(center, rotated_end);
        }
    }
    void scale(const Vec3& scale) override { if (!is_fixed) { start = vec3_mul(start, scale); end = vec3_mul(end, scale); } }
    void mirror(const Vec3& normal) override {
        if (!is_fixed) {
            Vec3 center = vec3_mul(vec3_add(start, end), 0.5f);
            start = vec3_add(center, vec3_negate(vec3_sub(start, center)));
            end = vec3_add(center, vec3_negate(vec3_sub(end, center)));
        }
    }
    AABB getAABB() const override { return aabb_from_points(&start, 2); }
    Vec3 getStartPoint() const override { return start; }
    Vec3 getEndPoint() const override { return end; }
    Vec3 getCenter() const override { return vec3_mul(vec3_add(start, end), 0.5f); }
    float getLength() const { return vec3_distance(start, end); }
    std::unique_ptr<SketchEntity> clone() const override { return std::make_unique<SketchLine>(start, end, is_construction); }
    void serialize(cJSON* json) const override;
};

class SketchCircle : public SketchEntity {
public:
    Vec3 center; float radius;
    SketchCircle(const Vec3& center, float radius, bool is_construction = false)
        : SketchEntity(SketchEntityType::CIRCLE, is_construction), center(center), radius(radius) {}
    void translate(const Vec3& delta) override { if (!is_fixed) center = vec3_add(center, delta); }
    void rotate(const Vec3& axis, float angle) override {
        if (!is_fixed) {
            float c = cosf(angle); float s = sinf(angle);
            Vec3 axis_norm = vec3_normalize(axis); Vec3 cross = vec3_cross(axis_norm, center);
            center = vec3_add(vec3_add(vec3_mul(center, c), vec3_mul(cross, s)), vec3_mul(vec3_cross(axis_norm, cross), 1.0f - c));
        }
    }
    void scale(const Vec3& scale) override { if (!is_fixed) { center = vec3_mul(center, scale); radius *= scale.x; } }
    void mirror(const Vec3& normal) override {
        if (!is_fixed) center = vec3_add(center, vec3_negate(vec3_sub(center, vec3_mul(normal, vec3_dot(center, normal)))));
    }
    AABB getAABB() const override { return (AABB){{center.x - radius, center.y - radius, center.z - radius}, {center.x + radius, center.y + radius, center.z + radius}}; }
    Vec3 getStartPoint() const override { return center; }
    Vec3 getEndPoint() const override { return center; }
    Vec3 getCenter() const override { return center; }
    float getCircumference() const { return 2.0f * M_PI * radius; }
    std::unique_ptr<SketchEntity> clone() const override { return std::make_unique<SketchCircle>(center, radius, is_construction); }
    void serialize(cJSON* json) const override;
};

class SketchArc : public SketchEntity {
public:
    Vec3 center; float radius; float start_angle; float end_angle;
    SketchArc(const Vec3& center, float radius, float start_angle, float end_angle, bool is_construction = false)
        : SketchEntity(SketchEntityType::ARC, is_construction), center(center), radius(radius), start_angle(start_angle), end_angle(end_angle) {}
    void translate(const Vec3& delta) override { if (!is_fixed) center = vec3_add(center, delta); }
    void rotate(const Vec3& axis, float angle) override {
        if (!is_fixed) {
            start_angle += angle; end_angle += angle;
            while (start_angle < 0) start_angle += 2 * M_PI; while (start_angle >= 2 * M_PI) start_angle -= 2 * M_PI;
            while (end_angle < 0) end_angle += 2 * M_PI; while (end_angle >= 2 * M_PI) end_angle -= 2 * M_PI;
            float c = cosf(angle); float s = sinf(angle); Vec3 axis_norm = vec3_normalize(axis); Vec3 cross = vec3_cross(axis_norm, center);
            center = vec3_add(vec3_add(vec3_mul(center, c), vec3_mul(cross, s)), vec3_mul(vec3_cross(axis_norm, cross), 1.0f - c));
        }
    }
    void scale(const Vec3& scale) override { if (!is_fixed) { center = vec3_mul(center, scale); radius *= scale.x; } }
    void mirror(const Vec3& normal) override {
        if (!is_fixed) {
            center = vec3_add(center, vec3_negate(vec3_sub(center, vec3_mul(normal, vec3_dot(center, normal)))));
            float temp = start_angle; start_angle = 2 * M_PI - end_angle; end_angle = 2 * M_PI - temp;
        }
    }
    AABB getAABB() const override {
        float min_x = center.x - radius, max_x = center.x + radius, min_y = center.y - radius, max_y = center.y + radius, min_z = center.z - radius, max_z = center.z + radius;
        float angles[4] = {start_angle, end_angle, start_angle + M_PI, end_angle + M_PI};
        for (int i = 0; i < 4; i++) {
            float angle = angles[i]; float x = center.x + radius * cosf(angle); float y = center.y + radius * sinf(angle);
            min_x = fminf(min_x, x); max_x = fmaxf(max_x, x); min_y = fminf(min_y, y); max_y = fmaxf(max_y, y);
        }
        return (AABB){{min_x, min_y, min_z}, {max_x, max_y, max_z}};
    }
    Vec3 getStartPoint() const override { return (Vec3){center.x + radius * cosf(start_angle), center.y + radius * sinf(start_angle), center.z}; }
    Vec3 getEndPoint() const override { return (Vec3){center.x + radius * cosf(end_angle), center.y + radius * sinf(end_angle), center.z}; }
    Vec3 getCenter() const override { return center; }
    float getLength() const { return radius * fabsf(end_angle - start_angle); }
    std::unique_ptr<SketchEntity> clone() const override { return std::make_unique<SketchArc>(center, radius, start_angle, end_angle, is_construction); }
    void serialize(cJSON* json) const override;
};

class SketchPoint : public SketchEntity {
public:
    Vec3 position;
    SketchPoint(const Vec3& position, bool is_construction = false) : SketchEntity(SketchEntityType::POINT, is_construction), position(position) {}
    void translate(const Vec3& delta) override { if (!is_fixed) position = vec3_add(position, delta); }
    void rotate(const Vec3& axis, float angle) override {
        if (!is_fixed) {
            float c = cosf(angle); float s = sinf(angle); Vec3 axis_norm = vec3_normalize(axis); Vec3 cross = vec3_cross(axis_norm, position);
            position = vec3_add(vec3_add(vec3_mul(position, c), vec3_mul(cross, s)), vec3_mul(vec3_cross(axis_norm, cross), 1.0f - c));
        }
    }
    void scale(const Vec3& scale) override { if (!is_fixed) position = vec3_mul(position, scale); }
    void mirror(const Vec3& normal) override {
        if (!is_fixed) position = vec3_add(position, vec3_negate(vec3_sub(position, vec3_mul(normal, vec3_dot(position, normal)))));
    }
    AABB getAABB() const override { return aabb_from_point(position); }
    Vec3 getStartPoint() const override { return position; }
    Vec3 getEndPoint() const override { return position; }
    Vec3 getCenter() const override { return position; }
    std::unique_ptr<SketchEntity> clone() const override { return std::make_unique<SketchPoint>(position, is_construction); }
    void serialize(cJSON* json) const override;
};

#endif