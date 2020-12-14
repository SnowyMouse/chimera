// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include "../../../output/output.hpp"

#include <vulkan/vulkan.h>

namespace Chimera {
    bool vk_command(int, const char **) noexcept {
        // Remove output prefix
        extern const char *output_prefix;
        output_prefix = nullptr;
        bool loaded_dynamically = false;
        uint32_t device_count = 0;
        
        // Get Vulkan on the line
        auto *vulkan = GetModuleHandle("vulkan-1.dll");
        
        if(!vulkan) {
            vulkan = LoadLibrary("vulkan-1.dll");
            loaded_dynamically = true;
            
            if(vulkan == NULL) {
                console_output("Vulkan is not installed (vulkan-1.dll not found)");
                return true;
            }
            
            console_output("Vulkan is installed (loaded by Chimera)");
        }
        else {
            console_output("Vulkan is installed (loaded by something else)");
        }
        
        // Doing all this because we can't link against Vulkan yet, or else people on old/potato PCs will complain that Chimera won't work.
        
        VkResult __attribute((stdcall)) (*vkCreateInstancePtr)(const VkInstanceCreateInfo *, const VkAllocationCallbacks *, VkInstance *) noexcept = nullptr;
        void __attribute((stdcall)) (*vkDestroyInstancePtr)(VkInstance, const VkAllocationCallbacks *) noexcept = nullptr;
        VkResult __attribute((stdcall)) (*vkEnumeratePhysicalDevicesPtr)(VkInstance, std::uint32_t *, VkPhysicalDevice *) noexcept = nullptr;
        void __attribute((stdcall)) (*vkGetPhysicalDevicePropertiesPtr)(VkPhysicalDevice, VkPhysicalDeviceProperties *) noexcept = nullptr;
        
        bool functions_found = false;
        
        if(!(vkCreateInstancePtr = reinterpret_cast<decltype(vkCreateInstancePtr)>(reinterpret_cast<std::uintptr_t>(GetProcAddress(vulkan, "vkCreateInstance"))))) {
            goto spaghetti_fest;
        }
        
        else if(!(vkDestroyInstancePtr = reinterpret_cast<decltype(vkDestroyInstancePtr)>(reinterpret_cast<std::uintptr_t>(GetProcAddress(vulkan, "vkDestroyInstance"))))) {
            goto spaghetti_fest;
        }
    
        else if(!(vkEnumeratePhysicalDevicesPtr = reinterpret_cast<decltype(vkEnumeratePhysicalDevicesPtr)>(reinterpret_cast<std::uintptr_t>(GetProcAddress(vulkan, "vkEnumeratePhysicalDevices"))))) {
            goto spaghetti_fest;
        }
    
        else if(!(vkGetPhysicalDevicePropertiesPtr = reinterpret_cast<decltype(vkGetPhysicalDevicePropertiesPtr)>(reinterpret_cast<std::uintptr_t>(GetProcAddress(vulkan, "vkGetPhysicalDeviceProperties"))))) {
            goto spaghetti_fest;
        }
        
        else {
            functions_found = true;
            
            // Create an instance
            VkInstance instance;
            VkInstanceCreateInfo create_info = {};
            VkApplicationInfo application_info = {};
            create_info.pApplicationInfo = &application_info;
            application_info.apiVersion = VK_API_VERSION_1_0;
            if(vkCreateInstancePtr(&create_info, nullptr, &instance) == VK_SUCCESS) {
                // Get device count
                if(vkEnumeratePhysicalDevicesPtr(instance, &device_count, nullptr) == VK_SUCCESS) {
                    console_output("%u compatible device%s found", device_count, device_count == 1 ? "" : "s");
                    if(device_count) {
                        std::vector<VkPhysicalDevice> devices(device_count);
                        vkEnumeratePhysicalDevicesPtr(instance, &device_count, devices.data());
                        
                        for(VkPhysicalDevice &device : devices) {
                            VkPhysicalDeviceProperties properties;
                            vkGetPhysicalDevicePropertiesPtr(device, &properties);
                            
                            std::uint32_t version_major = VK_VERSION_MAJOR(properties.apiVersion);
                            std::uint32_t version_minor = VK_VERSION_MINOR(properties.apiVersion);
                            std::uint32_t version_patch = VK_VERSION_PATCH(properties.apiVersion);
                            
                            // Get device type
                            const char *type;
                            switch(properties.deviceType) {
                                case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                                    type = "discrete GPU";
                                    break;
                                case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                                    type = "integrated GPU";
                                    break;
                                default:
                                    type = "unknown";
                                    break;
                            }
                            
                            console_output(" - %s (%s); Vulkan\xAE %u.%u.%u", properties.deviceName, type, version_major, version_minor, version_patch);
                        }
                    }
                }
                else {
                    console_output("Unable to query device count");
                }
                
                // Destruction 1000 the instance
                vkDestroyInstancePtr(instance, nullptr);
            }
        }
        
        spaghetti_fest:
        if(!functions_found) {
            console_output("Functions were missing");
        }
        
        if(loaded_dynamically) {
            FreeLibrary(vulkan);
        }
        
        if(device_count) {
            console_output(ConsoleColor {1.0F, 0.0F, 1.0F, 0.0F}, "Vulkan is supported");
        }
        else {
            console_error("Vulkan is not supported");
        }
        
        return device_count > 0;
    }
}
