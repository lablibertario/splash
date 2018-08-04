#include "./core/graph_object.h"

#include <algorithm>

#include "./core/root_object.h"

using namespace std;

namespace Splash
{

/*************/
GraphObject::GraphObject(RootObject* root)
    : _root(root)
{
    initializeTree();
    registerAttributes();
}

/*************/
GraphObject::~GraphObject()
{
    uninitializeTree();
}

/*************/
Attribute& GraphObject::operator[](const string& attr)
{
    auto attribFunction = _attribFunctions.find(attr);
    return attribFunction->second;
}

/*************/
Attribute& GraphObject::addAttribute(const string& name, const function<bool(const Values&)>& set, const vector<char>& types)
{
    return BaseObject::addAttribute(name, set, types);
}

/*************/
Attribute& GraphObject::addAttribute(const string& name, const function<bool(const Values&)>& set, const function<const Values()>& get, const vector<char>& types)
{
    auto& attribute = BaseObject::addAttribute(name, set, get, types);
    initializeTree();
    return attribute;
}

/*************/
void GraphObject::linkToParent(GraphObject* obj)
{
    auto parentIt = find(_parents.begin(), _parents.end(), obj);
    if (parentIt == _parents.end())
        _parents.push_back(obj);
    return;
}

/*************/
void GraphObject::unlinkFromParent(GraphObject* obj)
{
    auto parentIt = find(_parents.begin(), _parents.end(), obj);
    if (parentIt != _parents.end())
        _parents.erase(parentIt);
    return;
}

/*************/
bool GraphObject::linkTo(const shared_ptr<GraphObject>& obj)
{
    auto objectIt = find_if(_linkedObjects.begin(), _linkedObjects.end(), [&](const weak_ptr<GraphObject>& o) {
        auto object = o.lock();
        if (!object)
            return false;
        if (object == obj)
            return true;
        return false;
    });

    if (objectIt == _linkedObjects.end())
    {
        _linkedObjects.push_back(obj);
        obj->linkToParent(this);
        return true;
    }
    return false;
}

/*************/
void GraphObject::unlinkFrom(const shared_ptr<GraphObject>& obj)
{
    auto objectIt = find_if(_linkedObjects.begin(), _linkedObjects.end(), [&](const weak_ptr<GraphObject>& o) {
        auto object = o.lock();
        if (!object)
            return false;
        if (object == obj)
            return true;
        return false;
    });

    if (objectIt != _linkedObjects.end())
    {
        _linkedObjects.erase(objectIt);
        obj->unlinkFromParent(this);
    }
}

/*************/
Json::Value GraphObject::getConfigurationAsJson() const
{
    Json::Value root;
    if (_remoteType == "")
        root["type"] = _type;
    else
        root["type"] = _remoteType;

    for (auto& attr : _attribFunctions)
    {
        Values values;
        if (getAttribute(attr.first, values) == false || values.size() == 0)
            continue;

        Json::Value jsValue;
        jsValue = getValuesAsJson(values);
        root[attr.first] = jsValue;
    }
    return root;
}

/*************/
unordered_map<string, Values> GraphObject::getDistantAttributes() const
{
    unordered_map<string, Values> attribs;
    for (auto& attr : _attribFunctions)
    {
        if (!attr.second.doUpdateDistant())
            continue;

        Values values;
        if (getAttribute(attr.first, values, false, true) == false || values.size() == 0)
            continue;

        attribs[attr.first] = values;
    }

    return attribs;
}

/*************/
void GraphObject::setName(const string& name)
{
    if (name.empty())
        return;

    auto oldName = _name;
    _name = name;

    if (!_root)
        return;

    if (oldName.empty())
    {
        initializeTree();
    }
    else
    {
        auto& tree = _root->getTree();
        auto path = "/" + _root->getName() + "/objects/" + oldName;
        if (tree.hasBranchAt(path))
            tree.renameBranchAt(path, name);
    }
}

/*************/
bool GraphObject::setRenderingPriority(Priority priority)
{
    if (priority < Priority::PRE_CAMERA || priority >= Priority::POST_WINDOW)
        return false;
    _renderingPriority = priority;
    return true;
}
/*************/
void GraphObject::registerAttributes()
{
    addAttribute("alias",
        [&](const Values& args) {
            auto alias = args[0].as<string>();
            setAlias(alias);
            return true;
        },
        [&]() -> Values { return {getAlias()}; },
        {'s'});
    setAttributeDescription("alias", "Alias name");

    addAttribute("setSavable",
        [&](const Values& args) {
            auto savable = args[0].as<bool>();
            setSavable(savable);
            return true;
        },
        {'n'});

    addAttribute("priorityShift",
        [&](const Values& args) {
            _priorityShift = args[0].as<int>();
            return true;
        },
        [&]() -> Values { return {_priorityShift}; },
        {'n'});
    setAttributeDescription("priorityShift",
        "Shift to the default rendering priority value, for those cases where two objects should be rendered in a specific order. Higher value means lower priority");

    addAttribute("switchLock",
        [&](const Values& args) {
            auto attribIterator = _attribFunctions.find(args[0].as<string>());
            if (attribIterator == _attribFunctions.end())
                return false;

            string status;
            auto& attribFunctor = attribIterator->second;
            if (attribFunctor.isLocked())
            {
                status = "Unlocked";
                attribFunctor.unlock();
            }
            else
            {
                status = "Locked";
                attribFunctor.lock();
            }

            Log::get() << Log::MESSAGE << _name << "~~" << args[0].as<string>() << " - " << status << Log::endl;
            return true;
        },
        {'s'});
}

/*************/
void GraphObject::initializeTree()
{
    if (!_root || _name.empty())
        return;

    auto& tree = _root->getTree();
    auto path = "/" + _root->getName() + "/objects/" + _name;

    if (!tree.hasBranchAt(path))
        tree.createBranchAt(path);
    if (!tree.hasBranchAt(path + "/attributes"))
        tree.createBranchAt(path + "/attributes");

    // Create the leaves for the attributes in the tree
    path = path + "/attributes/";
    for (const auto& attribute : _attribFunctions)
    {
        if (!attribute.second.hasGetter())
            continue;
        auto attributeName = attribute.first;
        auto leafPath = path + attributeName;
        if (tree.hasLeafAt(leafPath))
            continue;
        if (!tree.createLeafAt(leafPath))
            throw runtime_error("Error while adding a leaf at path " + leafPath);

        auto leaf = tree.getLeafAt(leafPath);
        _treeCallbackIds[attributeName] = leaf->addCallback([=](const Value& value, const chrono::system_clock::time_point& /*timestamp*/) {
            auto attribIt = _attribFunctions.find(attributeName);
            if (attribIt == _attribFunctions.end())
                return;
            if (value == attribIt->second())
                return;
            setAttribute(attributeName, value.as<Values>());
        });
    }

    // Remove leaves for attributes which do not exist anymore
    auto leafList = tree.getBranchAt(path)->getLeafList();
    for (const auto& leafName : leafList)
    {
        if (_attribFunctions.find(leafName) != _attribFunctions.end())
            continue;
        tree.removeLeafAt(path + leafName);
    }
}

/*************/
void GraphObject::uninitializeTree()
{
    if (!_root || _name.empty())
        return;

    auto& tree = _root->getTree();
    auto path = "/" + _root->getName() + "/objects/" + _name;
    if (tree.hasBranchAt(path))
        tree.removeBranchAt(path);
}

} // namespace Splash
