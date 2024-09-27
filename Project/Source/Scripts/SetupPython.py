import subprocess
import pkg_resources

class Python:
    @staticmethod
    def Install(package):
        subprocess.check_call(['python', '-m', 'pip', 'install', package])

    @staticmethod
    def ValidatePackage(package):
        required = { package }
        installed = { pkg.key for pkg in pkg_resources.working_set }
        missing = required - installed

        if (missing):
            Python.Install(package)

    @staticmethod
    def CheckPython():
        Python.ValidatePackage('colorama')