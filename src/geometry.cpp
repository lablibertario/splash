#include "geometry.h"

using namespace std;

namespace Splash {

/*************/
Geometry::Geometry()
{
    _type = "geometry";

    _mesh.reset(new Mesh());
    update();
    _timestamp = _mesh->getTimestamp();
}

/*************/
Geometry::~Geometry()
{
    glDeleteBuffers(1, &_vertexCoords);
    glDeleteBuffers(1, &_texCoords);
    glDeleteBuffers(1, &_normals);
    for (auto v : _vertexArray)
        glDeleteVertexArrays(1, &(v.second));

    SLog::log << Log::DEBUG << "Geometry::~Geometry - Destructor" << Log::endl;
}

/*************/
void Geometry::activate()
{
    glBindVertexArray(_vertexArray[glfwGetCurrentContext()]);
}

/*************/
void Geometry::deactivate() const
{
    glBindVertexArray(0);
}

/*************/
bool Geometry::linkTo(BaseObjectPtr obj)
{
    if (dynamic_pointer_cast<Mesh>(obj).get() != nullptr)
    {
        MeshPtr mesh = dynamic_pointer_cast<Mesh>(obj);
        setMesh(mesh);
        return true;
    }

    return false;
}

/*************/
void Geometry::update()
{
    if (!_mesh)
        return; // No mesh, no update

    // Update the vertex buffers if mesh was updated
    if (_timestamp != _mesh->getTimestamp())
    {
        glDeleteBuffers(1, &_vertexCoords);
        glDeleteBuffers(1, &_texCoords);
        glDeleteBuffers(1, &_normals);

        glGenBuffers(1, &_vertexCoords);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexCoords);
        vector<float> vertices = _mesh->getVertCoords();
        if (vertices.size() == 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            return;
        }
        _verticesNumber = vertices.size() / 4;
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glGenBuffers(1, &_texCoords);
        glBindBuffer(GL_ARRAY_BUFFER, _texCoords);
        vector<float> texcoords = _mesh->getUVCoords();
        if (texcoords.size() == 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            return;
        }
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &_normals);
        glBindBuffer(GL_ARRAY_BUFFER, _normals);
        vector<float> normals = _mesh->getNormals();
        if (normals.size() == 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            return;
        }
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        for (auto v : _vertexArray)
            glDeleteVertexArrays(1, &(v.second));
        _vertexArray.clear();

        _timestamp = _mesh->getTimestamp();
    }

    GLFWwindow* context = glfwGetCurrentContext();
    if (_vertexArray.find(context) == _vertexArray.end())
    {
        _vertexArray[context] = 0;
        
        glGenVertexArrays(1, &(_vertexArray[context]));
        glBindVertexArray(_vertexArray[context]);

        glBindBuffer(GL_ARRAY_BUFFER, _vertexCoords);
        glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray((GLuint)0);

        glBindBuffer(GL_ARRAY_BUFFER, _texCoords);
        glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray((GLuint)1);

        glBindBuffer(GL_ARRAY_BUFFER, _normals);
        glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray((GLuint)2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

/*************/
void Geometry::registerAttributes()
{
}

} // end of namespace
