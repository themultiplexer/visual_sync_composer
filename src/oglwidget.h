#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include "frequencyregion.h"
#include <GL/gl.h>
#include <QWidget>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/normalize_dot.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define NUM_POINTS 1024

enum class VisMode {
    ExpMean,
    Mean,
    Variance
};

struct Vertex2D {
    glm::vec2 position;
    glm::vec4 color;
};

class OGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OGLWidget(int step, QWidget *parent = 0);
    ~OGLWidget();

    float getThresh();
    void setThresh(float newThresh);
    void processData(std::vector<float> &data, const std::function<void (FrequencyRegion &)> &callback);
    float getDecay() const;
    void setDecay(float newDecay);
    std::vector<FrequencyRegion*> getRegions() const;
    void setRegions(std::vector<FrequencyRegion *> newRegions);
    void setFrequencies(const std::vector<float> &leftFrequencies, const std::vector<float> &rightFrequencies);

    VisMode getVisMode() const;
    void setVisMode(VisMode newVisMode);

signals:
    void threshChanged();
    void rangeChanged();


protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QOpenGLShaderProgram *regionShaderProgram, *lineShaderProgram;
    QOpenGLVertexArrayObject vao, lineVao;
    QOpenGLBuffer vertexPositionBuffer, lineVertexPositionBuffer, vertexBuffer;
    GLuint shaderProgram;
    std::array<float, 1024> smoothFrequencies, smoothFrequencies2, runningMean1, runningVar1, runningMean2, runningVar2;
    std::array<int, 1024> recentFrequencies, recentFrequencies2;
    std::vector<FrequencyRegion*> regions;
    std::array<std::array<float, 512>, 1024> windows1, windows2;

    void createVBO();
private:
    bool eventFilter(QObject *obj, QEvent *event);
    GLuint m_vbo;
    bool dragging;
    int step;
    int currentRegionIndex;
    VisMode visMode;

    int index1, index2;

    float decay;
    std::vector<Vertex2D> generatePolylineQuads(const std::vector<Vertex2D> &points, float width);
    std::vector<Vertex2D> lineVertices, lineVertices2;
    void cleanupGL();
    void calcMean(float x_new, std::array<float, 1024> &means, std::array<float, 1024> &vars, std::array<float, 512> &data, int &index);
};

#endif // OGLWIDGET_H
