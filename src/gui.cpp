#include "gui.h"

using namespace std;
using namespace glv;

namespace Splash
{

/*************/
Gui::Gui(GlWindowPtr w)
{
    _type = "gui";

    if (w.get() == nullptr)
        return;

    _window = w;
    glfwMakeContextCurrent(_window->get());
    glGetError();
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    {
        TexturePtr texture(new Texture);
        texture->reset(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        _depthTexture = move(texture);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture->getTexId(), 0);
    }

    {
        TexturePtr texture(new Texture);
        texture->reset(GL_TEXTURE_2D, 0, GL_RGB8, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        _outTexture = move(texture);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _outTexture->getTexId(), 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        SLog::log << Log::WARNING << "Gui::" << __FUNCTION__ << " - Error while initializing framebuffer object: " << status << Log::endl;
    else
        SLog::log << Log::MESSAGE << "Gui::" << __FUNCTION__ << " - Framebuffer object successfully initialized" << Log::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glfwMakeContextCurrent(NULL);

    initGLV(_width, _height);
    registerAttributes();
}

/*************/
Gui::~Gui()
{
}

/*************/
bool Gui::render()
{
    glfwMakeContextCurrent(_window->get());

    GLenum error = glGetError();
    ImageSpec spec = _outTexture->getSpec();
    glViewport(0, 0, spec.width, spec.height);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    GLenum fboBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, fboBuffers);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0, 1.0, 1.0, 1.0); //< This is the transparent color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    
    _glv.drawWidgets(spec.width, spec.height, 0.016);

    glActiveTexture(GL_TEXTURE0);
    _outTexture->generateMipmap();

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    error = glGetError();
    if (error)
        SLog::log << Log::WARNING << "Gui::" << __FUNCTION__ << " - Error while rendering the camera: " << error << Log::endl;

    glfwMakeContextCurrent(NULL);
    return error != 0 ? true : false;
}

/*************/
void Gui::setOutputSize(int width, int height)
{
    if (width == 0 || height == 0)
        return;

    glfwMakeContextCurrent(_window->get());
    _depthTexture->reset(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    _outTexture->reset(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glfwMakeContextCurrent(NULL);

    initGLV(width, height);
}

/*************/
void Gui::initGLV(int width, int height)
{
    _glvLog.width(width / 2);
    _glvLog.height(height / 4);
    _glvLog.bottom(height - 8);
    _glvLog.left(8);
    _glvLog.colors().set(Color(0.2, 0.4, 1.0, 0.8), 0.7);
    _glv << _glvLog;
}

/*************/
void Gui::registerAttributes()
{
    _attribFunctions["size"] = AttributeFunctor([&](vector<Value> args) {
        if (args.size() < 2)
            return false;
        _width = args[0].asInt();
        _height = args[1].asInt();
        setOutputSize(_width, _height);
        return true;
    });
}

/*************/
void GlvTextBox::onDraw(GLV& g)
{
    draw::color(SPLASH_GLV_TEXTCOLOR);
    draw::lineWidth(SPLASH_GLV_FONTSIZE);
    float l = 4;
    float t = 4;
    float fontSize = 8;
    float lineSpacing = 1;

    // Compute the number of lines which would fit
    int nbrLines = h / (int)(fontSize + lineSpacing);

    // Convert the last lines of the text log
    vector<string> logs = SLog::log.getLogs(Log::DEBUG);
    string text;
    for (auto t = logs.begin() + std::max(0, ((int)logs.size() - nbrLines)); t != logs.end(); ++t)
        text += *t + string("\n");
    draw::text(text.c_str(), 4, 4);
}

} // end of namespace
