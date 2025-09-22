#ifndef KNOBWIDGET_H
#define KNOBWIDGET_H

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


class KnobWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    KnobWidget(QWidget *parent = 0);
    ~KnobWidget();

    void setFrequencies(const std::vector<float> &newFrequencies, bool peak, float level);

    float getPercentage() const;
    void setPercentage(float newPercentage);

    QColor getColor() const;
    void setColor(const QColor &newColor);

signals:
    void knobChanged(float value);

protected:
    void initializeGL();
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao, vao1;
    QOpenGLBuffer vertexPositionBuffer, vertexBuffer;

    GLuint shaderProgram;
    std::chrono::time_point<std::chrono::system_clock> timeZero;

private:
    float percentage;
    QColor color;
    QPoint lastPos;
    double distance;
};

#endif // KNOBWIDGET_H
