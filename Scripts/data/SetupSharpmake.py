import os.path as path
import os

from colorama import Fore, Back, Style
from Utility import Utility

sharpmakeURL = "https://github.com/ubisoft/Sharpmake/releases/download/0.57.0/Sharpmake-net6.0-Windows-0.57.0.zip"

class Sharpmake:
    @staticmethod
    def CheckIfSharpmakeInstalled():
        exists = path.exists("../Sharpmake/Sharpmake.Application.exe")
        return exists

    @staticmethod
    def InstallSharpmake():
        print(Fore.RED + "Sharpmake not found! Installing!")
        Utility.DownloadFile(sharpmakeURL, "sharpmake.zip")
        Utility.UnzipFile("sharpmake.zip", "../Sharpmake")

        os.remove("sharpmake.zip")

        print(Fore.GREEN + "Sharpmake has been installed!")

    @staticmethod
    def CheckSharpmake():
        sharpmakeExists = Sharpmake.CheckIfSharpmakeInstalled()
        if (sharpmakeExists == False):
            Sharpmake.InstallSharpmake()
        else:
            print(Fore.GREEN + "Sharpmake found!")
