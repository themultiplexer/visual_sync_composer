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

    QColor getColor() const;
    void setColor(const QColor &newColor);

    void setOuterPercentage(float newPercentage);
    float getOuterPercentage() const;
    float getInnerPercentage() const;
    void setInnerPercentage(float newPercentage);
signals:
    void verticalMouseMovement(float diff);

protected:
    void initializeGL() override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

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
