#ifndef CAD_CONSTRAINT_HPP
#define CAD_CONSTRAINT_HPP

#include "SketchEntity.hpp"
#include <vector>
#include <memory>

class Sketch;

// --- Types de contraintes ---
enum class ConstraintType {
    COINCIDENT,
    PARALLEL,
    PERPENDICULAR,
    TANGENT,
    DISTANCE,
    ANGLE,
    RADIUS,
    DIAMETER,
    HORIZONTAL,
    VERTICAL,
    MIDPOINT,
    ON_LINE,
    ON_CIRCLE
};

// --- Statut de la contrainte ---
enum class ConstraintStatus {
    UNSOLVED,
    SATISFIED,
    CONFLICT,
    OVER_CONSTRAINED
};

// --- Classe de base pour une contrainte ---
class Constraint {
public:
    ConstraintType type;
    ConstraintStatus status;
    SketchEntity* entity1;
    SketchEntity* entity2;
    float value;
    float current_value;
    float tolerance;
    bool is_reference;
    
    Constraint(ConstraintType type, SketchEntity* entity1, SketchEntity* entity2 = nullptr, float value = 0.0f)
        : type(type), status(ConstraintStatus::UNSOLVED), 
          entity1(entity1), entity2(entity2), value(value), 
          current_value(0.0f), tolerance(1e-6f), is_reference(false) {}
    
    virtual ~Constraint() = default;
    
    virtual float evaluate() const = 0;
    virtual void apply() = 0;
    virtual bool solve() = 0;
    
    virtual std::unique_ptr<Constraint> clone() const = 0;
    
    virtual void serialize(cJSON* json) const = 0;
    static std::unique_ptr<Constraint> deserialize(cJSON* json);
};

class CoincidentConstraint : public Constraint {
public:
    CoincidentConstraint(SketchEntity* entity1, SketchEntity* entity2)
        : Constraint(ConstraintType::COINCIDENT, entity1, entity2) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<CoincidentConstraint>(entity1, entity2); }
    void serialize(cJSON* json) const override;
};

class DistanceConstraint : public Constraint {
public:
    DistanceConstraint(SketchEntity* entity1, SketchEntity* entity2, float distance)
        : Constraint(ConstraintType::DISTANCE, entity1, entity2, distance) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<DistanceConstraint>(entity1, entity2, value); }
    void serialize(cJSON* json) const override;
};

class ParallelConstraint : public Constraint {
public:
    ParallelConstraint(SketchEntity* entity1, SketchEntity* entity2)
        : Constraint(ConstraintType::PARALLEL, entity1, entity2) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<ParallelConstraint>(entity1, entity2); }
    void serialize(cJSON* json) const override;
};

class PerpendicularConstraint : public Constraint {
public:
    PerpendicularConstraint(SketchEntity* entity1, SketchEntity* entity2)
        : Constraint(ConstraintType::PERPENDICULAR, entity1, entity2) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<PerpendicularConstraint>(entity1, entity2); }
    void serialize(cJSON* json) const override;
};

class TangentConstraint : public Constraint {
public:
    TangentConstraint(SketchEntity* entity1, SketchEntity* entity2)
        : Constraint(ConstraintType::TANGENT, entity1, entity2) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<TangentConstraint>(entity1, entity2); }
    void serialize(cJSON* json) const override;
};

class RadiusConstraint : public Constraint {
public:
    RadiusConstraint(SketchEntity* entity, float radius)
        : Constraint(ConstraintType::RADIUS, entity, nullptr, radius) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<RadiusConstraint>(entity1, value); }
    void serialize(cJSON* json) const override;
};

class AngleConstraint : public Constraint {
public:
    AngleConstraint(SketchEntity* entity1, SketchEntity* entity2, float angle)
        : Constraint(ConstraintType::ANGLE, entity1, entity2, angle) {}
    float evaluate() const override;
    void apply() override;
    bool solve() override;
    std::unique_ptr<Constraint> clone() const override { return std::make_unique<AngleConstraint>(entity1, entity2, value); }
    void serialize(cJSON* json) const override;
};

#endif