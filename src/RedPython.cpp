#include "RedPython.h"

namespace RedPython {

	RED4ext::CStackType ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, const pybind11::tuple &pyArgs) {

		RED4ext::StackArgs_t aArgs;
		std::vector<RED4ext::CString> pyArgsObjects;

		for (int i = 0; i < aFunc->params.size; ++i) {
			if (aFunc->params[i]->type->GetName() == "ScriptGameInstance") {
				RED4ext::ScriptGameInstance scriptGameInstance;
				aArgs.push_back({nullptr, &scriptGameInstance});
			}
		}

		for (const auto &pyArg: pyArgs) {
			aArgs.push_back(FromPython(pyArg, aFunc->params[aArgs.size()]->type));
		}

		RED4ext::CStackType result;
		if (aFunc->returnType) {
			result.type = aFunc->returnType->type;
			auto allocator = aFunc->returnType->type->GetAllocator();
			auto allocResult = allocator->Alloc(aFunc->returnType->type->GetSize());
			aFunc->returnType->type->Construct(allocResult.memory);
			result.value = allocResult.memory;
		}
		for (size_t i = 0; i < aArgs.size() && i < aFunc->params.size; i++) {
			auto &arg = aArgs[i];
			if (!arg.type) arg.type = aFunc->params[static_cast<uint32_t>(i)]->type;
		}
		RED4ext::CStack stack(aInstance, aArgs.data(), static_cast<uint32_t>(aArgs.size()), aFunc->returnType ? &result : nullptr);
		aFunc->Execute(&stack);
		return UnHandle(result);
	}

	RED4ext::CStackType ExecuteFunction(RED4ext::CStackType object, const std::string &funcName, const pybind11::tuple &pyArgs) {
		auto rtti = RED4ext::CRTTISystem::Get();
		auto cls = rtti->GetClass(object.type->GetName());
		auto aFunc = cls->GetFunction(funcName.c_str());
		return ExecuteFunction(object.value, aFunc, pyArgs);
	}

	RED4ext::CStackType GetInstance(const std::string &className) {
		auto rtti = RED4ext::CRTTISystem::Get();
		auto type = rtti->GetClass(className.c_str());
		auto engine = RED4ext::CGameEngine::Get();
		auto game = engine->framework->gameInstance;
		RED4ext::Handle<RED4ext::IScriptable> instance(game->GetInstance(type));
		return UnHandle({instance->GetType(), instance.GetPtr()});
	}

	RED4ext::CStackType ExecuteGlobalFunction(const std::string &funcName, const pybind11::tuple &pyArgs) {
		auto rtti = RED4ext::CRTTISystem::Get();
		RED4ext::CBaseFunction *func = rtti->GetFunction(funcName.c_str());
		return ExecuteFunction(GetInstance("cpPlayerSystem").value, func, pyArgs);
	}

	RED4ext::CStackType UnHandle(RED4ext::CStackType object) {
		if (object.type && object.value) {
			if (object.type->GetType() == RED4ext::ERTTIType::Handle) {
				RED4ext::Handle<RED4ext::IScriptable> handle = *static_cast<RED4ext::Handle<RED4ext::IScriptable> *>(object.value);
				return {handle->GetType(), handle.GetPtr()};
			}
		}
		return {object.type, object.value};
	}

	RED4ext::CStackType GetValuePtr(RED4ext::CProperty *prop, RED4ext::ScriptInstance aInstance) {
		void *holder = aInstance;
		if (prop->flags.b21) {
			using func_t = void *(*)(RED4ext::ScriptInstance);
			RED4ext::RelocFunc<func_t> func(RED4ext::Addresses::IScriptable_GetValueHolder);
			holder = func(aInstance);
		}
		void *value = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(holder) + prop->valueOffset);
		return {prop->type, value};
	}

	RED4ext::CStackType GetPropValue(RED4ext::CStackType object, const std::string &propName) {
		auto rtti = RED4ext::CRTTISystem::Get();
		auto cls = rtti->GetClass(object.type->GetName());
		auto prop = cls->GetProperty(propName.c_str());
		return GetValuePtr(prop, object.value);
	}

	void SetPropValue(RED4ext::CStackType object, const std::string &propName, const pybind11::handle &value) {
		auto rtti = RED4ext::CRTTISystem::Get();
		auto cls = rtti->GetClass(object.type->GetName());
		auto prop = cls->GetProperty(propName.c_str());
		auto prevValue = GetValuePtr(prop, object.value);
		auto newValue = FromPython(value, prop->type);
		prop->type->Assign(prevValue.value, newValue.value);
	}

	pybind11::object ToPython(RED4ext::CStackType object) {
		if (object.type == nullptr || object.value == nullptr) return pybind11::none();
		if (object.type->GetType() == RED4ext::ERTTIType::Class) {
			auto pyObj = pybind11::cast(object);
			auto rtti = RED4ext::CRTTISystem::Get();
			auto cls = (RED4ext::CClass *) object.type;
			RED4ext::DynArray<RED4ext::CBaseFunction *> funcs;
			rtti->GetClassFunctions(funcs);
			for (const auto &func: funcs) {
				if (cls->GetFunction(func->fullName)) {
					setattr(pyObj, func->shortName.ToString(), pybind11::cpp_function([object, &func](const pybind11::args &pyArgs) {
						return ExecuteFunction(object, func->fullName.ToString(), pyArgs);
					}));
				}
			}
			return pyObj;
		}
		if (object.type->GetType() == RED4ext::ERTTIType::Enum) return pybind11::cast((int32_t *) object.value);
		if (object.type->GetType() == RED4ext::ERTTIType::Fundamental) {
			if (object.type->GetName() == "Bool") return pybind11::cast((bool *) object.value);
			if (object.type->GetName() == "Int8") return pybind11::cast((int8_t *) object.value);
			if (object.type->GetName() == "Uint8") return pybind11::cast((uint8_t *) object.value);
			if (object.type->GetName() == "Int16") return pybind11::cast((int16_t *) object.value);
			if (object.type->GetName() == "Uint16") return pybind11::cast((uint16_t *) object.value);
			if (object.type->GetName() == "Int32") return pybind11::cast((int32_t *) object.value);
			if (object.type->GetName() == "Uint32") return pybind11::cast((uint32_t *) object.value);
			if (object.type->GetName() == "Int64") return pybind11::cast((int64_t *) object.value);
			if (object.type->GetName() == "Uint64") return pybind11::cast((uint64_t *) object.value);
			if (object.type->GetName() == "Float") return pybind11::cast((float *) object.value);
			if (object.type->GetName() == "Double") return pybind11::cast((double *) object.value);
		}
		MessageBoxA(nullptr, (std::string(object.type->GetName().ToString()) + " " + std::to_string((int) object.type->GetType())).c_str(), "Unknown Type", 0);
		return pybind11::none();
	}

	RED4ext::CStackType FromPython(const pybind11::handle &object, RED4ext::CBaseRTTIType *type) {
		if (type->GetType() == RED4ext::ERTTIType::Class) return object.cast<RED4ext::CStackType>();
		std::any value;
		if (type->GetType() == RED4ext::ERTTIType::Enum) value = object.cast<int32_t>();
		if (type->GetType() == RED4ext::ERTTIType::Fundamental) {
			if (type->GetName() == "Bool") value = object.cast<bool>();
			if (type->GetName() == "Int8") value = object.cast<int8_t>();
			if (type->GetName() == "Uint8") value = object.cast<uint8_t>();
			if (type->GetName() == "Int16") value = object.cast<int16_t>();
			if (type->GetName() == "Uint16") value = object.cast<uint16_t>();
			if (type->GetName() == "Int32") value = object.cast<int32_t>();
			if (type->GetName() == "Uint32") value = object.cast<uint32_t>();
			if (type->GetName() == "Int64") value = object.cast<int64_t>();
			if (type->GetName() == "Uint64") value = object.cast<uint64_t>();
			if (type->GetName() == "Float") value = object.cast<float>();
			if (type->GetName() == "Double") value = object.cast<double>();
		}
		if (type->GetType() == RED4ext::ERTTIType::Simple) {
			if (type->GetName() == "String") value = RED4ext::CString(pybind11::str(object).cast<std::string>().c_str());
		}
		auto allocResult = type->GetAllocator()->Alloc(type->GetSize());
		type->Assign(allocResult.memory, &value);
		return {type, allocResult.memory};
	}
}