#ifndef TUBEWIDGET_H
#define TUBEWIDGET_H

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
protected:
    void initializeGL();
    void paintGL();

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao, vao1;
    QOpenGLBuffer vertexPositionBuffer, vertexBuffer;
    GLuint shaderProgram;
private:
    float peak;
    float time;
};

#endif // TUBEWIDGET_H
