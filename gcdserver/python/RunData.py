
class RunData(object):
    def __init__(self,
                 configName=None,
                 startTime=None,
                 startFrac=None,
                 stopTime=None,
                 stopFrac=None,
                 runMode=None,
                 lightMode=None,
                 filterMode=None):

        self.configName = configName
        self.startTime = startTime
        self.startFrac = startFrac
        self.stopTime = stopTime
        self.stopFrac = stopFrac
        self.runMode = runMode
        self.lightMode = lightMode
        self.filterMode = filterMode
