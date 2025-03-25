#include "oglwidget.h"
#include "qdebug.h"
#include <cmath>


#define PROGRAM_VERTEX_ATTRIBUTE 0


OGLWidget::OGLWidget(int min, int max, int step, QWidget *parent)
    : QOpenGLWidget(parent), mouseDown(false), step(step), min(min), max(max), start(-0.9 + 2 *((float)min/(float)step)), end(-0.9 + 2 *((float)max/(float)step)), levels(), thresh(0.5), level(0.0), smoothLevel(0.0)
{
    for (int i = 0; i < NUM_POINTS; ++i) {
            frequencies.push_back(0.0);
    }
    setMouseTracking(true);
    installEventFilter(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
}

OGLWidget::~OGLWidget()
{

}

void OGLWidget::createVBO() {
    time_t seconds;
    seconds = time (NULL);

    std::vector<float> vertices;
    for (int i = 0; i < NUM_POINTS; ++i) {
        float from = -0.9 + ((float)i/NUM_POINTS) * 1.8;
        vertices.push_back(from);
        vertices.push_back(frequencies[i] - 0.9);
    }
    vertexBuffer.bind();
    vertexBuffer.allocate(vertices.data(), vertices.size() * sizeof(float));
    //vertexBuffer.release();
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setUpdateBehavior(NoPartialUpdate);
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHTING);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL);
    glLineWidth(2.0);
    glPointSize(10.0);

    vao1.create();
    vao1.bind();
    vertexBuffer.create();
    vertexBuffer.bind();
    OGLWidget::createVBO();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    vertexBuffer.release();
    vao1.release();

    // Create and compile the vertex shader using a raw string literal.
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc = R"(
            #version 330 core
            layout (location = 0) in vec2 vertex;
            uniform int pressed;
            uniform float start;
            uniform float end;
            out vec2 pos;

            void main() {
                pos = vertex;
                gl_Position = vec4(vertex, 0.0, 1.0);
            }
        )";
    vshader->compileSourceCode(vsrc);

    // Create and compile the fragment shader.
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc = R"(
            #version 330 core
            in vec2 pos;
            out vec4 FragColor;
            uniform int pressed;
            uniform float start;
            uniform float level;
            uniform float thresh;
            uniform int color;
            uniform float end;
            uniform float peak;

            #define PI 3.14159265359

            float plot(vec2 st, float pct){
              return  smoothstep(pct, pct, st.y) -
                      smoothstep(pct, pct, st.y);
            }

            void main() {
                int visible = (int)(pos.x > start && pos.x < end);

                float t = -1.0 + 2.0 * thresh;
                int threshold = (int)(pos.y < t + 0.01 && pos.y > t - 0.01);

                float y = smoothstep(start - 0.01, start, pos.x) - smoothstep(end, end + 0.01, pos.x);
                y = y - visible;

                vec3 c = vec3(peak, 1.0, peak);
                if (color == 1) {
                    c = vec3(1.0, peak, peak);
                }

                vec3 color = c * y;

                float pct = plot(pos, y);
                color = (1.0 - pct)*color;

                int visible2 = (int)(pos.y < (-1.0 + 2.0 * level));

                FragColor = vec4(color + ((float)visible * (float)visible2 * c), 1.0) + vec4(vec3(threshold * visible), 1.0);
            }
        )";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
    program->link();

    QVector<QVector2D> vertices;
    vertices << QVector2D(-1.f,  1.f)
             << QVector2D(-1.f, -1.f)
             << QVector2D( 1.f, -1.f)
             << QVector2D(-1.f,  1.f)
             << QVector2D( 1.f, -1.f)
             << QVector2D( 1.f, 1.f);

    vao.create();
    vao.bind();
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();
    vertexPositionBuffer.allocate(vertices.constData(), vertices.size() * sizeof(QVector2D));
    program->bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(QVector2D));
    vertexPositionBuffer.release();
    vao.release();
    program->release();
}

void OGLWidget::paintGL()
{
    if (peaked) {
        glClearColor(1,0,0,1);
        peak = 1.0;
    } else {
        glClearColor(0,0,0,1);
        if (peak > 0) {
            peak -= 0.05;
        }
    }

    if (level > smoothLevel) {
        smoothLevel = level;
    } else {
        if (smoothLevel > 0) {
            smoothLevel -= 0.02;
        }
    }

    levels.push(level);

    float sum = std::accumulate(levels.begin(), levels.end(), 0.0);
    float mean = sum / levels.size();

    OGLWidget::createVBO();
    vao1.bind();
    glDrawArrays(GL_LINE_STRIP, 0, NUM_POINTS);
    vao1.release();

    program->bind();
    program->setUniformValue("pressed", mouseDown);
    program->setUniformValue("peak", peak);
    program->setUniformValue("level", smoothLevel);
    program->setUniformValue("thresh", thresh);
    program->setUniformValue("color", start < end);
    program->setUniformValue("start", getStart());
    program->setUniformValue("end", getEnd());
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();

    peaked = false;
}

float OGLWidget::getEnd() const
{
    return end > start ? end : start;

}

void OGLWidget::setEnd(float newEnd)
{
    end = std::fmax(std::fmin(newEnd, 0.9), -0.9);
}

float OGLWidget::getStart() const
{
    return start < end ? start : end;
}

void OGLWidget::setStart(float newStart)
{
    start = std::fmax(std::fmin(newStart, 0.9), -0.9);
}

float OGLWidget::getThresh() const
{
    return thresh;
}

void OGLWidget::setThresh(float newThresh)
{
    thresh = newThresh;
}

int OGLWidget::getMax() const
{
    return max > min ? max : min;
}

int OGLWidget::getMin() const
{
    return min < max ? min : max;
}

bool OGLWidget::eventFilter(QObject *obj, QEvent *event) {
    auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (mouseEvent != nullptr) {
        float x = ((float)mouseEvent->pos().x() / (float)width());
        float y = 1.0 - ((float)mouseEvent->pos().y() / (float)height());
        float rx = (x - fmod(x, 1.0/512));
        float vx = std::fmax(std::fmin(rx, 0.95), 0.05);
        float px = -1.0 + 2 * vx;

        bool newInside = (px > getStart() && px < getEnd());
        bool newOnLine = newInside && (y < thresh + 0.05 && y > thresh - 0.05);
        if (newOnLine != onLine) {
            if (onLine) {
                emit mouseEnterEvent();
            } else {
                emit mouseLeaveEvent();
            }
        }
        onLine = newOnLine;

        if (mouseEvent->button() == Qt::LeftButton) {
            if (event->type() == QEvent::MouseButtonPress) {
                mouseDown = true;
                inside = newInside;
                dragging = onLine;
                dx = px;
                prestart = start;
                preend = end;
                if (!dragging && !inside) {
                    start = px;
                    min = (vx * 1.1111 - 0.05) * 512;
                }
            } else if (event->type() == QEvent::MouseButtonRelease) {
                mouseDown = false;
                if (dragging) {
                    thresh = y;
                } else if(!inside) {
                    end = -1.0 + 2 * vx;
                    max = (vx * 1.1111 - 0.05) * 512;
                }
                dragging = false;
                inside = false;
            }
        }
        if (mouseDown) {
            if (dragging) {
                thresh = y;
                emit valueChanged();
            } else if (inside) {
                setStart(prestart + (px - dx));
                setEnd(preend + (px - dx));
            } else {
                end = px;
            }
        }
        //current = mouseEvent->pos().x();
    }
}


void OGLWidget::setFrequencies(const std::vector<float> &newFrequencies, bool peak, float level)
{
    this->peaked = peak;
    this->frequencies = newFrequencies;
    this->level = std::fmin(level, 1.0);
    update();
}

void OGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
