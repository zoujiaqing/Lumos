#pragma once
#include "lmpch.h"

namespace Lumos
{
    namespace Graphics
    {
        class TextureCube;
        class Texture;

        class Environment
        {
        public:
            Environment();
            Environment(TextureCube* env)
            {
                m_Environmnet = Ref<TextureCube>(env);
                m_PrefilteredEnvironment = nullptr;
                m_IrradianceMap = nullptr;
            }
        
            Environment(TextureCube* env, TextureCube* irr)
            {
                m_Environmnet = Ref<TextureCube>(env);
                m_IrradianceMap = Ref<TextureCube>(irr);
                m_PrefilteredEnvironment = nullptr;
            }
        
            Environment(const String& filepath, bool genPrefilter, bool genIrradiance);
            Environment(const String& name, u32 numMip, u32 width, u32 height, const String& fileType = ".tga");
            ~Environment();

            TextureCube* GetEnvironmentMap() const { return m_Environmnet.get(); }
            TextureCube* GetPrefilteredMap() const { return m_PrefilteredEnvironment.get(); }
            TextureCube* GetIrradianceMap() const { return m_IrradianceMap.get(); }
        
            void SetEnvironmnet(TextureCube *environmnet);
            void SetPrefilteredEnvironment(TextureCube *prefilteredEnvironment);
            void SetIrradianceMap(TextureCube *irradianceMap);
        
        private:

            Ref<TextureCube> m_Environmnet;
            Ref<TextureCube> m_PrefilteredEnvironment;
            Ref<TextureCube> m_IrradianceMap;
        };
    }
}
