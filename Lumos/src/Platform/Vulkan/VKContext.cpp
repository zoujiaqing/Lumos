#include "LM.h"
#include "VKContext.h"
#include "VKDevice.h"
#include "VKCommandPool.h"
#include "VKCommandBuffer.h"
#include "Maths/Matrix4.h"

#include <imgui/imgui.h>

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"
#define VK_LAYER_LUNARG_ASSISTENT_LAYER_NAME "VK_LAYER_LUNARG_assistant_layer"
#define VK_LAYER_RENDERDOC_CAPTURE_NAME "VK_LAYER_RENDERDOC_Capture"

namespace Lumos
{
	namespace Graphics
	{
		std::vector<const char*> VKContext::GetRequiredExtensions()
		{
			std::vector<const char*> extensions;

			if (EnableValidationLayers)
			{
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			extensions.push_back("VK_KHR_surface");

	#if defined(_WIN32)
			 extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
			 extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	#elif defined(_DIRECT2DISPLAY)
			 extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
			 extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_XCB_KHR)
			 extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_IOS_MVK)
			 extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_MACOS_MVK)
			 extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
	#endif

			return extensions;
		}

		std::vector<const char*> VKContext::GetRequiredLayers()
		{
			std::vector<const char*> layers;

			if(m_StandardValidationLayer)
				layers.emplace_back(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME);

			if (m_RenderDocLayer)
				layers.emplace_back(VK_LAYER_RENDERDOC_CAPTURE_NAME);

			if (m_AssistanceLayer)
				layers.emplace_back(VK_LAYER_LUNARG_ASSISTENT_LAYER_NAME);
			
			return layers;
		}

		VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
		{
			auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pCallback);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
		{
			auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(
				instance, "vkDestroyDebugReportCallbackEXT"));

			if (func != nullptr)
			{
				func(instance, callback, pAllocator);
			}
		}

        VKContext::VKContext(const WindowProperties& properties, void* deviceContext) : m_VkInstance(nullptr), m_CommandPool(nullptr)
		{
			m_WindowContext = deviceContext;
			CreateInstance();
			SetupDebugCallback();

			Maths::Matrix4::SetUpCoordSystem(false, true);
		}

		VKContext::~VKContext()
		{
			delete m_CommandPool;
            
			DestroyDebugReportCallbackEXT(m_VkInstance, m_DebugCallback, nullptr);
			vkDestroyInstance(m_VkInstance, nullptr);
		}

		void VKContext::Init()
		{
            if(m_CommandPool != nullptr)
                delete m_CommandPool;
            
			m_CommandPool = new VKCommandPool();
		};

		void VKContext::Present()
		{

		}

		void VKContext::Unload()
		{
			delete m_CommandPool;
			m_CommandPool = nullptr;
		}

		VkBool32 VKContext::DebugCallback(VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t sourceObj,
			size_t location,
			int32_t msgCode,
			const char* pLayerPrefix,
			const char* pMsg,
			void* userData)
		{
			// Select prefix depending on flags passed to the callback
			// Note that multiple flags may be set for a single validation message
			// Error that may result in undefined behaviour

			LUMOS_CORE_WARN("[VULKAN] : [{0}] Code {1}  : {2}", pLayerPrefix, msgCode, pMsg);

			if(!flags)
				return VK_FALSE;

			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			{
				LUMOS_CORE_WARN("[VULKAN] - ERROR : [{0}] Code {1}  : {2}", pLayerPrefix, msgCode, pMsg);
			};
			// Warnings may hint at unexpected / non-spec API usage
			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			{
				LUMOS_CORE_WARN("[VULKAN] - WARNING : [{0}] Code {1}  : {2}", pLayerPrefix, msgCode, pMsg);
			};
			// May indicate sub-optimal usage of the API
			if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			{
				LUMOS_CORE_INFO("[VULKAN] - PERFORMANCE : [{0}] Code {1}  : {2}", pLayerPrefix, msgCode, pMsg);
			};
			// Informal messages that may become handy during debugging
			if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			{
				LUMOS_CORE_INFO("[VULKAN] - INFO : [{0}] Code {1}  : {2}", pLayerPrefix, msgCode, pMsg);
			}
			// Diagnostic info from the Vulkan loader and layers
			// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
			if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			{
				LUMOS_CORE_INFO("[VULKAN] - DEBUG : [{0}] Code {1}  : {2}", pLayerPrefix, msgCode, pMsg);
			}

			return VK_FALSE;
		}

		bool VKContext::CheckValidationLayerSupport(const std::vector<const char*> validationLayers)
		{
			uint32_t layerCount;
			vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

			m_InstanceLayers.resize(layerCount);
			vk::enumerateInstanceLayerProperties(&layerCount, m_InstanceLayers.data());

			for (const char* layerName : validationLayers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : m_InstanceLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
				{
					return false;
				}
			}

			return true;
		}

		bool VKContext::CheckExtensionSupport(const std::vector<const char*> extensions)
		{
			uint32_t extensionCount;
			vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

			m_InstanceExtensions.resize(extensionCount);
			vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, m_InstanceExtensions.data());

			for (const char* extensionName : extensions)
			{
				bool layerFound = false;

				for (const auto& layerProperties : m_InstanceExtensions)
				{
					if (strcmp(extensionName, layerProperties.extensionName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
				{
					return false;
				}
			}

			return true;
		}


		size_t VKContext::GetMinUniformBufferOffsetAlignment() const
		{
			return Graphics::VKDevice::Instance()->GetGPUProperties().limits.minUniformBufferOffsetAlignment;
		}

		void VKContext::CreateInstance()
		{
			if (volkInitialize() != VK_SUCCESS)
			{
				LUMOS_CORE_ERROR("volkInitialize failed");
			}

			if (volkGetInstanceVersion() == 0)
			{
				LUMOS_CORE_ERROR("Could not find loader");
			}

			vk::ApplicationInfo appInfo = {};
			appInfo.pApplicationName = "Sandbox";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Lumos";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			vk::InstanceCreateInfo createInfo = {};
			createInfo.pApplicationInfo = &appInfo;

			m_InstanceLayerNames = GetRequiredLayers();
			m_InstanceExtensionNames = GetRequiredExtensions();

			if (EnableValidationLayers && !CheckValidationLayerSupport(m_InstanceLayerNames))
			{
				 LUMOS_CORE_ERROR("[VULKAN] Validation layers requested, but not available!");
			}

			if (!CheckExtensionSupport(m_InstanceExtensionNames))
			{
				LUMOS_CORE_ERROR("[VULKAN] Extensions requested are not available!");
			}

			createInfo.enabledExtensionCount = static_cast<uint32_t>(m_InstanceExtensionNames.size());
			createInfo.ppEnabledExtensionNames = m_InstanceExtensionNames.data();

			createInfo.enabledLayerCount = static_cast<uint32_t>(m_InstanceLayerNames.size());
			createInfo.ppEnabledLayerNames = m_InstanceLayerNames.data();

			m_VkInstance = vk::createInstance(createInfo, nullptr);
			if (!m_VkInstance)
			{
				LUMOS_CORE_ERROR("[VULKAN] Failed to create instance!");
			}

			volkLoadInstance(m_VkInstance);
		}

		void VKContext::SetupDebugCallback()
		{
			if (!EnableValidationLayers) return;

			vk::DebugReportCallbackCreateInfoEXT createInfo = {};
			createInfo.flags =	vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning |
								vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::eDebug | vk::DebugReportFlagBitsEXT::ePerformanceWarning;

			createInfo.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(DebugCallback);

			m_DebugCallback = m_VkInstance.createDebugReportCallbackEXT(createInfo);
			if (!m_DebugCallback)
			{
				LUMOS_CORE_ERROR("[VULKAN] Failed to set up debug callback!");
			}
		}

		void VKContext::OnImGUI()
		{
			ImGui::BeginTabBar("#Vulkaninfo");

			if (ImGui::BeginTabItem("Instance")) 
			{

				ImGui::Text("Extensions:");

				auto globalExtensions = m_InstanceExtensions;
				for (auto const& extension : globalExtensions)
					ImGui::BulletText("%s (%d)", extension,
						VK_VERSION_PATCH(extension.specVersion));

				ImGui::EndTabItem();
			}
		}
	}
}
