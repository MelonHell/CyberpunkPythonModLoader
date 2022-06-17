#pragma once

#include "stdafx.h"

struct PyGameObject : RED4ext::CStackType {
	PyGameObject(RED4ext::CBaseRTTIType *type, RED4ext::ScriptInstance value);
	RED4ext::CBaseRTTIType *type;
	RED4ext::ScriptInstance value;

	pybind11::object exec(const std::string& funcName, std::vector<pybind11::object> pyArgs);
	pybind11::object get(const std::string& propName);
	void set(const std::string& propName, const pybind11::object& value);
	[[nodiscard]] std::string getTypeName() const;
	int getTypeType() const;
	std::vector<std::string> getFuncs();
};

PyGameObject ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, std::vector<pybind11::object> pyArgs);
PyGameObject ExecuteFunction(PyGameObject object, const std::string& funcName, std::vector<pybind11::object> pyArgs);
PyGameObject ExecuteGlobalFunction(const std::string& funcName, std::vector<pybind11::object> pyArgs);
PyGameObject GetInstance(const std::string& className);
PyGameObject UnHandle(RED4ext::CStackType object);
PyGameObject GetValuePtr(RED4ext::CProperty *prop, RED4ext::ScriptInstance aInstance);
PyGameObject GetPropValue(PyGameObject object, const std::string& propName);
void SetPropValue(PyGameObject object, const std::string& propName, const pybind11::object& value);
pybind11::object ToPython(PyGameObject object);
RED4ext::CStackType FromPython(const pybind11::object& object, RED4ext::CBaseRTTIType *type);