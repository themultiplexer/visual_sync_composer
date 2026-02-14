#ifndef TUBEWIDGET_H
#define TUBEWIDGET_H

#include "wifieventprocessor.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

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
#include <random>


class TubeWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    TubeWidget(QWidget *parent = 0);
    ~TubeWidget();

    void setFrequencies(const std::vector<float> &newFrequencies, bool peak, float level);

    bool peaked;
    void setPeaked(QColor color);
    QColor color;
    void setEffect(CONFIG_DATA effect);

    void sync();
protected:
    void initializeGL();
    void paintGL();

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao, vao1;
    QOpenGLBuffer vertexPositionBuffer, vertexBuffer;
    GLuint shaderProgram;

    std::random_device dev;
    std::mt19937 *rng;
    std::uniform_int_distribution<std::mt19937::result_type> *pixelRandom;
    std::chrono::time_point<std::chrono::system_clock> timeRef;

    int particle;
    CONFIG_DATA effect;

    QVector<float> sizes, brightness;

private:
    float peak;
    float time;
};

#endif // TUBEWIDGET_H
