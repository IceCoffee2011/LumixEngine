#include "engine/property_register.h"
#include "engine/associative_array.h"
#include "engine/crc32.h"
#include "engine/default_allocator.h"
#include "engine/iproperty_descriptor.h"
#include "engine/log.h"


namespace Lumix
{


namespace PropertyRegister
{


struct ComponentTypeData
{
	char id[50];
	u32 id_hash;
};


typedef AssociativeArray<ComponentType, Array<PropertyDescriptorBase*>> PropertyMap;


static PropertyMap* g_properties = nullptr;
static IAllocator* g_allocator = nullptr;


static Array<ComponentTypeData>& getComponentTypes()
{
	static DefaultAllocator allocator;
	static Array<ComponentTypeData> types(allocator);
	return types;
}


void init(IAllocator& allocator)
{
	ASSERT(!g_properties);
	g_properties = LUMIX_NEW(allocator, PropertyMap)(allocator);
	g_allocator = &allocator;
}


void shutdown()
{
	for (int j = 0; j < g_properties->size(); ++j)
	{
		Array<PropertyDescriptorBase*>& props = g_properties->at(j);
		for (auto* prop : props)
		{
			LUMIX_DELETE(*g_allocator, prop);
		}
	}

	LUMIX_DELETE(*g_allocator, g_properties);
	g_properties = nullptr;
	g_allocator = nullptr;
}


void add(const char* component_type, PropertyDescriptorBase* descriptor)
{
	getDescriptors(getComponentType(component_type)).push(descriptor);
}


Array<PropertyDescriptorBase*>& getDescriptors(ComponentType type)
{
	int props_index = g_properties->find(type);
	if (props_index < 0)
	{
		g_properties->emplace(type, *g_allocator);
		props_index = g_properties->find(type);
	}
	return g_properties->at(props_index);
}


const PropertyDescriptorBase* getDescriptor(ComponentType type, u32 name_hash)
{
	Array<PropertyDescriptorBase*>& props = getDescriptors(type);
	for (int i = 0; i < props.size(); ++i)
	{
		if (props[i]->getNameHash() == name_hash)
		{
			return props[i];
		}
		if (props[i]->getType() == PropertyDescriptorBase::ARRAY)
		{
			auto* array_desc = static_cast<ArrayDescriptorBase*>(props[i]);
			for (auto* child : array_desc->getChildren())
			{
				if (child->getNameHash() == name_hash)
				{
					return child;
				}
			}
		}
	}
	return nullptr;
}


const PropertyDescriptorBase* getDescriptor(const char* component_type, const char* property_name)
{
	return getDescriptor(getComponentType(component_type), crc32(property_name));
}


ComponentType getComponentTypeFromHash(u32 hash)
{
	auto& types = getComponentTypes();
	for (int i = 0, c = types.size(); i < c; ++i)
	{
		if (types[i].id_hash == hash)
		{
			return {i};
		}
	}
	ASSERT(false);
	return {-1};
}


u32 getComponentTypeHash(ComponentType type)
{
	return getComponentTypes()[type.index].id_hash;
}


ComponentType getComponentType(const char* id)
{
	u32 id_hash = crc32(id);
	auto& types = getComponentTypes();
	for (int i = 0, c = types.size(); i < c; ++i)
	{
		if (types[i].id_hash == id_hash)
		{
			return {i};
		}
	}

	auto& cmp_types = getComponentTypes();
	if (types.size() == ComponentType::MAX_TYPES_COUNT)
	{
		g_log_error.log("Engine") << "Too many component types";
		return INVALID_COMPONENT_TYPE;
	}

	ComponentTypeData& type = cmp_types.emplace();
	copyString(type.id, id);
	type.id_hash = id_hash;
	return {getComponentTypes().size() - 1};
}


int getComponentTypesCount()
{
	return getComponentTypes().size();
}


const char* getComponentTypeID(int index)
{
	return getComponentTypes()[index].id;
}


} // namespace PropertyRegister


} // namespace Lumix
