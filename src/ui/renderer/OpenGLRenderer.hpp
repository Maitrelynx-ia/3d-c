#ifndef CAD_OPENGL_RENDERER_HPP
#define CAD_OPENGL_RENDERER_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_6_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include <memory>

class Assembly;
class Part;
class Sketch;

enum class RenderMode;
enum class ViewType;

// --- Rendu OpenGL ---
class OpenGLRenderer : public QOpenGLWidget, protected QOpenGLFunctions_4_6_Core {
    Q_OBJECT

public:
    OpenGLRenderer(QWidget* parent = nullptr);
    ~OpenGLRenderer();
    
    // Gestion de l'assemblage
    void setAssembly(std::shared_ptr<Assembly> assembly);
    std::shared_ptr<Assembly> getAssembly() const { return assembly; }
    
    // Gestion de la pièce active
    void setCurrentPart(std::shared_ptr<Part> part);
    std::shared_ptr<Part> getCurrentPart() const { return current_part; }
    
    // Gestion de l'esquisse active
    void setCurrentSketch(std::shared_ptr<Sketch> sketch);
    std::shared_ptr<Sketch> getCurrentSketch() const { return current_sketch; }
    
    // Mode de rendu
    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() const { return render_mode; }
    
    // Vue
    void setViewType(ViewType type);
    ViewType getViewType() const { return view_type; }
    
    // Zoom
    void zoomIn();
    void zoomOut();
    void zoomFit();
    
    // Sélection
    int getSelectedObjectId() const { return selected_object_id; }
    
signals:
    void selectionChanged();
    void zoomChanged(float factor);
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    
private:
    // Données
    std::shared_ptr<Assembly> assembly;
    std::shared_ptr<Part> current_part;
    std::shared_ptr<Sketch> current_sketch;
    
    // Mode de rendu
    RenderMode render_mode;
    ViewType view_type;
    
    // Caméra
    QMatrix4x4 projection_matrix;
    QMatrix4x4 view_matrix;
    QMatrix4x4 model_matrix;
    
    Vec3 camera_position;
    Vec3 camera_target;
    Vec3 camera_up;
    float camera_fov;
    float camera_near;
    float camera_far;
    
    // Souris
    QPoint last_mouse_position;
    bool is_rotating;
    bool is_panning;
    bool is_selecting;
    
    // Sélection
    int selected_object_id;
    
    // Shaders
    QOpenGLShaderProgram* wireframe_shader;
    QOpenGLShaderProgram* shaded_shader;
    QOpenGLShaderProgram* selection_shader;
    
    // Buffers
    QOpenGLBuffer* vertex_buffer;
    QOpenGLBuffer* index_buffer;
    QOpenGLVertexArrayObject* vao;
    
    // Méthodes
    void setupShaders();
    void setupBuffers();
    void updateMatrices();
    void updateCamera();
    
    void renderAssembly();
    void renderPart(Part* part);
    void renderSketch(Sketch* sketch);
    void renderWireframe();
    void renderShaded();
    void renderTransparent();
    void renderEdgesOnly();
    
    int pickObject(int x, int y);
    
    void extractGeometry();
    
    // Vues standard
    void setIsometricView();
    void setFrontView();
    void setTopView();
    void setRightView();
    void setBackView();
    void setBottomView();
    void setLeftView();
};

#endif // CAD_OPENGL_RENDERER_HPP

