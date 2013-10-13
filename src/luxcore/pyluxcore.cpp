/***************************************************************************
 *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *
 *                                                                         *
 *   This file is part of LuxRays.                                         *
 *                                                                         *
 *   LuxRays is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   LuxRays is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   LuxRays website: http://www.luxrender.net                             *
 ***************************************************************************/

#include <Python.h>
#include <boost/python.hpp>

#include <luxcore/luxcore.h>

using namespace std;
using namespace luxcore;

namespace luxcore {

static const char *LuxCoreVersion() {
	static const char *luxCoreVersion = LUXCORE_VERSION_MAJOR "." LUXCORE_VERSION_MINOR;
	return luxCoreVersion;
}

typedef void (luxrays::Properties::*Properties_Load_Ptr)(luxrays::Properties);

BOOST_PYTHON_MODULE(pyluxcore) {
	using namespace boost::python;

	docstring_options doc_options(
		true,	// Show user defined docstrings
		true,	// Show python signatures
		false	// Show c++ signatures
	);

	//This 'module' is actually a fake package
	object package = scope();
	package.attr("__path__") = "pyluxcore";
	package.attr("__package__") = "pyluxcore";
	package.attr("__doc__") = "New LuxRender Python bindings\n\n"
			"Provides access to the new LuxRender API in python\n\n";

	def("version", LuxCoreVersion, "Returns the LuxCore version");

	//--------------------------------------------------------------------------
	// Property class
	//--------------------------------------------------------------------------

    class_<luxrays::Property>("Property", init<string>())
		.def(init<string, bool>())
		.def(init<string, int>())
		.def(init<string, double>())
		.def(init<string, string>())

		.def("GetName", &luxrays::Property::GetName, return_value_policy<copy_const_reference>())
		.def("GetSize", &luxrays::Property::GetSize)
		.def("Clear", &luxrays::Property::Clear, return_internal_reference<1>())

		.def("Get", &luxrays::Property::Get<bool>)
		.def("Get", &luxrays::Property::Get<int>)
		.def("Get", &luxrays::Property::Get<double>)
		.def("Get", &luxrays::Property::Get<string>)
	
		.def("GetValuesString", &luxrays::Property::GetValuesString)
		.def("ToString", &luxrays::Property::ToString)

		.def("Add", &luxrays::Property::Add<bool>, return_internal_reference<1>())
		.def("Add", &luxrays::Property::Add<int>, return_internal_reference<1>())
		.def("Add", &luxrays::Property::Add<double>, return_internal_reference<1>())
		.def("Add", &luxrays::Property::Add<string>, return_internal_reference<1>())

		.def("Set", &luxrays::Property::Set<bool>, return_internal_reference<1>())
		.def("Set", &luxrays::Property::Set<int>, return_internal_reference<1>())
		.def("Set", &luxrays::Property::Set<double>, return_internal_reference<1>())
		.def("Set", &luxrays::Property::Set<string>, return_internal_reference<1>())

		.def(self_ns::str(self))
    ;

	//--------------------------------------------------------------------------
	// Properties class
	//--------------------------------------------------------------------------

    class_<luxrays::Properties>("Properties", init<>())
		.def(init<string>())

		// Don't ask me why the hell the compiler can deduce the template
		// argument and I have to declare Properties_Load_Ptr
		.def<Properties_Load_Ptr>("Load", &luxrays::Properties::Load)
		.def("LoadFromFile", &luxrays::Properties::LoadFromFile)
		.def("LoadFromString", &luxrays::Properties::LoadFromString)

		.def(self_ns::str(self))
    ;
}

}
