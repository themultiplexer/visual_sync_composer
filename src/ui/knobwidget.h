#ifndef KNOBWIDGET_H
#define KNOBWIDGET_H

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>

#include <QQuickFramebufferObject>

#include <QtQml/qqmlregistration.h>

class KnobWidget : public QQuickFramebufferObject::Renderer
{
public:
    KnobWidget();
    ~KnobWidget();

    void setFrequencies(const std::vector<float> &newFrequencies, bool peak, float level);

    QColor getColor() const;
    void setColor(const QColor &newColor);

    void setOuterPercentage(float newPercentage);
    float getOuterPercentage() const;
    float getInnerPercentage() const;
    void setInnerPercentage(float newPercentage);
    void updateColor();
signals:
    void verticalMouseMovement(float diff);

protected:
    void initializeGL();
    void render() override;

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao, vao1;
    QOpenGLBuffer vertexPositionBuffer, vertexBuffer;

    GLuint shaderProgram;
    std::chrono::time_point<std::chrono::system_clock> timeZero;
private:
    float innerPercentage, outerPercentage;
    QColor color;
    QPoint lastPos;
    double distance;
    bool shiftPressed;
};

#endif // KNOBWIDGET_H
