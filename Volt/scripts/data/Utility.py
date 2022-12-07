import sys
import zipfile
import urllib.request as urlReq

from colorama import Fore, Back, Style

class Utility:
    @staticmethod
    def ProgressBar(a, b, c):
        progressBarLength = 34
        progress = (progressBarLength * a * b / c)
    
        sys.stdout.write('\r')
        sys.stdout.write('[')
    
        for i in range(int(progress)):
            sys.stdout.write('█')

        for i in range(progressBarLength - int(progress)):
            sys.stdout.write('-')
        sys.stdout.write(']')
        sys.stdout.write(f"{round(min(100*a*b/c, 100.0), 1)}%" + " / 100%")

    @staticmethod
    def DownloadFile(url, dlPath):
        opener = urlReq.build_opener()
        opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36')]
        urlReq.install_opener(opener)
        sys.stdout.write(Fore.GREEN)
        result = urlReq.urlretrieve(url, dlPath, Utility.ProgressBar)
        print("")
        return result

    @staticmethod
    def UnzipFile(source, destination):
        with zipfile.ZipFile(source, 'r') as zipObj:
            zipObj.extract("premake5.exe", destination)