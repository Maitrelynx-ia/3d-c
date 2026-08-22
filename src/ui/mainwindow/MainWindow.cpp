#include "MainWindow.hpp"
#include "../renderer/OpenGLRenderer.hpp"
#include "../modeling/assembly/Assembly.hpp"
#include "../modeling/part/Part.hpp"
#include "../modeling/sketch/Sketch.hpp"
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFormLayout>
#include <QSettings>
#include <QStandardPaths>

// --- MainWindow ---
MainWindow::MainWindow(QWidget* parent) 
    : QMainWindow(parent), 
      tree_view(nullptr), graphics_view(nullptr), 
      main_tool_bar(nullptr), view_tool_bar(nullptr), 
      property_dock(nullptr), history_dock(nullptr), layers_dock(nullptr), 
      status_bar(nullptr), tree_model(nullptr), renderer(nullptr),
      current_assembly(std::make_shared<Assembly>("New Assembly")),
      current_part(nullptr), current_sketch(nullptr),
      render_mode(RenderMode::SHADED), view_type(ViewType::ISOMETRIC)
{
    setWindowTitle("CAD Engine");
    setMinimumSize(1024, 768);
    resize(1280, 960);
    
    createActions();
    createMenus();
    createToolBars();
    createDockWidgets();
    createStatusBar();
    createTreeView();
    createGraphicsView();
    
    loadSettings();
    updateTreeView();
    updateStatusBar();
    
    // Connexions
    connect(renderer, &OpenGLRenderer::selectionChanged, this, &MainWindow::onSelectionChanged);
}

MainWindow::~MainWindow() {
    saveSettings();
    delete tree_model;
}

// --- Création des actions ---
void MainWindow::createActions() {
    // Fichier
    new_action = new QAction("New", this);
    new_action->setShortcut(QKeySequence::New);
    new_action->setStatusTip("Create a new project");
    connect(new_action, &QAction::triggered, this, &MainWindow::newProject);
    
    open_action = new QAction("Open", this);
    open_action->setShortcut(QKeySequence::Open);
    open_action->setStatusTip("Open an existing project");
    connect(open_action, &QAction::triggered, this, &MainWindow::openProject);
    
    save_action = new QAction("Save", this);
    save_action->setShortcut(QKeySequence::Save);
    save_action->setStatusTip("Save the current project");
    connect(save_action, &QAction::triggered, this, &MainWindow::saveProject);
    
    save_as_action = new QAction("Save As", this);
    save_as_action->setShortcut(QKeySequence::SaveAs);
    save_as_action->setStatusTip("Save the current project as");
    connect(save_as_action, &QAction::triggered, this, &MainWindow::saveProjectAs);
    
    exit_action = new QAction("Exit", this);
    exit_action->setShortcut(QKeySequence::Quit);
    exit_action->setStatusTip("Exit the application");
    connect(exit_action, &QAction::triggered, this, &MainWindow::exitApp);
    
    // Édition
    undo_action = new QAction("Undo", this);
    undo_action->setShortcut(QKeySequence::Undo);
    undo_action->setStatusTip("Undo the last action");
    connect(undo_action, &QAction::triggered, this, &MainWindow::undo);
    
    redo_action = new QAction("Redo", this);
    redo_action->setShortcut(QKeySequence::Redo);
    redo_action->setStatusTip("Redo the last undone action");
    connect(redo_action, &QAction::triggered, this, &MainWindow::redo);
    
    cut_action = new QAction("Cut", this);
    cut_action->setShortcut(QKeySequence::Cut);
    cut_action->setStatusTip("Cut the selection");
    connect(cut_action, &QAction::triggered, this, &MainWindow::cut);
    
    copy_action = new QAction("Copy", this);
    copy_action->setShortcut(QKeySequence::Copy);
    copy_action->setStatusTip("Copy the selection");
    connect(copy_action, &QAction::triggered, this, &MainWindow::copy);
    
    paste_action = new QAction("Paste", this);
    paste_action->setShortcut(QKeySequence::Paste);
    paste_action->setStatusTip("Paste from clipboard");
    connect(paste_action, &QAction::triggered, this, &MainWindow::paste);
    
    delete_action = new QAction("Delete", this);
    delete_action->setShortcut(QKeySequence::Delete);
    delete_action->setStatusTip("Delete the selection");
    connect(delete_action, &QAction::triggered, this, &MainWindow::deleteSelection);
    
    // Modélisation
    sketch_action = new QAction("Sketch", this);
    sketch_action->setShortcut(Qt::CTRL | Qt::Key_S);
    sketch_action->setStatusTip("Create a new sketch");
    connect(sketch_action, &QAction::triggered, this, &MainWindow::createSketch);
    
    extrude_action = new QAction("Extrude", this);
    extrude_action->setShortcut(Qt::CTRL | Qt::Key_E);
    extrude_action->setStatusTip("Extrude a sketch");
    connect(extrude_action, &QAction::triggered, this, &MainWindow::createExtrude);
    
    revolve_action = new QAction("Revolve", this);
    revolve_action->setShortcut(Qt::CTRL | Qt::Key_R);
    revolve_action->setStatusTip("Revolve a sketch");
    connect(revolve_action, &QAction::triggered, this, &MainWindow::createRevolve);
    
    fillet_action = new QAction("Fillet", this);
    fillet_action->setShortcut(Qt::CTRL | Qt::Key_F);
    fillet_action->setStatusTip("Add a fillet");
    connect(fillet_action, &QAction::triggered, this, &MainWindow::createFillet);
    
    chamfer_action = new QAction("Chamfer", this);
    chamfer_action->setShortcut(Qt::CTRL | Qt::Key_C);
    chamfer_action->setStatusTip("Add a chamfer");
    connect(chamfer_action, &QAction::triggered, this, &MainWindow::createChamfer);
    
    hole_action = new QAction("Hole", this);
    hole_action->setShortcut(Qt::CTRL | Qt::Key_H);
    hole_action->setStatusTip("Create a hole");
    connect(hole_action, &QAction::triggered, this, &MainWindow::createHole);
    
    boolean_union_action = new QAction("Union", this);
    boolean_union_action->setStatusTip("Boolean union");
    connect(boolean_union_action, &QAction::triggered, this, &MainWindow::createBooleanUnion);
    
    boolean_difference_action = new QAction("Difference", this);
    boolean_difference_action->setStatusTip("Boolean difference");
    connect(boolean_difference_action, &QAction::triggered, this, &MainWindow::createBooleanDifference);
    
    boolean_intersection_action = new QAction("Intersection", this);
    boolean_intersection_action->setStatusTip("Boolean intersection");
    connect(boolean_intersection_action, &QAction::triggered, this, &MainWindow::createBooleanIntersection);
    
    // Affichage
    wireframe_action = new QAction("Wireframe", this);
    wireframe_action->setCheckable(true);
    wireframe_action->setChecked(render_mode == RenderMode::WIREFRAME);
    connect(wireframe_action, &QAction::triggered, this, &MainWindow::setWireframeMode);
    
    shaded_action = new QAction("Shaded", this);
    shaded_action->setCheckable(true);
    shaded_action->setChecked(render_mode == RenderMode::SHADED);
    connect(shaded_action, &QAction::triggered, this, &MainWindow::setShadedMode);
    
    transparent_action = new QAction("Transparent", this);
    transparent_action->setCheckable(true);
    transparent_action->setChecked(render_mode == RenderMode::TRANSPARENT);
    connect(transparent_action, &QAction::triggered, this, &MainWindow::setTransparentMode);
    
    edges_only_action = new QAction("Edges Only", this);
    edges_only_action->setCheckable(true);
    edges_only_action->setChecked(render_mode == RenderMode::EDGES_ONLY);
    connect(edges_only_action, &QAction::triggered, this, &MainWindow::setEdgesOnlyMode);
    
    // Vues
    isometric_action = new QAction("Isometric", this);
    isometric_action->setCheckable(true);
    isometric_action->setChecked(view_type == ViewType::ISOMETRIC);
    connect(isometric_action, &QAction::triggered, this, &MainWindow::setIsometricView);
    
    front_action = new QAction("Front", this);
    front_action->setCheckable(true);
    front_action->setChecked(view_type == ViewType::FRONT);
    connect(front_action, &QAction::triggered, this, &MainWindow::setFrontView);
    
    top_action = new QAction("Top", this);
    top_action->setCheckable(true);
    top_action->setChecked(view_type == ViewType::TOP);
    connect(top_action, &QAction::triggered, this, &MainWindow::setTopView);
    
    right_action = new QAction("Right", this);
    right_action->setCheckable(true);
    right_action->setChecked(view_type == ViewType::RIGHT);
    connect(right_action, &QAction::triggered, this, &MainWindow::setRightView);
    
    back_action = new QAction("Back", this);
    back_action->setCheckable(true);
    back_action->setChecked(view_type == ViewType::BACK);
    connect(back_action, &QAction::triggered, this, &MainWindow::setBackView);
    
    bottom_action = new QAction("Bottom", this);
    bottom_action->setCheckable(true);
    bottom_action->setChecked(view_type == ViewType::BOTTOM);
    connect(bottom_action, &QAction::triggered, this, &MainWindow::setBottomView);
    
    left_action = new QAction("Left", this);
    left_action->setCheckable(true);
    left_action->setChecked(view_type == ViewType::LEFT);
    connect(left_action, &QAction::triggered, this, &MainWindow::setLeftView);
    
    // Zoom
    zoom_in_action = new QAction("Zoom In", this);
    zoom_in_action->setShortcut(QKeySequence::ZoomIn);
    connect(zoom_in_action, &QAction::triggered, this, &MainWindow::zoomIn);
    
    zoom_out_action = new QAction("Zoom Out", this);
    zoom_out_action->setShortcut(QKeySequence::ZoomOut);
    connect(zoom_out_action, &QAction::triggered, this, &MainWindow::zoomOut);
    
    zoom_fit_action = new QAction("Zoom Fit", this);
    zoom_fit_action->setShortcut(Qt::Key_F);
    connect(zoom_fit_action, &QAction::triggered, this, &MainWindow::zoomFit);
}

// --- Création des menus ---
void MainWindow::createMenus() {
    // Menu Fichier
    QMenu* file_menu = menuBar()->addMenu("File");
    file_menu->addAction(new_action);
    file_menu->addAction(open_action);
    file_menu->addSeparator();
    file_menu->addAction(save_action);
    file_menu->addAction(save_as_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);
    
    // Menu Édition
    QMenu* edit_menu = menuBar()->addMenu("Edit");
    edit_menu->addAction(undo_action);
    edit_menu->addAction(redo_action);
    edit_menu->addSeparator();
    edit_menu->addAction(cut_action);
    edit_menu->addAction(copy_action);
    edit_menu->addAction(paste_action);
    edit_menu->addAction(delete_action);
    
    // Menu Modélisation
    QMenu* modeling_menu = menuBar()->addMenu("Modeling");
    modeling_menu->addAction(sketch_action);
    modeling_menu->addAction(extrude_action);
    modeling_menu->addAction(revolve_action);
    modeling_menu->addSeparator();
    modeling_menu->addAction(fillet_action);
    modeling_menu->addAction(chamfer_action);
    modeling_menu->addAction(hole_action);
    modeling_menu->addSeparator();
    modeling_menu->addAction(boolean_union_action);
    modeling_menu->addAction(boolean_difference_action);
    modeling_menu->addAction(boolean_intersection_action);
    
    // Menu Affichage
    QMenu* view_menu = menuBar()->addMenu("View");
    
    QMenu* render_menu = view_menu->addMenu("Render Mode");
    render_menu->addAction(wireframe_action);
    render_menu->addAction(shaded_action);
    render_menu->addAction(transparent_action);
    render_menu->addAction(edges_only_action);
    
    QMenu* camera_menu = view_menu->addMenu("Camera");
    camera_menu->addAction(isometric_action);
    camera_menu->addAction(front_action);
    camera_menu->addAction(top_action);
    camera_menu->addAction(right_action);
    camera_menu->addAction(back_action);
    camera_menu->addAction(bottom_action);
    camera_menu->addAction(left_action);
    
    camera_menu->addSeparator();
    camera_menu->addAction(zoom_in_action);
    camera_menu->addAction(zoom_out_action);
    camera_menu->addAction(zoom_fit_action);
    
    // Menu Outils
    QMenu* tools_menu = menuBar()->addMenu("Tools");
    // TODO: Ajouter des outils (simulation, mise en plan, etc.)
    
    // Menu Aide
    QMenu* help_menu = menuBar()->addMenu("Help");
    // TODO: Ajouter de l'aide
}

// --- Création des barres d'outils ---
void MainWindow::createToolBars() {
    // Barre d'outils principale
    main_tool_bar = addToolBar("Main");
    main_tool_bar->addAction(new_action);
    main_tool_bar->addAction(open_action);
    main_tool_bar->addAction(save_action);
    main_tool_bar->addSeparator();
    main_tool_bar->addAction(undo_action);
    main_tool_bar->addAction(redo_action);
    main_tool_bar->addSeparator();
    main_tool_bar->addAction(sketch_action);
    main_tool_bar->addAction(extrude_action);
    main_tool_bar->addAction(revolve_action);
    
    // Barre d'outils de vue
    view_tool_bar = addToolBar("View");
    view_tool_bar->addAction(isometric_action);
    view_tool_bar->addAction(front_action);
    view_tool_bar->addAction(top_action);
    view_tool_bar->addAction(right_action);
    view_tool_bar->addSeparator();
    view_tool_bar->addAction(wireframe_action);
    view_tool_bar->addAction(shaded_action);
    view_tool_bar->addSeparator();
    view_tool_bar->addAction(zoom_in_action);
    view_tool_bar->addAction(zoom_out_action);
    view_tool_bar->addAction(zoom_fit_action);
}

// --- Création des docks ---
void MainWindow::createDockWidgets() {
    // Dock Arbre de modélisation (à gauche)
    QDockWidget* tree_dock = new QDockWidget("Model Tree", this);
    tree_dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    tree_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, tree_dock);
    
    QWidget* tree_widget = new QWidget();
    QVBoxLayout* tree_layout = new QVBoxLayout(tree_widget);
    tree_layout->setContentsMargins(0, 0, 0, 0);
    tree_view = new QTreeView();
    tree_layout->addWidget(tree_view);
    tree_dock->setWidget(tree_widget);
    
    // Dock Éditeur de propriétés (à droite)
    property_dock = new QDockWidget("Properties", this);
    property_dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    property_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, property_dock);
    
    QWidget* property_widget = new QWidget();
    QVBoxLayout* property_layout = new QVBoxLayout(property_widget);
    property_layout->setContentsMargins(5, 5, 5, 5);
    
    QLabel* property_label = new QLabel("No selection");
    property_label->setAlignment(Qt::AlignCenter);
    property_layout->addWidget(property_label);
    
    property_dock->setWidget(property_widget);
    
    // Dock Historique (en bas à gauche)
    history_dock = new QDockWidget("History", this);
    history_dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    history_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, history_dock);
    
    QWidget* history_widget = new QWidget();
    QVBoxLayout* history_layout = new QVBoxLayout(history_widget);
    history_layout->setContentsMargins(5, 5, 5, 5);
    
    QLabel* history_label = new QLabel("No history");
    history_label->setAlignment(Qt::AlignCenter);
    history_layout->addWidget(history_label);
    
    history_dock->setWidget(history_widget);
    
    // Dock Calques (en bas à droite)
    layers_dock = new QDockWidget("Layers", this);
    layers_dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    layers_dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, layers_dock);
    
    QWidget* layers_widget = new QWidget();
    QVBoxLayout* layers_layout = new QVBoxLayout(layers_widget);
    layers_layout->setContentsMargins(5, 5, 5, 5);
    
    QLabel* layers_label = new QLabel("No layers");
    layers_label->setAlignment(Qt::AlignCenter);
    layers_layout->addWidget(layers_label);
    
    layers_dock->setWidget(layers_widget);
}

// --- Création de la barre de statut ---
void MainWindow::createStatusBar() {
    status_bar = new QStatusBar(this);
    setStatusBar(status_bar);
    
    QLabel* status_label = new QLabel("Ready");
    status_bar->addWidget(status_label, 1);
    
    QLabel* coords_label = new QLabel("X: 0.00, Y: 0.00, Z: 0.00");
    status_bar->addWidget(coords_label);
}

// --- Création de l'arbre de modélisation ---
void MainWindow::createTreeView() {
    tree_model = new QStandardItemModel(this);
    tree_view->setModel(tree_model);
    tree_view->setHeaderHidden(true);
    tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
    
    connect(tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, 
            this, &MainWindow::onSelectionChanged);
}

// --- Création de la vue graphique ---
void MainWindow::createGraphicsView() {
    graphics_view = new QGraphicsView(this);
    graphics_view->setRenderHint(QPainter::Antialiasing);
    graphics_view->setRenderHint(QPainter::SmoothPixmapTransform);
    graphics_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    
    // Créer le widget OpenGL
    renderer = new OpenGLRenderer(graphics_view);
    graphics_view->setViewport(renderer);
    
    setCentralWidget(graphics_view);
    
    // Connexions pour le zoom et le pan
    connect(renderer, &OpenGLRenderer::zoomChanged, this, [this](float factor) {
        if (factor > 1.0f) {
            statusBar->showMessage("Zooming in");
        } else {
            statusBar->showMessage("Zooming out");
        }
    });
}

// --- Mise à jour de l'arbre de modélisation ---
void MainWindow::updateTreeView() {
    tree_model->clear();
    
    if (!current_assembly) {
        return;
    }
    
    // Racine : Assemblage
    QStandardItem* assembly_item = new QStandardItem(QIcon(":/icons/assembly.png"), QString::fromStdString(current_assembly->getName()));
    tree_model->appendRow(assembly_item);
    
    // Pièces
    for (Part* part : current_assembly->getParts()) {
        QStandardItem* part_item = new QStandardItem(QIcon(":/icons/part.png"), QString::fromStdString(part->getName()));
        assembly_item->appendRow(part_item);
        
        // Esquisses
        for (const auto& sketch : part->getSketches()) {
            QStandardItem* sketch_item = new QStandardItem(QIcon(":/icons/sketch.png"), QString::fromStdString(sketch->getName()));
            part_item->appendRow(sketch_item);
            
            // Entités de l'esquisse
            for (const auto& entity : sketch->getEntities()) {
                QString entity_name;
                switch (entity->type) {
                    case SketchEntityType::LINE: entity_name = "Line"; break;
                    case SketchEntityType::CIRCLE: entity_name = "Circle"; break;
                    case SketchEntityType::ARC: entity_name = "Arc"; break;
                    case SketchEntityType::POINT: entity_name = "Point"; break;
                    default: entity_name = "Entity"; break;
                }
                QStandardItem* entity_item = new QStandardItem(QIcon(":/icons/line.png"), entity_name);
                sketch_item->appendRow(entity_item);
            }
        }
        
        // Features
        for (const auto& feature : part->getFeatures()) {
            QString feature_name;
            switch (feature->type) {
                case FeatureType::EXTRUDE: feature_name = "Extrude"; break;
                case FeatureType::REVOLVE: feature_name = "Revolve"; break;
                case FeatureType::FILLET: feature_name = "Fillet"; break;
                case FeatureType::CHAMFER: feature_name = "Chamfer"; break;
                case FeatureType::HOLE: feature_name = "Hole"; break;
                default: feature_name = "Feature"; break;
            }
            QStandardItem* feature_item = new QStandardItem(QIcon(":/icons/feature.png"), feature_name);
            part_item->appendRow(feature_item);
        }
    }
    
    tree_view->expandAll();
}

// --- Mise à jour de l'éditeur de propriétés ---
void MainWindow::updatePropertyEditor() {
    // TODO: Implémenter la mise à jour de l'éditeur de propriétés
    // en fonction de la sélection
}

// --- Mise à jour de la barre de statut ---
void MainWindow::updateStatusBar() {
    QString status;
    
    if (current_sketch) {
        status = "Sketch: " + QString::fromStdString(current_sketch->getName());
    } else if (current_part) {
        status = "Part: " + QString::fromStdString(current_part->getName());
    } else if (current_assembly) {
        status = "Assembly: " + QString::fromStdString(current_assembly->getName());
    } else {
        status = "Ready";
    }
    
    status_bar->showMessage(status);
}

// --- Chargement des paramètres ---
void MainWindow::loadSettings() {
    QSettings settings("CAD Engine", "Settings");
    
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    render_mode = static_cast<RenderMode>(settings.value("renderMode", (int)RenderMode::SHADED).toInt());
    view_type = static_cast<ViewType>(settings.value("viewType", (int)ViewType::ISOMETRIC).toInt());
    
    // Mettre à jour les actions
    wireframe_action->setChecked(render_mode == RenderMode::WIREFRAME);
    shaded_action->setChecked(render_mode == RenderMode::SHADED);
    transparent_action->setChecked(render_mode == RenderMode::TRANSPARENT);
    edges_only_action->setChecked(render_mode == RenderMode::EDGES_ONLY);
    
    isometric_action->setChecked(view_type == ViewType::ISOMETRIC);
    front_action->setChecked(view_type == ViewType::FRONT);
    top_action->setChecked(view_type == ViewType::TOP);
    right_action->setChecked(view_type == ViewType::RIGHT);
    back_action->setChecked(view_type == ViewType::BACK);
    bottom_action->setChecked(view_type == ViewType::BOTTOM);
    left_action->setChecked(view_type == ViewType::LEFT);
    
    // Appliquer le mode de rendu
    renderer->setRenderMode(render_mode);
    renderer->setViewType(view_type);
}

// --- Sauvegarde des paramètres ---
void MainWindow::saveSettings() {
    QSettings settings("CAD Engine", "Settings");
    
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("renderMode", (int)render_mode);
    settings.setValue("viewType", (int)view_type);
}

// --- Gestion de l'assemblage ---
void MainWindow::setAssembly(std::shared_ptr<Assembly> assembly) {
    current_assembly = assembly;
    updateTreeView();
    updateStatusBar();
    renderer->setAssembly(assembly);
}

void MainWindow::setCurrentPart(std::shared_ptr<Part> part) {
    current_part = part;
    current_sketch = nullptr;
    updateTreeView();
    updateStatusBar();
    renderer->setCurrentPart(part);
}

void MainWindow::setCurrentSketch(std::shared_ptr<Sketch> sketch) {
    current_sketch = sketch;
    updateTreeView();
    updateStatusBar();
    renderer->setCurrentSketch(sketch);
}

// --- Gestion du mode de rendu ---
void MainWindow::setRenderMode(RenderMode mode) {
    render_mode = mode;
    renderer->setRenderMode(mode);
    
    wireframe_action->setChecked(mode == RenderMode::WIREFRAME);
    shaded_action->setChecked(mode == RenderMode::SHADED);
    transparent_action->setChecked(mode == RenderMode::TRANSPARENT);
    edges_only_action->setChecked(mode == RenderMode::EDGES_ONLY);
}

void MainWindow::setWireframeMode() {
    setRenderMode(RenderMode::WIREFRAME);
}

void MainWindow::setShadedMode() {
    setRenderMode(RenderMode::SHADED);
}

void MainWindow::setTransparentMode() {
    setRenderMode(RenderMode::TRANSPARENT);
}

void MainWindow::setEdgesOnlyMode() {
    setRenderMode(RenderMode::EDGES_ONLY);
}

// --- Gestion de la vue ---
void MainWindow::setViewType(ViewType type) {
    view_type = type;
    renderer->setViewType(type);
    
    isometric_action->setChecked(type == ViewType::ISOMETRIC);
    front_action->setChecked(type == ViewType::FRONT);
    top_action->setChecked(type == ViewType::TOP);
    right_action->setChecked(type == ViewType::RIGHT);
    back_action->setChecked(type == ViewType::BACK);
    bottom_action->setChecked(type == ViewType::BOTTOM);
    left_action->setChecked(type == ViewType::LEFT);
}

void MainWindow::setIsometricView() {
    setViewType(ViewType::ISOMETRIC);
}

void MainWindow::setFrontView() {
    setViewType(ViewType::FRONT);
}

void MainWindow::setTopView() {
    setViewType(ViewType::TOP);
}

void MainWindow::setRightView() {
    setViewType(ViewType::RIGHT);
}

void MainWindow::setBackView() {
    setViewType(ViewType::BACK);
}

void MainWindow::setBottomView() {
    setViewType(ViewType::BOTTOM);
}

void MainWindow::setLeftView() {
    setViewType(ViewType::LEFT);
}

// --- Zoom ---
void MainWindow::zoomIn() {
    renderer->zoomIn();
}

void MainWindow::zoomOut() {
    renderer->zoomOut();
}

void MainWindow::zoomFit() {
    renderer->zoomFit();
}

// --- Rafraîchissement ---
void MainWindow::refresh() {
    updateTreeView();
    updatePropertyEditor();
    updateStatusBar();
    renderer->update();
}

// --- Fermeture ---
void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    QMainWindow::closeEvent(event);
}

// --- Slots de modélisation ---
void MainWindow::newProject() {
    current_assembly = std::make_shared<Assembly>("New Assembly");
    current_part = nullptr;
    current_sketch = nullptr;
    refresh();
}

void MainWindow::openProject() {
    QString filename = QFileDialog::getOpenFileName(this, "Open Project", 
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "CAD Files (*.cad);;All Files (*)");
    if (!filename.isEmpty()) {
        // TODO: Charger le projet
        // current_assembly = Assembly::deserialize(filename.toStdString().c_str());
        refresh();
    }
}

void MainWindow::saveProject() {
    if (current_assembly) {
        QString filename = QFileDialog::getSaveFileName(this, "Save Project",
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                        "CAD Files (*.cad);;All Files (*)");
        if (!filename.isEmpty()) {
            // TODO: Sauvegarder le projet
            // current_assembly->serialize(filename.toStdString().c_str());
        }
    }
}

void MainWindow::saveProjectAs() {
    saveProject();
}

void MainWindow::exitApp() {
    close();
}

void MainWindow::undo() {
    // TODO: Implémenter undo
}

void MainWindow::redo() {
    // TODO: Implémenter redo
}

void MainWindow::cut() {
    // TODO: Implémenter cut
}

void MainWindow::copy() {
    // TODO: Implémenter copy
}

void MainWindow::paste() {
    // TODO: Implémenter paste
}

void MainWindow::deleteSelection() {
    // TODO: Implémenter delete
}

void MainWindow::createSketch() {
    if (current_part) {
        Sketch* sketch = current_part->createSketch(SketchPlane::XY);
        current_sketch = std::shared_ptr<Sketch>(sketch);
        refresh();
    } else if (current_assembly) {
        // Créer une nouvelle pièce avec une esquisse
        auto part = std::make_shared<Part>("Part_" + QString::number(current_assembly->getParts().size() + 1).toStdString());
        current_assembly->addPart(part.get());
        current_part = part;
        Sketch* sketch = current_part->createSketch(SketchPlane::XY);
        current_sketch = std::shared_ptr<Sketch>(sketch);
        refresh();
    }
}

void MainWindow::createExtrude() {
    if (current_sketch) {
        current_part->addExtrude(current_sketch.get(), 10.0f);
        current_sketch = nullptr;
        refresh();
    }
}

void MainWindow::createRevolve() {
    if (current_sketch) {
        current_part->addRevolve(current_sketch.get(), 360.0f, VEC3_Z);
        current_sketch = nullptr;
        refresh();
    }
}

void MainWindow::createFillet() {
    // TODO: Implémenter fillet
}

void MainWindow::createChamfer() {
    // TODO: Implémenter chamfer
}

void MainWindow::createHole() {
    // TODO: Implémenter hole
}

void MainWindow::createBooleanUnion() {
    // TODO: Implémenter boolean union
}

void MainWindow::createBooleanDifference() {
    // TODO: Implémenter boolean difference
}

void MainWindow::createBooleanIntersection() {
    // TODO: Implémenter boolean intersection
}

// --- Sélection ---
void MainWindow::onSelectionChanged() {
    // TODO: Mettre à jour la sélection dans l'arbre et l'éditeur de propriétés
    updatePropertyEditor();
    updateStatusBar();
}

// --- Mise à jour de l'assemblage ---
void MainWindow::onAssemblyChanged() {
    refresh();
}

