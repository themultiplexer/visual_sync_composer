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

#define NUM_POINTS 640

class OGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OGLWidget(int min, int max, int step, QWidget *parent = 0);
    ~OGLWidget();

    void setFrequencies(const std::vector<float> &newFrequencies);

    int getMin();
    int getMax();
    float getThresh();
    void setThresh(float newThresh);
    void processData(std::vector<float> &data, const std::function<void (FrequencyRegion &)> &callback);
signals:
    void valueChanged();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao, vao1;
    QOpenGLBuffer vertexPositionBuffer, vertexBuffer;
    GLuint shaderProgram;
    std::vector<float> frequencies;
    std::vector<float> smoothFrequencies;
    bool mouseDown;

    FrequencyRegion high, low;

    void createVBO();
private:
    bool eventFilter(QObject *obj, QEvent *event);
    GLuint m_vbo;
    bool dragging;
};

#endif // OGLWIDGET_H
