#!/usr/bin/env python
import logging
logging.basicConfig()
rootLogger = logging.getLogger('')
rootLogger.setLevel(logging.DEBUG)

from icecube.simprod.modules import PPCResampleCorsika

if __name__ == '__main__':
   stats = {}
   ptprop = PPCResampleCorsika()
   ptprop.ExecuteOpts(stats)
