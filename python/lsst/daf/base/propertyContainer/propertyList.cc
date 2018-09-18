#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "lsst/daf/base/Citizen.h"
#include "lsst/daf/base/PropertyList.h"
#include "lsst/daf/base/DateTime.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace daf {
namespace base {
namespace {

template <typename T, typename C>
void declareAccessors(C& cls, std::string const& name) {
    const std::string getName = "get" + name;
    cls.def(getName.c_str(), (T (PropertyList::*)(std::string const&) const) & PropertyList::get<T>,
            "name"_a);
    cls.def(getName.c_str(), (T (PropertyList::*)(std::string const&, T const&) const) & PropertyList::get<T>,
            "name"_a, "defaultValue"_a);

    // Warning: __len__ is ambiguous so do not attempt to define it. It could return
    // the number of unique names or the number of entries (e.g. as returned by toList,
    // a pure Python method). C++ begin and end iterate over unique names, but users often
    // view PropertyList as a representation of a FITS header. When in doubt, refuse to guess.

    const std::string getArrayName = "getArray" + name;
    cls.def(getArrayName.c_str(),
            (std::vector<T> (PropertyList::*)(std::string const&) const) & PropertyList::getArray<T>,
            "name"_a);

    const std::string setName = "set" + name;
    cls.def(setName.c_str(), (void (PropertyList::*)(std::string const&, T const&)) & PropertyList::set<T>);
    cls.def(setName.c_str(),
            (void (PropertyList::*)(std::string const&, std::vector<T> const&)) & PropertyList::set<T>);
    cls.def(setName.c_str(), (void (PropertyList::*)(std::string const&, T const&, std::string const&)) &
                                     PropertyList::set<T>);
    cls.def(setName.c_str(),
            (void (PropertyList::*)(std::string const&, std::vector<T> const&, std::string const&)) &
                    PropertyList::set<T>);

    const std::string addName = "add" + name;
    cls.def(addName.c_str(), (void (PropertyList::*)(std::string const&, T const&)) & PropertyList::add<T>);
    cls.def(addName.c_str(),
            (void (PropertyList::*)(std::string const&, std::vector<T> const&)) & PropertyList::add<T>);
    cls.def(addName.c_str(), (void (PropertyList::*)(std::string const&, T const&, std::string const&)) &
                                     PropertyList::add<T>);
    cls.def(addName.c_str(),
            (void (PropertyList::*)(std::string const&, std::vector<T> const&, std::string const&)) &
                    PropertyList::add<T>);

    const std::string typeName = "TYPE_" + name;
    cls.attr(typeName.c_str()) = py::cast(typeid(T), py::return_value_policy::reference);
}

}  // <anonymous>

PYBIND11_MODULE(propertyList, mod) {
    py::module::import("lsst.daf.base.persistable");

    py::class_<PropertyList, std::shared_ptr<PropertyList>> cls(mod, "PropertyList");

    cls.def(py::init<>());

    cls.def("nameCount", &PropertyList::nameCount);
    cls.def("names", &PropertyList::names);
    cls.def("getComment", &PropertyList::getComment);
    cls.def("getOrderedNames", &PropertyList::getOrderedNames);
    cls.def("deepCopy", &PropertyList::deepCopy);

    cls.def("exists", &PropertyList::exists);
    cls.def("isArray", &PropertyList::isArray);
    cls.def("valueCount", &PropertyList::valueCount);
    cls.def("typeOf", &PropertyList::typeOf, py::return_value_policy::reference);
    cls.def("toString", &PropertyList::toString, "indent"_a = "");
    cls.def("copy",
            py::overload_cast<std::string const &, PropertySet const &, std::string const &, bool>(
                &PropertyList::copy
            ),
            "dest"_a, "source"_a, "name"_a, "asScalar"_a=false);
    cls.def("copy",
            py::overload_cast<std::string const &, PropertyList const &, std::string const &, bool>(
                &PropertyList::copy
            ),
            "dest"_a, "source"_a, "name"_a, "asScalar"_a=false);
    cls.def("combine", py::overload_cast<PropertySet const &>(&PropertyList::combine));
    cls.def("combine", py::overload_cast<PropertyList const &>(&PropertyList::combine));
    cls.def("remove", &PropertyList::remove);
    cls.def("getAsBool", &PropertyList::getAsBool);
    cls.def("getAsInt", &PropertyList::getAsInt);
    cls.def("getAsInt64", &PropertyList::getAsInt64);
    cls.def("getAsDouble", &PropertyList::getAsDouble);
    cls.def("getAsString", &PropertyList::getAsString);
    cls.def("getAsPersistablePtr", &PropertyList::getAsPersistablePtr);

    declareAccessors<bool>(cls, "Bool");
    declareAccessors<short>(cls, "Short");
    declareAccessors<int>(cls, "Int");
    declareAccessors<long>(cls, "Long");
    declareAccessors<long long>(cls, "LongLong");
    declareAccessors<float>(cls, "Float");
    declareAccessors<double>(cls, "Double");
    declareAccessors<std::string>(cls, "String");
    declareAccessors<DateTime>(cls, "DateTime");

    // unclear why overload_cast doesn't work here, but lambda does
    cls.def("setPropertySet",
            [](PropertyList & self, std::string const & name, PropertySet const & p) { self.set(name, p); });
    cls.def("setPropertyList",
            [](PropertyList & self, std::string const & name, PropertyList const & p) { self.set(name, p); });
}

}  // base
}  // daf
}  // lsst
