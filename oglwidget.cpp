#include "oglwidget.h"
#include "qdebug.h"
#include <cmath>

#define PROGRAM_VERTEX_ATTRIBUTE 0

OGLWidget::OGLWidget(int min, int max, int step, QWidget *parent)
    : QOpenGLWidget(parent), mouseDown(false), levels(), level(0.0), smoothLevel(0.0), low(3, 10, NUM_POINTS, "low"), high(50, 100, NUM_POINTS, "high")
{
    for (int i = 0; i < NUM_POINTS; ++i) {
        frequencies.push_back(0.0);
        smoothFrequencies.push_back(0.0);
    }
    setMouseTracking(true);
    installEventFilter(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
}

OGLWidget::~OGLWidget()
{

}

int OGLWidget::getMin() {
    return low.getMin();
}

int OGLWidget::getMax() {
    return low.getMax();
}

float OGLWidget::getThresh() {
    return low.getThresh();
}

void OGLWidget::setThresh(float newThresh) {
    return low.setThresh(newThresh);
}

void OGLWidget::createVBO() {
    time_t seconds;
    seconds = time (NULL);

    std::vector<float> vertices;
    for (int i = 0; i < NUM_POINTS; ++i) {
        float from = -1.0 + ((float)i/NUM_POINTS) * 2.0;
        vertices.push_back(from);
        vertices.push_back(smoothFrequencies[i] - 1.0);
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

            uniform float regions[12];

            void main() {
                FragColor = vec4(0.0);
                for (int i = 0; i < 2; i++) {
                    int ind = i * 6;
                    float start = regions[ind + 0];
                    float end = regions[ind + 1];
                    float level = regions[ind + 2];
                    float thresh = regions[ind + 3];
                    float peak = regions[ind + 4];
                    int color = (int)regions[ind + 5];

                    float t = -1.0 + 2.0 * thresh;
                    int visible = (int)(pos.x > start && pos.x < end);
                    int threshold = (int)(pos.y < t + 0.01 && pos.y > t - 0.01);
                    start = (int)(pos.x < start + 0.001 && pos.x > start - 0.001);
                    end = (int)(pos.x < end + 0.001 && pos.x > end - 0.001);

                    vec4 c = vec4(peak, 1.0, peak, 1.0);
                    if (color == 1) {
                        c = vec4(1.0, peak, peak, 1.0);
                    }

                    int visible2 = (int)(pos.y < (-1.0 + 2.0 * level));

                    FragColor += (c * start + c * end) + threshold * visible * vec4(1.0) + vec4(vec3(1.0), 0.5) * visible2 * visible;
                }
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
    glClearColor(0,0,0,1);

    if (peaked) {
        peak = 1.0;
    } else {
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
    std::vector<GLfloat> test;

    for (auto reg : {&low, &high}) {
        test.push_back(reg->getStart());
        test.push_back(reg->getEnd());
        test.push_back(smoothLevel);
        test.push_back(reg->getThresh());
        test.push_back(peak);
        test.push_back(static_cast<GLfloat>(reg->getColor()));
    }

    program->setUniformValueArray("regions", test.data(), 12, 1);
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();

    peaked = false;
}


bool OGLWidget::eventFilter(QObject *obj, QEvent *event) {
    auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (mouseEvent != nullptr) {
        float x = ((float)mouseEvent->pos().x() / (float)width());
        float y = ((float)mouseEvent->pos().y() / (float)height());


        int i = 0;
        for (auto& reg : {&low, &high}) {
            i++;

            reg->mouseEvent(x, y);

            if (reg->newInside) {
                setCursor(Qt::OpenHandCursor);
                if (reg->newOnLine && !reg->dragging) {
                    setCursor(Qt::SizeVerCursor);
                }
                break;
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }

        if (mouseEvent->button() == Qt::LeftButton) {
            if (event->type() == QEvent::MouseButtonPress) {
                low.mouseClick(x, y);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                low.mouseReleased(x, y);

            }
        }
        if (mouseEvent->button() == Qt::RightButton) {
            if (event->type() == QEvent::MouseButtonPress) {
                high.mouseClick(x, y);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                high.mouseReleased(x, y);
            }
        }
    }
}

void OGLWidget::setFrequencies(const std::vector<float> &newFrequencies, bool peak, float level)
{
    this->frequencies = newFrequencies;
    for (int i = 0; i < newFrequencies.size(); i++) {
        if (newFrequencies[i] > smoothFrequencies[i]) {
            smoothFrequencies[i] = newFrequencies[i];
        } else {
            smoothFrequencies[i] = (smoothFrequencies[i] > 0.05) ? smoothFrequencies[i] - 0.05 : 0.0;
        }
    }

    this->peaked = peak;
    this->level = std::fmin(level, 1.0);
    update();
}

void OGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
