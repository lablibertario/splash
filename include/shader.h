/*
 * Copyright (C) 2013 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blobserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobserver.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @shader.h
 * The Shader class
 */

#ifndef SPLASH_SHADER_H
#define SPLASH_SHADER_H

#define GLFW_NO_GLU
#define GL_GLEXT_PROTOTYPES

#include "config.h"
#include "coretypes.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <GLFW/glfw3.h>

#include "texture.h"

namespace Splash {

class Shader : public BaseObject
{
    public:
        enum ShaderType
        {
            vertex = 0,
            geometry,
            fragment
        };

        enum Sideness
        {
            doubleSided = 0,
            singleSided,
            inverted
        };

        enum Fill
        {
            texture = 0,
            color,
            uv,
            wireframe,
            window
        };

        /**
         * Constructor
         */
        Shader();

        /**
         * Destructor
         */
        ~Shader();

        /**
         * No copy constructor, but a move one
         */
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader& operator=(Shader&& s)
        {
            if (this != &s)
            {
                _shaders = s._shaders;
                _program = s._program;
                _isLinked = s._isLinked;
                _locationMVP = s._locationMVP;
                _locationNormalMatrix = s._locationNormalMatrix;
                _locationSide = s._locationSide;
                _locationTextureNbr = s._locationTextureNbr;
                _locationBlendingMap = s._locationBlendingMap;
                _locationBlendWidth = s._locationBlendWidth;
                _locationBlackLevel = s._locationBlackLevel;
                _locationColor = s._locationColor;
                _locationScale = s._locationScale;
                _locationLayout = s._locationLayout;

                _fill = s._fill;
                _sideness = s._sideness;
                _textureNbr = s._textureNbr;
                _useBlendingMap = s._useBlendingMap;
                _blendWidth = s._blendWidth;
                _blackLevel = s._blackLevel;
                _brightness = s._brightness;
                _color = s._color;
                _scale = s._scale;
                _textureOverlap = s._textureOverlap;
                _layout = s._layout;
            }
            return *this;
        }

        /**
         * Activate this shader
         */
        void activate();

        /**
         * Activate the blending from the given texture nbr
         */
        void activateBlending(int textureNbr) {_useBlendingMap = textureNbr;}

        /**
         * Deactivate this shader
         */
        void deactivate();

        /**
         * Deactivate the blending
         */
        void deactivateBlending() {_useBlendingMap = 0;}

        /**
         * Set the sideness of the object
         */
        void setSideness(const Sideness side);
        Sideness getSideness() const {return _sideness;}

        /**
         * Set a shader source
         */
        void setSource(const std::string& src, const ShaderType type);

        /**
         * Set a shader source from file
         */
        void setSourceFromFile(const std::string filename, const ShaderType type);

        /**
         * Add a new texture to use
         */
        void setTexture(const TexturePtr texture, const GLuint textureUnit, const std::string& name);

        /**
         * Set the view projection matrix
         */
        void setModelViewProjectionMatrix(const glm::mat4& mvp);

    private:
        mutable std::mutex _mutex;

        std::map<ShaderType, GLuint> _shaders;
        GLuint _program {0};
        bool _isLinked = {false};
        GLint _locationMVP {0};
        GLint _locationNormalMatrix {0};
        GLint _locationSide {0};
        GLint _locationTextureNbr {0};
        GLint _locationBlendingMap {0};
        GLint _locationBlendWidth {0};
        GLint _locationBlackLevel {0};
        GLint _locationBrightness {0};
        GLint _locationColor {0};
        GLint _locationScale {0};
        GLint _locationLayout {0};

        // Rendering parameters
        Fill _fill {texture};
        Sideness _sideness {doubleSided};
        int _textureNbr {0};
        int _useBlendingMap {0};
        float _blendWidth {0.05f};
        float _blackLevel {0.f};
        float _brightness {1.f};
        glm::vec4 _color {0.0, 1.0, 0.0, 1.0};
        glm::vec3 _scale {1.0, 1.0, 1.0};
        bool _textureOverlap {true};
        std::vector<int> _layout {0, 0, 0, 0};

        /**
         * Compile the shader program
         */
        void compileProgram();

        /**
         * Link the shader program
         */
        bool linkProgram();

        /**
         * Get a string expression of the shader type, used for logging
         */
        std::string stringFromShaderType(ShaderType type);

        /**
         * Replace a shader with an empty one
         */
        void resetShader(ShaderType type);

        /**
         * Register new functors to modify attributes
         */
        void registerAttributes();
};

typedef std::shared_ptr<Shader> ShaderPtr;

} // end of namespace

#endif // SPLASH_SHADER_H
