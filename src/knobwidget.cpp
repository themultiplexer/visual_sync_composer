#include "knobwidget.h"
#include "helper.h"


#define PROGRAM_VERTEX_ATTRIBUTE 0


KnobWidget::KnobWidget(QWidget *parent)
    : QOpenGLWidget(parent), outerPercentage(0.5f), innerPercentage(0.75f), color(255, 0, 0), shiftPressed(false)
{

    setMouseTracking(true);
    installEventFilter(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    timeZero = std::chrono::system_clock::now();
}

KnobWidget::~KnobWidget()
{

}

void KnobWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setUpdateBehavior(NoPartialUpdate);
    glClearColor(0,0,0,0);

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
            out vec4 fragColor;

            uniform float iTime;
            uniform vec2 iResolution;
            uniform float per1;
            uniform float per2;
            uniform vec3 knobColor;

            float sdRing( in vec2 p, in vec2 n, in float r, in float th )
            {
                p.x = abs(p.x);

                p = mat2(n.x,n.y,-n.y,n.x)*p;

                return max( abs(length(p)-r)-th*0.5,
                            length(vec2(p.x,max(0.0,abs(r-p.y)-th*0.5)))*sign(p.x) );
            }

            float sdCircle( vec2 p, float r )
            {
                return length(p) - r;
            }

            mat2 rot(float a) {
                float c = cos(a), s = sin(a);
                return mat2(c,-s,s,c);
            }

            vec3 hsv2rgb(vec3 c)
            {
                vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
                return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
            }

            void main()
            {
                vec2 fragCoord = gl_FragCoord.xy;
                // normalized pixel coordinates
                vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;

                float t1 = -(3.14159 * 0.75  - 0.05)*per1 - 0.1;
                vec2 p1 = rot(t1 - (3.14159 * 0.25)) * p;

                float t2 = -(3.14159 * 0.75  - 0.05)*per2 - 0.1;
                vec2 p2 = rot(t2 - (3.14159 * 0.25)) * p;

                float d1 = sdRing(p1, vec2(cos(t1),sin(t1)), 0.8, 0.1);
                float d2 = sdRing(p2, vec2(cos(t2),sin(t2)), 0.6, 0.1);

                vec4 col1 = (d1>0.0) ? vec4(vec3(0.0), 0.0) : vec4(vec3(1.0), 1.0);
                col1 = mix( col1, vec4(1.0), 1.0-smoothstep(0.0,0.05,abs(d1)) );

                vec4 col2 = (d2>0.0) ? vec4(vec3(0.0), 0.0) : vec4(vec3(1.0), 1.0);
                col2 = mix( col2, vec4(1.0), 1.0-smoothstep(0.0,0.05,abs(d2)) );

                float d3 = sdCircle(p, 0.35);

                vec4 col3 = (d3>0.0) ? vec4(vec3(0.207843137), 1.0) : vec4(knobColor, 1.0);
                col3 = mix( col3, vec4(knobColor, 1.0), 1.0-smoothstep(0.0,0.05, abs(d3)) );

                fragColor = col1 + col2 + col3;
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

void KnobWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastPos = event->pos();
    }
}

void KnobWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int dx = event->pos().x() - lastPos.x();
        int dy = event->pos().y() - lastPos.y();
        lastPos = event->pos();
        emit verticalMouseMovement(dy * 0.5 * 0.01);
    }
}

void KnobWidget::wheelEvent(QWheelEvent *event)
{
    qDebug() << "wheel ev angle delta:" << event->angleDelta();
    qDebug() << "wheel ev pixel delta:" << event->pixelDelta();
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    distance += numSteps;
    update();
}

QColor KnobWidget::getColor() const
{
    return color;
}

void KnobWidget::setColor(const QColor &newColor)
{
    color = newColor;
    update();
}

float KnobWidget::getOuterPercentage() const
{
    return outerPercentage;
}

void KnobWidget::updateColor() {
    rgb color = hsv2rgb({getOuterPercentage() * 360.0f, getInnerPercentage(), 1.0});
    qDebug() << color.r << color.g << color.b;
    setColor(QColor(color.r * 255, color.g * 255, color.b * 255));
}

void KnobWidget::setOuterPercentage(float newPercentage)
{
    outerPercentage = newPercentage;
    updateColor();
    update();
}

float KnobWidget::getInnerPercentage() const
{
    return innerPercentage;
}

void KnobWidget::setInnerPercentage(float newPercentage)
{
    innerPercentage = newPercentage;
    updateColor();
    update();
}

void KnobWidget::paintGL()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timeZero).count();
    program->bind();
    program->setUniformValue("iTime", ((float)ms / 1000.0f));
    program->setUniformValue("iResolution", QVector2D(width(), height()));
    program->setUniformValue("per1", outerPercentage);
    program->setUniformValue("per2", innerPercentage);
    program->setUniformValue("knobColor", QVector3D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0));
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();
}

