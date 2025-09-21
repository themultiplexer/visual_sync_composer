#include "knobwidget.h"


#define PROGRAM_VERTEX_ATTRIBUTE 0


KnobWidget::KnobWidget(QWidget *parent)
    : QOpenGLWidget(parent)
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
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0);
    glPointSize(10.0);

    vao1.create();
    vao1.bind();
    vertexBuffer.create();
    vertexBuffer.bind();

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
            out vec4 fragColor;

            uniform float iTime;
            uniform vec2 iResolution;
            uniform vec3 iMouse;

            float sdRing( in vec2 p, in vec2 n, in float r, in float th )
            {
                p.x = abs(p.x);
                p = mat2(n.x,n.y,-n.y,n.x)*p;

                return max( abs(length(p)-r)-th*0.5, length(vec2(p.x,max(0.0,abs(r-p.y)-th*0.5)))*sign(p.x) );
            }

            void main()
            {
                vec2 fragCoord = gl_FragCoord.xy;
                // normalized pixel coordinates
                vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;
                vec2 m = (2.0*iMouse.xy-iResolution.xy)/iResolution.y;

                // animation
                float t = 3.14159*(0.5+0.5*cos(iTime*0.52));
                vec2 cs = vec2(cos(t),sin(t));
                const float ra = 0.5;
                const float th = 0.2;

                // distance
                float d = sdRing(p, cs, ra, th);

                // coloring
                vec3 col = (d>0.0) ? vec3(0.9,0.6,0.3) : vec3(0.65,0.85,1.0);
                col *= 1.0 - exp2(-24.0*abs(d));
                col *= 0.8 + 0.2*cos(110.0*abs(d));
                col = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.01,abs(d)) );

                // mouse interaction
                if( iMouse.z>0.001 ) {
                    d = sdRing(m, cs, ra, th);
                    d = min( abs(length(p-m)-abs(d))-0.0025,
                             length(p-m)-0.015 );
                    col = mix(col, vec3(1.0,1.0,0.0), 1.0-smoothstep(0.0, 0.005, d) );
                }

                fragColor = vec4(col, 1.0);
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

void KnobWidget::paintGL()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timeZero).count();
    program->bind();
    program->setUniformValue("iTime", ((float)ms / 1000.0f));
    program->setUniformValue("iResolution", QVector2D(width(), height()));
    program->setUniformValue("iMouse", QVector3D(width(), height(), 0));
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();
}

