class OGLRenderer : public QQuickFramebufferObject::Renderer
{
public:
    void render() override
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // your OpenGL rendering here

        update();
    }
};