#include "oglwidget.h"
#include "audiowindow.h"
#include "qdebug.h"
#include "qtimer.h"
#include <cmath>
#include <QOpenGLDebugLogger>

#define PROGRAM_VERTEX_ATTRIBUTE 0

OGLWidget::OGLWidget(int step, QWidget *parent)
    : QOpenGLWidget(parent), step(step), decay(0.02), regions(), currentRegionIndex(0)
{
    regions.push_back(new FrequencyRegion(1, 1, 5, NUM_POINTS, "low"));
    regions.push_back(new FrequencyRegion(2, 170, 205, NUM_POINTS, "high"));

    setMouseTracking(true);
    installEventFilter(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
}

OGLWidget::~OGLWidget()
{

}

void OGLWidget::cleanupGL() {
    makeCurrent(); // Ensure the context is current
    qDebug() << "Cleanup";

    if (lineShaderProgram) {
        delete lineShaderProgram;
        lineShaderProgram = nullptr;
    }

    if (regionShaderProgram) {
        delete regionShaderProgram;
        regionShaderProgram = nullptr;
    }

    doneCurrent(); // Done with the context
}

float OGLWidget::getThresh() {
    return regions[0]->getThresh();
}

void OGLWidget::setThresh(float newThresh) {
    return regions[0]->setThresh(newThresh);
}

void OGLWidget::processData(std::vector<float> &data, const std::function <void (FrequencyRegion&)>& callback)
{
    for (auto& reg : regions) {
        if (reg->processData(data)) {
            callback(*reg);
        }
    }
}

void OGLWidget::initializeGL()
{
    qDebug() << "init GL";
    initializeOpenGLFunctions();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OGLWidget::cleanupGL);

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
        uniform float stepsize;

        float inverseLogScale(float y, float base) {
            float absY = (y + 1.0)/2.0;
            float scaled = (pow(base, absY) - 1.0) / (base - 1.0);
            return -1.0 + scaled * 2.0;
        }

        void main() {
            FragColor = vec4(0.0);
            for (int i = 0; i < 2; i++) {
                int ind = i * 6;
                float start = regions[ind + 0];
                float end = regions[ind + 1];
                float level = regions[ind + 2];
                float thresh = regions[ind + 3];
                float peak = regions[ind + 4];
                int color = int(regions[ind + 5]);

                float t = -1.0 + 2.0 * thresh;
                int visible = int(pos.x > start && pos.x < end);
                int threshold = int(pos.y < t + 0.01 && pos.y > t - 0.01);
                float width = 0.001 + 0.01 * peak;
                start = int(pos.x < start + width && pos.x > start - width);
                end = int(pos.x < end + width && pos.x > end - width);

                vec4 c = vec4(peak, 1.0, peak, 1.0);
                if (color == 1) {
                    c = vec4(1.0, peak, peak, 1.0);
                }

                int visible2 = int(pos.y < (-1.0 + 2.0 * level));

                FragColor += (c * start + c * end) + threshold * visible * vec4(1.0) + vec4(vec3(1.0), 0.5) * visible2 * visible;
            }
            float mx = mod(inverseLogScale(pos.x, 40), stepsize);
            FragColor += int(mx > 0.002 && mx < 0.004) * vec4(vec3(0.2), 1.0);
        }
    )";
    fshader->compileSourceCode(fsrc);

    // Create and compile the vertex shader using a raw string literal.
    QOpenGLShader *lineVShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *lineVSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 vertex;
        layout (location = 1) in vec4 color;

        out vec2 pos;
        out vec4 col;

        float signedLogScale(float x, float base) {
            float absX = (x + 1.0)/2.0;
            float scaled = log(absX * (base - 1.0) + 1.0) / log(base);
            return -1.0 + scaled * 2.0;
        }

        void main() {
            vec2 p = vec2(signedLogScale(vertex.x, 40), vertex.y);
            //p = vertex;
            pos = p;
            col = color;
            gl_Position = vec4(p, 0.0, 1.0);
        }
    )";
    lineVShader->compileSourceCode(lineVSrc);

    // Create and compile the fragment shader.
    QOpenGLShader *lineFShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *lineFSrc = R"(
        #version 330 core
        in vec2 pos;
        in vec4 col;
        out vec4 FragColor;

        void main() {
            FragColor = col;
        }
    )";
    lineFShader->compileSourceCode(lineFSrc);

    lineShaderProgram = new QOpenGLShaderProgram(this);
    lineShaderProgram->addShader(lineVShader);
    lineShaderProgram->addShader(lineFShader);
    lineShaderProgram->link();

    lineVao.create();
    lineVao.bind();
    lineVertexPositionBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    lineVertexPositionBuffer.create();
    lineVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    lineVertexPositionBuffer.bind();
    lineVertexPositionBuffer.allocate(lineVertices.data(), lineVertices.size() * sizeof(Vertex2D));

    lineShaderProgram->bind();
    lineShaderProgram->enableAttributeArray(0);
    lineShaderProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex2D, position), 2, sizeof(Vertex2D));
    lineShaderProgram->enableAttributeArray(1);
    lineShaderProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex2D, color), 4, sizeof(Vertex2D));
    lineShaderProgram->release();
    lineVao.release();

    regionShaderProgram = new QOpenGLShaderProgram(this);
    regionShaderProgram->addShader(vshader);
    regionShaderProgram->addShader(fshader);
    regionShaderProgram->link();

    QVector<QVector2D> vertices;
    vertices << QVector2D(-1.f,  1.f)
             << QVector2D(-1.f, -1.f)
             << QVector2D( 1.f, -1.f)
             << QVector2D(-1.f,  1.f)
             << QVector2D( 1.f, -1.f)
             << QVector2D( 1.f, 1.f);

    vao.create();
    vao.bind();
    vertexPositionBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();
    vertexPositionBuffer.allocate(vertices.data(), vertices.size() * sizeof(QVector2D));

    regionShaderProgram->bind();
    regionShaderProgram->setUniformValue("stepsize", 0.05f);
    regionShaderProgram->enableAttributeArray(0);
    regionShaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(QVector2D));
    vertexPositionBuffer.release();
    vao.release();
}

void OGLWidget::paintGL()
{
    glClearColor(0,0,0,1);

    lineShaderProgram->bind();
    lineVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, lineVertices.size());
    lineVao.release();
    lineShaderProgram->release();

    lineVertexPositionBuffer.bind();
    lineVertexPositionBuffer.allocate(lineVertices2.data(), lineVertices2.size() * sizeof(Vertex2D));
    lineVertexPositionBuffer.release();

    lineShaderProgram->bind();
    lineVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, lineVertices2.size());
    lineVao.release();
    lineShaderProgram->release();

    regionShaderProgram->bind();
    std::vector<GLfloat> test;
    for (auto reg : regions) {
        test.push_back(-1.0 + 2 * reg->getStart());
        test.push_back(-1.0 + 2 * reg->getEnd());
        test.push_back(reg->getSmoothLevel());
        test.push_back(reg->getThresh());
        test.push_back(reg->getPeak());
        test.push_back(static_cast<GLfloat>(reg->getColor()));
    }
    regionShaderProgram->setUniformValueArray("regions", test.data(), 12, 1);
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    regionShaderProgram->release();
}

std::vector<FrequencyRegion*> OGLWidget::getRegions() const
{
    return regions;
}

void OGLWidget::setRegions(std::vector<FrequencyRegion*> newRegions)
{
    regions = newRegions;
}

// Creates quads (2 triangles per segment) along a polyline with width
std::vector<Vertex2D> OGLWidget::generatePolylineQuads(const std::vector<Vertex2D>& points, float width) {
    std::vector<Vertex2D> vertices;

    if (points.size() < 2) {
        std::cerr << "Error: Need at least 2 points to generate a polyline.\n";
        return vertices;
    }

    float halfWidth = width / 2.0f;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        Vertex2D p0 = points[i];
        Vertex2D p1 = points[i + 1];

        glm::vec2 dir = glm::normalize(p1.position - p0.position);
        glm::vec2 normal = glm::vec2(-dir.y, dir.x); // Perpendicular to direction

        // Compute the quad corners
        glm::vec2 v0 = p0.position + normal * halfWidth;
        glm::vec2 v1 = p0.position - normal * halfWidth;
        glm::vec2 v2 = p1.position - normal * halfWidth;
        glm::vec2 v3 = p1.position + normal * halfWidth;

        // Two triangles per quad: v0-v1-v2 and v2-v3-v0
        vertices.push_back({ v0, p0.color });
        vertices.push_back({ v1, p0.color });
        vertices.push_back({ v2, p0.color });

        vertices.push_back({ v2, p1.color });
        vertices.push_back({ v3, p1.color });
        vertices.push_back({ v0, p1.color });
    }

    return vertices;
}


bool OGLWidget::eventFilter(QObject *obj, QEvent *event) {
    auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (mouseEvent != nullptr) {
        float x = ((float)mouseEvent->pos().x() / (float)width());
        float y = ((float)mouseEvent->pos().y() / (float)height());

        FrequencyRegion* active = regions[mouseEvent->modifiers() == Qt::ControlModifier];

        for (auto& reg : regions) {
            reg->mouseEvent(x, y);

            if (reg->getNewInside()) {
                setCursor(Qt::OpenHandCursor);
                if (reg->getNewOnLine()) {
                    if (!reg->getDragging()) {
                        setCursor(Qt::SizeVerCursor);
                    }
                } else if (reg->getNewOnStart() || reg->getNewOnEnd()) {
                    setCursor(Qt::SizeHorCursor);
                } else {
                    setCursor(Qt::ArrowCursor);
                }
                active = reg;
                emit rangeChanged();
                if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease) {
                    return true;
                }
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }

        if (mouseEvent->button() == Qt::LeftButton) {
            if (event->type() == QEvent::MouseButtonPress) {
                active->mouseClick(x, y);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                active->mouseReleased(x, y);
            }
        }
    }
    return false;
}

VisMode OGLWidget::getVisMode() const
{
    return visMode;
}

void OGLWidget::setVisMode(VisMode newVisMode)
{
    visMode = newVisMode;
}

float OGLWidget::getDecay() const
{
    return decay;
}

void OGLWidget::setDecay(float newDecay)
{
    decay = newDecay;
}

void OGLWidget::calcMean(float x_new, std::array<float, 1024> &means, std::array<float, 1024> &vars, std::array<float, 512> &data, int &index) {
    float mean = means[index];
    int window_size = 512;
    int next_index = (index + 1) % window_size;
    float x_old = data[next_index];
    float new_mean = mean + (x_new - x_old) / (float)window_size;

    //vars[i] += (x_new - mean) * (x_new - new_mean) - (x_old - mean) * (x_old - new_mean);
    vars[index] += (x_new + x_old - mean - new_mean) * (x_new - x_old);

    means[index] = new_mean;
    data[next_index] = x_new;
    index = next_index;
}

void OGLWidget::setFrequencies(const std::vector<float> &leftFrequencies, const std::vector<float> &rightFrequencies)
{
    std::vector<Vertex2D> path1, path2;
    float width = 0.005f;
    float alpha = 0.7;

    for (int i = 0; i < NUM_POINTS; i++) {

        calcMean(leftFrequencies[i], runningMean1, runningVar1, windows1[i], index1);
        calcMean(rightFrequencies[i], runningMean2, runningVar2, windows2[i], index2);

        if (visMode == VisMode::ExpMean) {
            smoothFrequencies[i] = (alpha * leftFrequencies[i]) + (1.0 - alpha) * smoothFrequencies[i];
            smoothFrequencies2[i] = (alpha * rightFrequencies[i]) + (1.0 - alpha) * smoothFrequencies2[i];
        } else if (visMode == VisMode::Mean) {
            smoothFrequencies[i] =  runningMean1[i] * 500.0;
            smoothFrequencies2[i] =  runningMean2[i] * 500.0;
        } if (visMode == VisMode::Variance) {
            smoothFrequencies[i] = (runningVar1[i] / (float)512) * 1000.0;
            smoothFrequencies2[i] = (runningVar2[i] / (float)512) * 1000.0;
        }


        if (leftFrequencies[i] > smoothFrequencies[i]) {
            recentFrequencies[i] = 100;
        } else {
            recentFrequencies[i] -= recentFrequencies[i] > 10 ? 10 : 0;
        }

        if (rightFrequencies[i] > smoothFrequencies2[i]) {
            recentFrequencies2[i] = 100;
        } else {
            recentFrequencies2[i] -= recentFrequencies2[i] > 10 ? 10 : 0;
        }

        rgb c = hsv2rgb({((float)i/(float)NUM_POINTS) * 360, (1.0 - ((float)recentFrequencies[i] / 100.0)), 1.0});
        path1.push_back({glm::vec2(((float)i/(float)NUM_POINTS) * 2.0 - 1.0, smoothFrequencies[i] * 2.0 - (1.0 - width)), glm::vec4(c.r, c.g, c.b, 1.0) });
        path2.push_back({glm::vec2(((float)i/(float)NUM_POINTS) * 2.0 - 1.0, smoothFrequencies2[i] * 2.0 - (1.1 - width)), glm::vec4(0.5,0.5,0.5, 1.0) });
    }
    lineVertices = generatePolylineQuads(path1, width);
    lineVertices2 = generatePolylineQuads(path2, width);

    lineVertexPositionBuffer.bind();
    lineVertexPositionBuffer.allocate(lineVertices.data(), lineVertices.size() * sizeof(Vertex2D));
    lineVertexPositionBuffer.release();


    update();
}

void OGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
