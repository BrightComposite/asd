//---------------------------------------------------------------------------

#pragma once

#ifndef GROUND_OBJECT_H
#define GROUND_OBJECT_H

//---------------------------------------------------------------------------

#include <physics/Physics.h>

//---------------------------------------------------------------------------

namespace Rapture
{
	class PlaneObject; 

	link_class(physics, PlaneObject, Class<Physical>);

	class PlaneObject : public Physical
	{
	public:
		PlaneObject(Scene * scene, PhysicalWorld * world, float level) :
			Physical
			(
				scene->append<OrientedObject>(fvec{0.0f, level, 0.0f}, fquat{floatv::right, fmath::half_pi}),
				world,
				new btStaticPlaneShape({0, 1, 0}, 0)
			)
		{
			setclass(PlaneObject);
		}

		virtual ~PlaneObject() {}
	};
}

//---------------------------------------------------------------------------
#endif
