import os.path as path
import os

from colorama import Fore, Back, Style
from Utility import Utility

premakeURL = "https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip"

class Premake:
    @staticmethod
    def CheckIfPremakeInstalled():
        exists = path.exists("data/premake5.exe")
        return exists

    @staticmethod
    def InstallPremake():
        print(Fore.RED + "Premake not found! Installing!")
        Utility.DownloadFile(premakeURL, "premake.zip")
        Utility.UnzipFile("premake.zip", "data")

        os.remove("premake.zip")

        print(Fore.GREEN + "Premake has been installed!")

    @staticmethod
    def CheckPremake():
        premakeExists = Premake.CheckIfPremakeInstalled()
        if (premakeExists == False):
            Premake.InstallPremake()
        else:
            print(Fore.GREEN + "Premake found!")
