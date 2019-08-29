#pragma once
#include "ECS.h"
#include "ECSDefines.h"
#include "ECS/ISystem.h"
#include "Core/Typename.h"

namespace Lumos
{
    class SystemManager
    {
    public:

        template<typename T>
        Ref<T> RegisterSystem()
        {
			auto typeName = typeid(T).hash_code();
            
            LUMOS_CORE_ASSERT(m_Systems.find(typeName) == m_Systems.end(), "Registering system more than once.");
            
            // Create a pointer to the system and return it so it can be used externally
            Ref<T> system = CreateRef<T>();
            m_Systems.insert({typeName, std::move( system) });
            return system;
        }

		template<typename T>
		Ref<T> RegisterSystem(T* t)
		{
			auto typeName = typeid(T).hash_code();

			LUMOS_CORE_ASSERT(m_Systems.find(typeName) == m_Systems.end(), "Registering system more than once.");

			// Create a pointer to the system and return it so it can be used externally
            Ref<T> system = Ref<T>(t);
            m_Systems.insert({ typeName,std::move( system) });
			return system;
		}

		template<typename T>
		void RemoveSystem()
		{
			auto typeName = typeid(T).hash_code();

			if (m_Systems.find(typeName) != m_Systems.end())
			{
				m_Systems.erase(typeName);
			}
		}

		template<typename T>
		T* GetSystem()
		{
			auto typeName = typeid(T).hash_code();

			if (m_Systems.find(typeName) != m_Systems.end())
			{
				return dynamic_cast<T*>(m_Systems[typeName].get());
			}

			return nullptr;
		}

		template<typename T>
		T* HasSystem()
		{
			auto typeName = typeid(T).hash_code();

			return m_Systems.find(typeName) != m_Systems.end();
		}

		void OnUpdate(TimeStep* dt)
		{
			for (auto& system : m_Systems)
				system.second->OnUpdate(dt);
		}

		void OnImGUI()
		{
			for (auto& system : m_Systems)
				system.second->OnIMGUI();
		}

    private:
        // Map from system type string pointer to a system pointer
        std::unordered_map<size_t, Ref<ISystem>> m_Systems;
    };
}
