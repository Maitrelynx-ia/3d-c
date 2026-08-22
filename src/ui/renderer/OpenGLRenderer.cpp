#include "OpenGLRenderer.hpp"
#include "../modeling/assembly/Assembly.hpp"
#include "../modeling/part/Part.hpp"
#include "../modeling/sketch/Sketch.hpp"
#include "../core/geometry/brep.h"
#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QDebug>

// --- OpenGLRenderer ---
OpenGLRenderer::OpenGLRenderer(QWidget* parent) 
    : QOpenGLWidget(parent),
      assembly(nullptr), current_part(nullptr), current_sketch(nullptr),
      render_mode(RenderMode::SHADED), view_type(ViewType::ISOMETRIC),
      camera_position(0, 0, 50), camera_target(0, 0, 0), camera_up(0, 1, 0),
      camera_fov(45.0f), camera_near(0.1f), camera_far(1000.0f),
      is_rotating(false), is_panning(false), is_selecting(false),
      selected_object_id(-1),
      wireframe_shader(nullptr), shaded_shader(nullptr), selection_shader(nullptr),
      vertex_buffer(nullptr), index_buffer(nullptr), vao(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

OpenGLRenderer::~OpenGLRenderer() {
    makeCurrent();
    
    delete wireframe_shader;
    delete shaded_shader;
    delete selection_shader;
    delete vertex_buffer;
    delete index_buffer;
    delete vao;
    
    doneCurrent();
}

// --- Initialisation OpenGL ---
void OpenGLRenderer::initializeGL() {
    initializeOpenGLFunctions();
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    setupShaders();
    setupBuffers();
    updateMatrices();
}

// --- Redimensionnement ---
void OpenGLRenderer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    
    projection_matrix.setToIdentity();
    projection_matrix.perspective(camera_fov, (float)w / h, camera_near, camera_far);
    
    updateMatrices();
}

// --- Rendu ---
void OpenGLRenderer::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!assembly) {
        return;
    }
    
    updateMatrices();
    
    switch (render_mode) {
        case RenderMode::WIREFRAME:
            renderWireframe();
            break;
        case RenderMode::SHADED:
            renderShaded();
            break;
        case RenderMode::TRANSPARENT:
            renderTransparent();
            break;
        case RenderMode::EDGES_ONLY:
            renderEdgesOnly();
            break;
    }
    
    // Rendre l'esquisse active
    if (current_sketch) {
        renderSketch(current_sketch.get());
    }
}

// --- Configuration des shaders ---
void OpenGLRenderer::setupShaders() {
    // Shader Wireframe
    wireframe_shader = new QOpenGLShaderProgram();
    wireframe_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/wireframe.vert");
    wireframe_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/wireframe.frag");
    wireframe_shader->link();
    
    // Shader Shaded
    shaded_shader = new QOpenGLShaderProgram();
    shaded_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shaded.vert");
    shaded_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shaded.frag");
    shaded_shader->link();
    
    // Shader Selection
    selection_shader = new QOpenGLShaderProgram();
    selection_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/selection.vert");
    selection_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/selection.frag");
    selection_shader->link();
}

// --- Configuration des buffers ---
void OpenGLRenderer::setupBuffers() {
    vao = new QOpenGLVertexArrayObject();
    vao->create();
    vao->bind();
    
    vertex_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertex_buffer->create();
    vertex_buffer->bind();
    vertex_buffer->allocate(0);
    
    index_buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    index_buffer->create();
    index_buffer->bind();
    index_buffer->allocate(0);
    
    vao->release();
}

// --- Mise a jour des matrices ---
void OpenGLRenderer::updateMatrices() {
    view_matrix.setToIdentity();
    view_matrix.lookAt(camera_position, camera_target, camera_up);
    
    model_matrix.setToIdentity();
}

// --- Mise a jour de la camera ---
void OpenGLRenderer::updateCamera() {
    switch (view_type) {
        case ViewType::ISOMETRIC:
            setIsometricView();
            break;
        case ViewType::FRONT:
            setFrontView();
            break;
        case ViewType::TOP:
            setTopView();
            break;
        case ViewType::RIGHT:
            setRightView();
            break;
        case ViewType::BACK:
            setBackView();
            break;
        case ViewType::BOTTOM:
            setBottomView();
            break;
        case ViewType::LEFT:
            setLeftView();
            break;
    }
    
    updateMatrices();
    update();
}

// --- Rendu de l'assemblage ---
void OpenGLRenderer::renderAssembly() {
    if (!assembly) return;
    
    for (Part* part : assembly->getParts()) {
        renderPart(part);
    }
}

void OpenGLRenderer::renderPart(Part* part) {
    if (!part || !part->getBody()) return;
    
    extractGeometry();
    
    switch (render_mode) {
        case RenderMode::WIREFRAME:
            wireframe_shader->bind();
            break;
        case RenderMode::SHADED:
            shaded_shader->bind();
            break;
        case RenderMode::TRANSPARENT:
            shaded_shader->bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case RenderMode::EDGES_ONLY:
            wireframe_shader->bind();
            break;
    }
    
    wireframe_shader->setUniformValue("projection_matrix", projection_matrix);
    wireframe_shader->setUniformValue("view_matrix", view_matrix);
    wireframe_shader->setUniformValue("model_matrix", model_matrix);
    
    vao->bind();
    vao->release();
    wireframe_shader->release();
}

void OpenGLRenderer::renderSketch(Sketch* sketch) {
    if (!sketch) return;
    
    wireframe_shader->bind();
    wireframe_shader->setUniformValue("projection_matrix", projection_matrix);
    wireframe_shader->setUniformValue("view_matrix", view_matrix);
    wireframe_shader->setUniformValue("model_matrix", model_matrix);
    wireframe_shader->setUniformValue("color", QColor(255, 0, 0));
    
    for (const auto& entity : sketch->getEntities()) {
    }
    
    wireframe_shader->release();
}

// --- Modes de rendu ---
void OpenGLRenderer::renderWireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
    renderAssembly();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLRenderer::renderShaded() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
    renderAssembly();
}

void OpenGLRenderer::renderTransparent() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderAssembly();
    glDisable(GL_BLEND);
}

void OpenGLRenderer::renderEdgesOnly() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(1.0f, 1.0f, 1.0f);
    renderAssembly();
    glColor3f(0.0f, 0.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    renderAssembly();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// --- Selection ---
int OpenGLRenderer::pickObject(int x, int y) {
    return -1;
}

// --- Extraction de la geometrie ---
void OpenGLRenderer::extractGeometry() {
}

// --- Gestion de l'assemblage ---
void OpenGLRenderer::setAssembly(std::shared_ptr<Assembly> assembly) {
    this->assembly = assembly;
    update();
}

void OpenGLRenderer::setCurrentPart(std::shared_ptr<Part> part) {
    current_part = part;
    update();
}

void OpenGLRenderer::setCurrentSketch(std::shared_ptr<Sketch> sketch) {
    current_sketch = sketch;
    update();
}

// --- Mode de rendu ---
void OpenGLRenderer::setRenderMode(RenderMode mode) {
    render_mode = mode;
    update();
}

// --- Vue ---
void OpenGLRenderer::setViewType(ViewType type) {
    view_type = type;
    updateCamera();
}

void OpenGLRenderer::setIsometricView() {
    camera_position = Vec3{20, 20, 20};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 1, 0};
    view_type = ViewType::ISOMETRIC;
}

void OpenGLRenderer::setFrontView() {
    camera_position = Vec3{0, 0, 50};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 1, 0};
    view_type = ViewType::FRONT;
}

void OpenGLRenderer::setTopView() {
    camera_position = Vec3{0, 50, 0};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 0, -1};
    view_type = ViewType::TOP;
}

void OpenGLRenderer::setRightView() {
    camera_position = Vec3{50, 0, 0};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 1, 0};
    view_type = ViewType::RIGHT;
}

void OpenGLRenderer::setBackView() {
    camera_position = Vec3{0, 0, -50};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 1, 0};
    view_type = ViewType::BACK;
}

void OpenGLRenderer::setBottomView() {
    camera_position = Vec3{0, -50, 0};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 0, 1};
    view_type = ViewType::BOTTOM;
}

void OpenGLRenderer::setLeftView() {
    camera_position = Vec3{-50, 0, 0};
    camera_target = Vec3{0, 0, 0};
    camera_up = Vec3{0, 1, 0};
    view_type = ViewType::LEFT;
}

// --- Zoom ---
void OpenGLRenderer::zoomIn() {
    camera_position = vec3_add(camera_position, 
        vec3_mul(vec3_normalize(vec3_sub(camera_target, camera_position)), 5.0f));
    emit zoomChanged(1.1f);
    update();
}

void OpenGLRenderer::zoomOut() {
    camera_position = vec3_sub(camera_position, 
        vec3_mul(vec3_normalize(vec3_sub(camera_target, camera_position)), 5.0f));
    emit zoomChanged(0.9f);
    update();
}

void OpenGLRenderer::zoomFit() {
    if (!assembly) return;
    AABB bounds = AABB_EMPTY;
    for (Part* part : assembly->getParts()) {
        AABB part_bounds = part->computeAABB();
        bounds = aabb_merge(bounds, part_bounds);
    }
    if (bounds.min.x == INFINITY) return;
    Vec3 center = aabb_center(bounds);
    Vec3 size = aabb_size(bounds);
    float max_size = fmaxf(fmaxf(size.x, size.y), size.z);
    camera_position = vec3_add(center, Vec3{0, 0, max_size * 1.5f});
    camera_target = center;
    camera_up = Vec3{0, 1, 0};
    emit zoomChanged(1.0f);
    update();
}

// --- Evenements souris ---
void OpenGLRenderer::mousePressEvent(QMouseEvent* event) {
    last_mouse_position = event->position().toPoint();
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier) {
            is_selecting = true;
        } else if (event->modifiers() & Qt::ShiftModifier) {
            is_panning = true;
        } else {
            is_rotating = true;
        }
    }
    QOpenGLWidget::mousePressEvent(event);
}

void OpenGLRenderer::mouseMoveEvent(QMouseEvent* event) {
    QPoint current_pos = event->position().toPoint();
    QPoint delta = current_pos - last_mouse_position;
    if (is_rotating) {
        float sensitivity = 0.5f;
        Vec3 right = vec3_normalize(vec3_cross(camera_up, vec3_normalize(vec3_sub(camera_target, camera_position))));
        Vec3 up = camera_up;
        float yaw = delta.x() * sensitivity * 0.01f;
        Mat4 yaw_matrix = mat4_rotation_y(yaw);
        Vec3 new_position = mat4_mul_vec3(yaw_matrix, vec3_sub(camera_position, camera_target));
        camera_position = vec3_add(camera_target, new_position);
        float pitch = -delta.y() * sensitivity * 0.01f;
        Mat4 pitch_matrix = mat4_rotation_axis(right, pitch);
        new_position = mat4_mul_vec3(pitch_matrix, vec3_sub(camera_position, camera_target));
        camera_position = vec3_add(camera_target, new_position);
        updateMatrices();
        update();
    } else if (is_panning) {
        float sensitivity = 0.01f;
        Vec3 right = vec3_normalize(vec3_cross(camera_up, vec3_normalize(vec3_sub(camera_target, camera_position))));
        Vec3 up = camera_up;
        Vec3 forward = vec3_normalize(vec3_sub(camera_target, camera_position));
        Vec3 pan = vec3_add(
            vec3_mul(right, -delta.x() * sensitivity),
            vec3_mul(up, delta.y() * sensitivity)
        );
        camera_position = vec3_add(camera_position, pan);
        camera_target = vec3_add(camera_target, pan);
        updateMatrices();
        update();
    }
    last_mouse_position = current_pos;
    QOpenGLWidget::mouseMoveEvent(event);
}

void OpenGLRenderer::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        is_rotating = false;
        is_panning = false;
        is_selecting = false;
        if (event->modifiers() & Qt::ControlModifier) {
            int x = event->position().x();
            int y = event->position().y();
            selected_object_id = pickObject(x, y);
            emit selectionChanged();
        }
    }
    QOpenGLWidget::mouseReleaseEvent(event);
}

void OpenGLRenderer::wheelEvent(QWheelEvent* event) {
    QPoint num_degrees = event->angleDelta();
    float delta = num_degrees.y() / 120.0f;
    if (delta > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
    QOpenGLWidget::wheelEvent(event);
}