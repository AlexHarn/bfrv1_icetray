#!/usr/bin/env python
import logging
logging.basicConfig()
rootLogger = logging.getLogger('')
rootLogger.setLevel(logging.DEBUG)

from icecube.simprod.modules import ClSimResampleCorsika

if __name__ == '__main__':
   stats = {}
   ptprop = ClSimResampleCorsika()
   ptprop.ExecuteOpts(stats)
