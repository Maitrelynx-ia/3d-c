#ifndef CAD_MAIN_WINDOW_HPP
#define CAD_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QDockWidget>
#include <QTreeView>
#include <QGraphicsView>
#include <QToolBar>
#include <QStatusBar>
#include <QStandardItemModel>
#include <memory>

class OpenGLRenderer;
class Part;
class Assembly;
class Sketch;

enum class RenderMode { WIREFRAME, SHADED, TRANSPARENT, EDGES_ONLY };
enum class ViewType { ISOMETRIC, FRONT, TOP, RIGHT, BACK, BOTTOM, LEFT };

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void setAssembly(std::shared_ptr<Assembly> assembly);
    std::shared_ptr<Assembly> getAssembly() const { return current_assembly; }
    void setCurrentPart(std::shared_ptr<Part> part);
    std::shared_ptr<Part> getCurrentPart() const { return current_part; }
    void setCurrentSketch(std::shared_ptr<Sketch> sketch);
    std::shared_ptr<Sketch> getCurrentSketch() const { return current_sketch; }
    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() const { return render_mode; }
    void setViewType(ViewType type);
    ViewType getViewType() const { return view_type; }
    void refresh();
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    QTreeView* tree_view;
    QGraphicsView* graphics_view;
    QToolBar* main_tool_bar;
    QToolBar* view_tool_bar;
    QDockWidget* property_dock;
    QDockWidget* history_dock;
    QDockWidget* layers_dock;
    QStatusBar* status_bar;
    QStandardItemModel* tree_model;
    OpenGLRenderer* renderer;
    std::shared_ptr<Assembly> current_assembly;
    std::shared_ptr<Part> current_part;
    std::shared_ptr<Sketch> current_sketch;
    RenderMode render_mode;
    ViewType view_type;
    QAction* new_action; QAction* open_action; QAction* save_action; QAction* save_as_action; QAction* exit_action;
    QAction* undo_action; QAction* redo_action; QAction* cut_action; QAction* copy_action; QAction* paste_action; QAction* delete_action;
    QAction* sketch_action; QAction* extrude_action; QAction* revolve_action; QAction* fillet_action; QAction* chamfer_action; QAction* hole_action;
    QAction* boolean_union_action; QAction* boolean_difference_action; QAction* boolean_intersection_action;
    QAction* wireframe_action; QAction* shaded_action; QAction* transparent_action; QAction* edges_only_action;
    QAction* isometric_action; QAction* front_action; QAction* top_action; QAction* right_action; QAction* back_action; QAction* bottom_action; QAction* left_action;
    QAction* zoom_in_action; QAction* zoom_out_action; QAction* zoom_fit_action; QAction* pan_action; QAction* rotate_action;
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void createStatusBar();
    void createTreeView();
    void createGraphicsView();
    void updateTreeView();
    void updatePropertyEditor();
    void updateStatusBar();
    void loadSettings();
    void saveSettings();
private slots:
    void newProject(); void openProject(); void saveProject(); void saveProjectAs(); void exitApp();
    void undo(); void redo(); void cut(); void copy(); void paste(); void deleteSelection();
    void createSketch(); void createExtrude(); void createRevolve(); void createFillet(); void createChamfer(); void createHole();
    void createBooleanUnion(); void createBooleanDifference(); void createBooleanIntersection();
    void setWireframeMode(); void setShadedMode(); void setTransparentMode(); void setEdgesOnlyMode();
    void setIsometricView(); void setFrontView(); void setTopView(); void setRightView(); void setBackView(); void setBottomView(); void setLeftView();
    void zoomIn(); void zoomOut(); void zoomFit();
    void onSelectionChanged();
    void onAssemblyChanged();
};

#endif