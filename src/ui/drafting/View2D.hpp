#ifndef CAD_VIEW_2D_HPP
#define CAD_VIEW_2D_HPP

#include "../modeling/part/Part.hpp"
#include "../core/geometry/vec3.h"
#include <vector>
#include <memory>

// --- Types de vues 2D ---
enum class View2DType {
    FRONT,
    TOP,
    RIGHT,
    BACK,
    BOTTOM,
    LEFT,
    SECTION,
    DETAIL
};

// --- Vue 2D ---
class View2D {
public:
    View2DType type;
    Part* part;
    float scale;
    Vec3 offset;
    
    std::vector<Vec3> silhouette;
    
    std::vector<std::pair<Vec3, Vec3>> visible_edges;
    
    std::vector<std::pair<Vec3, Vec3>> hidden_edges;
    
    Vec3 center;
    
    View2D(View2DType type, Part* part, float scale = 1.0f);
    ~View2D();
    
    void generate();
    
    void render(QPainter* painter, const QRectF& rect);
    
    void exportToPDF(QPdfWriter* writer);
    void exportToDXF(class DXFWriter* writer);
    
    std::unique_ptr<View2D> clone() const;
};

// --- Cotation ---
class Dimension {
public:
    enum Type { HORIZONTAL, VERTICAL, ANGULAR, RADIAL, DIAMETRAL };
    
    Type type;
    Vec3 start;
    Vec3 end;
    Vec3 text_position;
    std::string text;
    float value;
    bool is_leader;
    
    Dimension(Type type, const Vec3& start, const Vec3& end, float value, const std::string& text = "");
    ~Dimension();
    
    void render(QPainter* painter);
    void exportToPDF(QPdfWriter* writer);
    void exportToDXF(class DXFWriter* writer);
    
    std::unique_ptr<Dimension> clone() const;
};

// --- Mise en plan ---
class Drawing {
private:
    std::string name;
    std::vector<std::unique_ptr<View2D>> views;
    std::vector<std::unique_ptr<Dimension>> dimensions;
    Part* part;
    
public:
    Drawing(const std::string& name, Part* part);
    ~Drawing();
    
    View2D* addView(View2DType type, float scale = 1.0f);
    void removeView(View2D* view);
    const std::vector<std::unique_ptr<View2D>>& getViews() const { return views; }
    
    Dimension* addDimension(Dimension::Type type, const Vec3& start, const Vec3& end, float value);
    void removeDimension(Dimension* dimension);
    const std::vector<std::unique_ptr<Dimension>>& getDimensions() const { return dimensions; }
    
    void exportToPDF(const std::string& filename);
    void exportToDXF(const std::string& filename);
    
    void render(QPainter* painter, const QRectF& rect);
    
    std::unique_ptr<Drawing> clone() const;
};

#endif // CAD_VIEW_2D_HPP
