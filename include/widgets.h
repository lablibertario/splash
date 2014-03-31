/*
 * Copyright (C) 2014 Emmanuel Durand
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
 * @widgets.h
 * The splash widgets
 */

#ifndef WIDGETS_H
#define WIDGETS_H

#define SPLASH_GLV_TEXTCOLOR 1.0, 1.0, 1.0
#define SPLASH_GLV_FONTSIZE 8.0
#define SPLASH_GLV_FONTWIDTH 2.0

#define GLFW_NO_GLU
#define GL_GLEXT_PROTOTYPES

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <GLFW/glfw3.h>
#include <glv.h>

#include "camera.h"
#include "object.h"
#include "texture.h"

namespace Splash
{
class Gui;
class Scene;
typedef std::weak_ptr<Scene> SceneWeakPtr;

/*************/
class GlvTextBox : public glv::View
{
    public:
        void onDraw(glv::GLV& g);
        bool onEvent(glv::Event::t e, glv::GLV& g);
        void setTextFunc(std::function<std::string(GlvTextBox& that)> func) {getText = func;}

        float fontSize {8};
        float lineSpacing {1};
        std::atomic_int _scrollOffset {0};

    private:
        std::function<std::string(GlvTextBox& that)> getText;
};

/*************/
class GlvControl : public glv::View
{
    public:
        GlvControl();
        void onDraw(glv::GLV& g);
        bool onEvent(glv::Event::t e, glv::GLV& g);
        void setScene(SceneWeakPtr scene) {_scene = scene;}

    private:
        SceneWeakPtr _scene;
        bool _ready {false};

        int _objIndex {0};
        bool _isDistant {false};
        glv::Label _selectedObjectName;
        glv::View _left, _right;
        glv::Placer _titlePlacer;

        std::vector<glv::Label*> _properties;
        std::vector<glv::NumberDialer*> _numbers;

        std::string getNameByIndex();
        void changeTarget(std::string name);
};

/*************/
class GlvGlobalView : public glv::View3D
{
    friend Gui;
    public:
        GlvGlobalView();
        void onDraw(glv::GLV& g);
        bool onEvent(glv::Event::t e, glv::GLV& g);
        void setScene(SceneWeakPtr scene) {_scene = scene;}
        void setCamera(CameraPtr cam);
        void setObject(ObjectPtr obj) {_camera->linkTo(obj);}

    protected:
        int _baseWidth {800};
        CameraPtr _camera, _guiCamera;
        SceneWeakPtr _scene;

        bool _camerasHidden {false};

        // Store the previous camera values
        std::vector<Value> _eye, _target, _up, _fov, _principalPoint;

        // Previous point added
        std::vector<Value> _previousPointAdded;

        glv::Label _camLabel;
};

/*************/
class GlvGraph : public glv::View
{
    public:
        GlvGraph();
        void onDraw(glv::GLV& g);
        bool onEvent(glv::Event::t e, glv::GLV& g);
        void onResize(glv::space_t dx, glv::space_t dy);

    private:
        glv::Plot _plot;
        glv::PlotFunction1D _plotFunction;
        glv::Label _graphLabel, _scaleLabel;
        glv::Style _style;

        std::atomic_uint _target {0};

        unsigned int _maxHistoryLength {500};
        std::map<std::string, std::deque<unsigned long long>> _durationGraph;
};

} // end of namespace

#endif // WIDGETS_H
