/***************************************************************************
 * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <luxcore/luxcore.h>

using namespace std;
using namespace luxcore;
using namespace boost::python;

namespace luxcore {

static const char *LuxCoreVersion() {
	static const char *luxCoreVersion = LUXCORE_VERSION_MAJOR "." LUXCORE_VERSION_MINOR;
	return luxCoreVersion;
}

//------------------------------------------------------------------------------
// Glue for Properties class
//------------------------------------------------------------------------------

static luxrays::Property *Property_InitWithList(const str &name, boost::python::list &l) {
	luxrays::Property *prop = new luxrays::Property(extract<string>(name));

	const boost::python::ssize_t size = len(l);
	for (boost::python::ssize_t i = 0; i < size; ++i) {
		const string objType = extract<string>((l[i].attr("__class__")).attr("__name__"));

		if (objType == "bool") {
			const bool v = extract<bool>(l[i]);
			prop->Add(v);
		} else if (objType == "int") {
			const int v = extract<int>(l[i]);
			prop->Add(v);
		} else if (objType == "float") {
			const double v = extract<double>(l[i]);
			prop->Add(v);
		} else if (objType == "str") {
			const string v = extract<string>(l[i]);
			prop->Add(v);
		} else
			throw std::runtime_error("Unsupported data type included in Property constructor list: " + objType);
	}

	return prop;
}

static boost::python::list Property_GetValuesList(luxrays::Property *prop) {
	boost::python::list l;
	for (u_int i = 0; i < prop->GetSize(); ++i) {
		const std::type_info &tinfo = prop->GetValueType(i);

		if (tinfo == typeid(bool))
			l.append(prop->GetValue<bool>(i));
		else if (tinfo == typeid(int))
			l.append(prop->GetValue<int>(i));
		else if (tinfo == typeid(double))
			l.append(prop->GetValue<double>(i));
		else if (tinfo == typeid(string))
			l.append(prop->GetValue<string>(i));
		else
			throw std::runtime_error("Unsupported data type in list extraction of Property: " + prop->GetName());
	}

	return l;
}

static bool Property_GetBool(luxrays::Property *prop) {
	return prop->GetValue<bool>(0);
}

static int Property_GetInt(luxrays::Property *prop) {
	return prop->GetValue<int>(0);
}

static double Property_GetFloat(luxrays::Property *prop) {
	return prop->GetValue<double>(0);
}

static string Property_GetString(luxrays::Property *prop) {
	return prop->GetValue<string>(0);
}

static boost::python::list Properties_GetAllNames1(luxrays::Properties *props) {
	boost::python::list l;
	const vector<string> &keys = props->GetAllNames();
	BOOST_FOREACH(const string &key, keys) {
		l.append(key);
	}

	return l;
}

static boost::python::list Properties_GetAllNames2(luxrays::Properties *props, const string &prefix) {
	boost::python::list l;
	const vector<string> keys = props->GetAllNames(prefix);
	BOOST_FOREACH(const string &key, keys) {
		l.append(key);
	}

	return l;
}

static boost::python::list Properties_GetAllUniqueSubNames(luxrays::Properties *props, const string &prefix) {
	boost::python::list l;
	const vector<string> keys = props->GetAllUniqueSubNames(prefix);
	BOOST_FOREACH(const string &key, keys) {
		l.append(key);
	}

	return l;
}

static luxrays::Property Properties_GetWithDefaultValues(luxrays::Properties *props,
		const string &name, boost::python::list &l) {
	luxrays::PropertyValues values;
	
	const boost::python::ssize_t size = len(l);
	for (boost::python::ssize_t i = 0; i < size; ++i) {
		const string objType = extract<string>((l[i].attr("__class__")).attr("__name__"));

		if (objType == "bool") {
			const bool v = extract<bool>(l[i]);
			values.push_back(v);
		} else if (objType == "int") {
			const int v = extract<int>(l[i]);
			values.push_back(v);
		} else if (objType == "float") {
			const double v = extract<double>(l[i]);
			values.push_back(v);
		} else if (objType == "str") {
			const string v = extract<string>(l[i]);
			values.push_back(v);
		} else
			throw std::runtime_error("Unsupported data type included in Properties Get with default method: " + objType);
	}

	return luxrays::Property(name, values);
}

//------------------------------------------------------------------------------

BOOST_PYTHON_MODULE(pyluxcore) {
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

	class_<vector<unsigned char> >("UnsignedCharVector")
        .def(vector_indexing_suite<vector<unsigned char> >());
	class_<vector<float> >("FloatVector")
        .def(vector_indexing_suite<vector<float> >());

	//--------------------------------------------------------------------------
	// Property class
	//--------------------------------------------------------------------------

    class_<luxrays::Property>("Property", init<string>())
		.def(init<string, bool>())
		.def(init<string, int>())
		.def(init<string, double>())
		.def(init<string, string>())
		.def("__init__", make_constructor(Property_InitWithList))

		.def("GetName", &luxrays::Property::GetName, return_internal_reference<>())
		.def("GetSize", &luxrays::Property::GetSize)
		.def("Clear", &luxrays::Property::Clear, return_internal_reference<>())

		.def("GetValue", &luxrays::Property::GetValue<bool>)
		.def("GetValue", &luxrays::Property::GetValue<int>)
		.def("GetValue", &luxrays::Property::GetValue<double>)
		.def("GetValue", &luxrays::Property::GetValue<string>)

		.def("GetValues", &Property_GetValuesList)

		.def("GetBool", &Property_GetBool)
		.def("GetInt", &Property_GetInt)
		.def("GetFloat", &Property_GetFloat)
		.def("GetString", &Property_GetString)
	
		.def("GetValuesString", &luxrays::Property::GetValuesString)
		.def("ToString", &luxrays::Property::ToString)

		.def("Add", &luxrays::Property::Add<bool>, return_internal_reference<>())
		.def("Add", &luxrays::Property::Add<int>, return_internal_reference<>())
		.def("Add", &luxrays::Property::Add<double>, return_internal_reference<>())
		.def("Add", &luxrays::Property::Add<string>, return_internal_reference<>())

		.def("Set", &luxrays::Property::Set<bool>, return_internal_reference<>())
		.def("Set", &luxrays::Property::Set<int>, return_internal_reference<>())
		.def("Set", &luxrays::Property::Set<double>, return_internal_reference<>())
		.def("Set", &luxrays::Property::Set<string>, return_internal_reference<>())

		.def(self_ns::str(self))
    ;

	//--------------------------------------------------------------------------
	// Properties class
	//--------------------------------------------------------------------------

    class_<luxrays::Properties>("Properties", init<>())
		.def(init<string>())

		// Required because Properties::Set is overloaded
		.def<luxrays::Properties &(luxrays::Properties::*)(const luxrays::Property &)>
			("Set", &luxrays::Properties::Set, return_internal_reference<>())
		.def<luxrays::Properties &(luxrays::Properties::*)(const luxrays::Properties &)>
			("Set", &luxrays::Properties::Set, return_internal_reference<>())
		.def<luxrays::Properties &(luxrays::Properties::*)(const luxrays::Properties &, const std::string &)>
			("Set", &luxrays::Properties::Set, return_internal_reference<>())
		.def("SetFromFile", &luxrays::Properties::SetFromFile, return_internal_reference<>())
		.def("SetFromString", &luxrays::Properties::SetFromString, return_internal_reference<>())

		.def("Clear", &luxrays::Properties::Clear, return_internal_reference<>())
		.def("GetAllNames", &Properties_GetAllNames1)
		.def("GetAllNames", &Properties_GetAllNames2)
		.def("GetAllUniqueSubNames", &Properties_GetAllUniqueSubNames)
		.def("HaveNames", &luxrays::Properties::HaveNames)

		.def<const luxrays::Property &(luxrays::Properties::*)(const std::string &) const>
			("Get", &luxrays::Properties::Get, return_internal_reference<>())
		.def("Get", &Properties_GetWithDefaultValues)
	
		.def(self_ns::str(self))
    ;

	//--------------------------------------------------------------------------
	// Scene class
	//--------------------------------------------------------------------------

    class_<Scene>("Scene", init<optional<float> >())
		.def(init<string, optional<float> >())
		.def("Parse", &Scene::Parse)
    ;

	//--------------------------------------------------------------------------
	// RenderConfig class
	//--------------------------------------------------------------------------

    class_<RenderConfig>("RenderConfig", init<luxrays::Properties>())
		.def(init<luxrays::Properties, Scene *>()[with_custodian_and_ward<1, 3>()])
		.def("GetProperties", &RenderConfig::GetProperties, return_internal_reference<>())
		.def("GetScene", &RenderConfig::GetScene, return_internal_reference<>())
		.def("Parse", &RenderConfig::Parse)
    ;

	//--------------------------------------------------------------------------
	// RenderSession class
	//--------------------------------------------------------------------------

    class_<RenderSession>("RenderSession", init<RenderConfig *>()[with_custodian_and_ward<1, 2>()])
		.def("Start", &RenderSession::Start)
		.def("Stop", &RenderSession::Stop)
		.def("BeginSceneEdit", &RenderSession::BeginSceneEdit)
		.def("EndSceneEdit", &RenderSession::EndSceneEdit)
		.def("NeedPeriodicFilmSave", &RenderSession::NeedPeriodicFilmSave)
		.def("SaveFilm", &RenderSession::SaveFilm)
		.def("GetScreenBuffer", &RenderSession::GetScreenBuffer)
		.def("UpdateStats", &RenderSession::UpdateStats)
		.def("GetStats", &RenderSession::GetStats, return_internal_reference<>())
    ;
}

}
