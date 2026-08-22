#include "View2D.hpp"
#include "../modeling/part/Part.hpp"
#include <QPainter>
#include <QPdfWriter>

// --- View2D ---
View2D::View2D(View2DType type, Part* part, float scale) 
    : type(type), part(part), scale(scale), offset(VEC3_ZERO), center(VEC3_ZERO) {}

View2D::~View2D() = default;

void View2D::generate() {
    if (!part) return;
    
    AABB aabb = part->computeAABB();
    center = aabb_center(aabb);
    
    switch (type) {
        case View2DType::FRONT:
        case View2DType::BACK: {
            for (const auto& feature : part->getFeatures()) {
                if (feature->shape) {
                }
            }
            break;
        }
        case View2DType::TOP:
        case View2DType::BOTTOM: {
            for (const auto& feature : part->getFeatures()) {
                if (feature->shape) {
                }
            }
            break;
        }
        case View2DType::RIGHT:
        case View2DType::LEFT: {
            for (const auto& feature : part->getFeatures()) {
                if (feature->shape) {
                }
            }
            break;
        }
        default:
            break;
    }
}

void View2D::render(QPainter* painter, const QRectF& rect) {
    if (!part) return;
    
    painter->save();
    
    painter->translate(rect.center());
    painter->scale(scale, scale);
    painter->translate(-center.x, -center.y);
    
    painter->setPen(QPen(Qt::gray, 1, Qt::DotLine));
    for (const auto& edge : hidden_edges) {
        painter->drawLine(QPointF(edge.first.x, edge.first.y), QPointF(edge.second.x, edge.second.y));
    }
    
    painter->setPen(QPen(Qt::black, 2));
    for (const auto& edge : visible_edges) {
        painter->drawLine(QPointF(edge.first.x, edge.first.y), QPointF(edge.second.x, edge.second.y));
    }
    
    painter->setPen(QPen(Qt::black, 3));
    if (silhouette.size() > 1) {
        painter->drawPolyline(&silhouette[0].x, &silhouette[0].y, silhouette.size());
    }
    
    painter->restore();
}

// --- Dimension ---
Dimension::Dimension(Type type, const Vec3& start, const Vec3& end, float value, const std::string& text)
    : type(type), start(start), end(end), value(value), text(text), is_leader(false) {
    if (this->text.empty()) {
        this->text = std::to_string(value);
    }
    text_position = vec3_mul(vec3_add(start, end), 0.5f);
}

Dimension::~Dimension() = default;

void Dimension::render(QPainter* painter) {
    painter->save();
    painter->setPen(QPen(Qt::black, 1));
    
    switch (type) {
        case HORIZONTAL: {
            Vec3 start_proj = start;
            Vec3 end_proj = end;
            start_proj.y = text_position.y;
            end_proj.y = text_position.y;
            painter->drawLine(QPointF(start.x, start.y), QPointF(start_proj.x, start_proj.y));
            painter->drawLine(QPointF(end.x, end.y), QPointF(end_proj.x, end_proj.y));
            painter->drawLine(QPointF(start_proj.x, start_proj.y), QPointF(end_proj.x, end_proj.y));
            break;
        }
        case VERTICAL: {
            Vec3 start_proj = start;
            Vec3 end_proj = end;
            start_proj.x = text_position.x;
            end_proj.x = text_position.x;
            painter->drawLine(QPointF(start.x, start.y), QPointF(start_proj.x, start_proj.y));
            painter->drawLine(QPointF(end.x, end.y), QPointF(end_proj.x, end_proj.y));
            painter->drawLine(QPointF(start_proj.x, start_proj.y), QPointF(end_proj.x, end_proj.y));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QPointF(text_position.x - 30, text_position.y), QString::fromStdString(text));
            break;
        }
        case RADIAL: {
            Vec3 center = text_position;
            painter->drawLine(QPointF(center.x, center.y), QPointF(start.x, start.y));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QPointF(text_position.x + 10, text_position.y), QString::fromStdString("R" + text));
            break;
        }
        case DIAMETRAL: {
            Vec3 center = text_position;
            painter->drawLine(QPointF(start.x, start.y), QPointF(end.x, end.y));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QPointF(text_position.x, text_position.y + 20), QString::fromStdString("Φ" + text));
            break;
        }
        default:
            break;
    }
    painter->restore();
}

void Dimension::drawArrow(QPainter* painter, const QPointF& start, const QPointF& end, bool at_end) {
    QPointF p1, p2;
    if (at_end) {
        p1 = end;
        p2 = start;
    } else {
        p1 = start;
        p2 = end;
    }
    QVector<QPointF> arrow;
    arrow << p1;
    QPointF dir = p2 - p1;
    dir = dir / sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    QPointF perp(-dir.y(), dir.x());
    arrow << p1 + dir * 10 + perp * 5;
    arrow << p1 + dir * 10 - perp * 5;
    arrow << p1;
    painter->drawPolyline(arrow);
}

// --- Drawing ---
Drawing::Drawing(const std::string& name, Part* part) 
    : name(name), part(part) {}

Drawing::~Drawing() = default;

View2D* Drawing::addView(View2DType type, float scale) {
    auto view = std::make_unique<View2D>(type, part, scale);
    View2D* ptr = view.get();
    views.push_back(std::move(view));
    return ptr;
}

void Drawing::removeView(View2D* view) {
    auto it = std::remove_if(views.begin(), views.end(), 
        [view](const std::unique_ptr<View2D>& v) { return v.get() == view; });
    views.erase(it, views.end());
}

Dimension* Drawing::addDimension(Dimension::Type type, const Vec3& start, const Vec3& end, float value) {
    auto dimension = std::make_unique<Dimension>(type, start, end, value);
    Dimension* ptr = dimension.get();
    dimensions.push_back(std::move(dimension));
    return ptr;
}

void Drawing::removeDimension(Dimension* dimension) {
    auto it = std::remove_if(dimensions.begin(), dimensions.end(), 
        [dimension](const std::unique_ptr<Dimension>& d) { return d.get() == dimension; });
    dimensions.erase(it, dimensions.end());
}

void Drawing::exportToPDF(const std::string& filename) {
    QPdfWriter writer(QString::fromStdString(filename));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(50, 50, 50, 50));
    QPainter painter(&writer);
    float y = 50;
    for (auto& view : views) {
        QRectF rect(50, y, 200, 200);
        view->render(&painter, rect);
        y += 250;
    }
    for (auto& dimension : dimensions) {
        dimension->render(&painter);
    }
    painter.end();
}

void Drawing::render(QPainter* painter, const QRectF& rect) {
    float y = rect.top();
    float view_height = rect.height() / views.size();
    for (auto& view : views) {
        QRectF view_rect(rect.left(), y, rect.width(), view_height);
        view->render(painter, view_rect);
        y += view_height;
    }
    for (auto& dimension : dimensions) {
        dimension->render(painter);
    }
}

std::unique_ptr<Drawing> Drawing::clone() const {
    auto new_drawing = std::make_unique<Drawing>(name, part);
    for (const auto& view : views) {
        new_drawing->views.push_back(view->clone());
    }
    for (const auto& dimension : dimensions) {
        new_drawing->dimensions.push_back(dimension->clone());
    }
    return new_drawing;
}