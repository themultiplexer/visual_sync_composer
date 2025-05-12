#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include "frequencyregion.h"
#include "helper.h"
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
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/normalize_dot.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define NUM_POINTS 1024

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

    void setFrequencies(const std::vector<float> &newFrequencies);

    float getThresh();
    void setThresh(float newThresh);
    void processData(std::vector<float> &data, const std::function<void (FrequencyRegion &)> &callback);
    float getDecay() const;
    void setDecay(float newDecay);
    std::vector<FrequencyRegion*> getRegions() const;
    void setRegions(std::vector<FrequencyRegion *> newRegions);

signals:
    void valueChanged();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QOpenGLShaderProgram *regionShaderProgram, *lineShaderProgram;
    QOpenGLVertexArrayObject vao, lineVao;
    QOpenGLBuffer vertexPositionBuffer, lineVertexPositionBuffer, vertexBuffer;
    GLuint shaderProgram;
    std::vector<float> frequencies;
    std::vector<float> smoothFrequencies;
    std::vector<int> recentFrequencies;
    std::vector<FrequencyRegion*> regions;

    void createVBO();
private:
    bool eventFilter(QObject *obj, QEvent *event);
    GLuint m_vbo;
    bool dragging;
    int step;
    int currentRegionIndex;
    float decay;
    std::vector<Vertex2D> generatePolylineQuads(const std::vector<Vertex2D> &points, float width);
    std::vector<Vertex2D> lineVertices;
    void cleanupGL();
};

#endif // OGLWIDGET_H
