import subprocess
import colorama
import os

from colorama import Fore, Back, Style
from Utility import Utility

vulkanURL = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"

class Vulkan:
    @staticmethod
    def ReadVulkanEnvPath():
        subprocess.call("data/VulkanEnvCheck.exe")
        while(True):
            if (Vulkan.IsProcessOpen("VulkanEnvCheck.exe") == False):
                break
    
        f = open("temp", "r")
        var = f.read()
        f.close()
        os.remove("temp")

        return var

    @staticmethod
    def IsProcessOpen(processName):
        progs = str(subprocess.check_output('tasklist'))
        if processName in progs:
            return True
        else:
            return False

    @staticmethod
    def InstallVulkan():
        print(Fore.GREEN + "Downloading Vulkan SDK installer...")
        vulkanExeName = "VulkanSDK.exe"

        Utility.DownloadFile(vulkanURL, vulkanExeName)
        print("Download finished! Running installer!")

        while(True):
            subprocess.call(vulkanExeName, shell=True)

            while(True):
                if (Vulkan.IsProcessOpen(vulkanExeName) == False):
                    break
                
            newVulkanPath = Vulkan.ReadVulkanEnvPath()
            if (newVulkanPath == "None"):
                inputStr = input(Fore.WHITE + "Vulkan did not seem to install properly! Would you like to try again? [" + Fore.GREEN + "Y" + Fore.WHITE + "/" + Fore.RED + "N" + Fore.WHITE + "]:")
                if (inputStr.lower() == "n"):
                    break
            else:
                break

        os.remove(vulkanExeName)
    
    @staticmethod
    def CheckVulkan():
        vulkanPath = Vulkan.ReadVulkanEnvPath()

        if (vulkanPath == "None"):
            print(Fore.RED + "Vulkan SDK not installed! Installing!")
            Vulkan.InstallVulkan()

        elif(vulkanPath.find("1.3.") == -1):
            print(Fore.RED + "Correct Vulkan version not found! Installing 1.3.xxx")
            Vulkan.InstallVulkan()

        else:
            print(Fore.GREEN + "Correct Vulkan version found!")