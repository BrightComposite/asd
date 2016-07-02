//---------------------------------------------------------------------------

#include <scene/Scene.h>
#include <scene/Camera.h>

//---------------------------------------------------------------------------

namespace Rapture
{
	implement_link(Scene);
	implement_link(SceneObject);
	implement_link(Drawable);
	implement_link(WorldObject);
	implement_link(OrientedObject);

	SceneObject::SceneObject(Scene * scene) : _scene(scene)
	{
		setclass(SceneObject);
		scene->_objects.emplace_back(this);
	}

	Drawable::Drawable(Scene * scene)
	{
		scene->_drawables.push_back(this);
	}

	Scene::Scene(Widget * widget, const string & name) : Scene(name)
	{
		if(widget->graphics() notkindof (Graphics3D))
			throw Exception("Widget for the scene must support 3D graphics!");

		_widget = widget;
		_widget->append<Component>(this);
		connect(*_widget, this, &Scene::onWidgetResize);

		_firstTick = _lastTick = clock::now();
		_ticks = 0;
	}

	Scene::Scene(const string & name) : Named(name)
	{
		setclass(Scene);
	}

	Scene::~Scene()
	{
		disconnect(*_widget, this, &Scene::onWidgetResize);
	}

	Graphics3D & Scene::graphics() const
	{
		return *static_cast<Graphics3D *>(_widget->graphics());
	}

	Widget & Scene::widget() const
	{
		return *_widget;
	}

	UISpace & Scene::space() const
	{
		return *_widget->space();
	}

	Viewport Scene::viewport() const
	{
		return _widget->viewport();
	}

	Camera * Scene::camera() const
	{
		return _camera;
	}

	void Scene::invalidate() const
	{
		space().invalidate(_widget);
	}

	void Scene::render() const
	{
		space().invalidate(_widget);
		space().validate();
	}

	void Scene::setCamera(Camera * camera)
	{
		_camera = camera;
	}

	void Scene::attach(Handle<SceneObject> obj)
	{
		auto & scene = obj->_scene;

		if(scene != nullptr)
		{
			scene->detach(obj);
			scene = this;
		}

		_objects.push_back(obj);

		if(obj kindof(Drawable))
			_drawables.push_back(dynamic_cast<Drawable *>(static_cast<SceneObject *>(obj)));
	}

	void Scene::detach(SceneObject * obj)
	{
		if(obj->_scene != this)
			return;

		for(auto i = _objects.begin(); i != _objects.end(); ++i)
		{
			if(*i == obj)
			{
				_objects.erase(i);
				break;
			}
		}

		if(obj kindof(Drawable))
		{
			auto * drawable = dynamic_cast<Drawable *>(obj);

			for(auto i = _drawables.begin(); i != _drawables.end(); ++i)
			{
				if(*i == drawable)
				{
					_drawables.erase(i);
					break;
				}
			}
		}
	}

	void Camera::setProjectionMode(ProjectionMode mode)
	{
		if(_projectionMode == mode)
			return;

		_projectionMode = mode;
		updateProjection();
	}

	void Camera::updateProjection()
	{
		float aspect = _scene->viewport().ratio();
		float z = 1.0f / _zoom;

		switch(_projectionMode)
		{
			case ProjectionMode::Ortho:
			{
				_scene->graphics().updateUniform<Uniforms::Projection>(fmat::orthot(-1.0f / aspect, 1.0f / aspect, -1.0f, 1.0f, -z, z));
				break;
			}

			case ProjectionMode::Perspective:
			{
				_scene->graphics().updateUniform<Uniforms::Projection>(fmat::perspectivet(_fov, aspect, 0.01f, 2 * z));
				break;
			}
		}
	}

	void Camera::setZoom(float zoom)
	{
		_zoom = zoom;
	}

	void Camera::setFieldOfView(float fov)
	{
		_fov = fov;
	}

	void Scene::onWidgetResize(Handle<WidgetResizeMessage> & msg, Widget & w)
	{
		if(_camera)
			_camera->updateProjection();
	}

	void Scene::draw(Graphics3D & graphics, const IntRect & viewport) const
	{
		for(auto & drawable : _drawables)
			drawable->draw(graphics, viewport, _camera ? _camera->zoom() : 0.0f);
	}

	void Scene::setTickLength(milliseconds length)
	{
		_tickLength = length;
		_firstTick = _lastTick - (_tickLength * _ticks);
	}

	void Scene::update()
	{
		if(clock::now() - _lastTick > _tickLength)
		{
			_lastTick = clock::now();
			ticks_t ticks = (_lastTick - _firstTick) / _tickLength;

			for(; _ticks < ticks; ++_ticks)
			{
				for(auto & obj : _objects)
					obj->update(_ticks);
			}
		}
	}

	void Camera::update()
	{
		_scene->graphics().updateUniform<Uniforms::View>(fmat::lookTo(_pos, _dir.forward(), fvec::up).transpose());
	}
}