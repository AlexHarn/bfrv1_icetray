from icecube import icetray,dataio,dataclasses
from icecube.icetray.i3logging import log_fatal

import csv
import datetime

###############################################################
# Python snowservice :  interpolate from in-situ measurements #
###############################################################
## Since the time of the run is not yet known when the GCD is going through, the user is going to
## have to enter it.
class ChangeSnowHeight_interpolated(icetray.I3Module):
    def __init__(self, ctx):
        icetray.I3Module.__init__(self,ctx)
        self.AddParameter('year','Year', 0)
        self.AddParameter('month','Month', 0)
        self.AddParameter('day','Day', 0)
        self.AddParameter('filename','In-situ measurements spreadsheet (csv)','IT81-MasterwithSnowMeasurements.csv')
        self.AddOutBox("OutBox")

    def Configure(self):
        year = self.GetParameter('year')
        month = self.GetParameter('month')
        day = self.GetParameter('day')
        self.filedate = datetime.date(year, month, day)
        self.snowfile = self.GetParameter('filename')
        self.newheights = self._readFile(self.snowfile, self.filedate)

    def Geometry(self, frame):
        if 'I3Geometry' in frame:
            geom = frame['I3Geometry']
            stageo = geom.stationgeo
            for e,st in stageo:
                updated_heights = dataclasses.I3StationGeo()
                if not self.newheights.has_key(e):
                    log_fatal( 'Did not find station ' + str(e)+ ' in new snowheight dictt')
                    continue
                #ok we have I3TankGeo here... look it up
                ## assume first is A and second is B
                st[0].snowheight = max((self.newheights[e][0],0.))
                st[1].snowheight = max((self.newheights[e][1],0.))
                updated_heights.append(st[0])
                updated_heights.append(st[1])
                stageo[e] = updated_heights
            frame.Delete('I3Geometry')
            frame['I3Geometry'] = geom
        else:
            print 'No geometry found'
        self.PushFrame(frame,"OutBox")

    def _convertDate(self, i, s):
        ## Need this function because the strings come in a couple different formats.
        try:
            d = datetime.datetime.strptime(s[i].strip(),'%b-%y')
        except:
            d = datetime.datetime.strptime(s[i].strip(),'%d-%b-%y')
        return datetime.date(d.year,d.month,d.day)


    def _readFile(self, filename, filedate):
        file = open(filename, "rU")     # Need "universal newline" mode for some reason
        csvread = csv.reader(file,delimiter=',')
        #csvread = csv.reader(file, "rU")
        ## Skip the first line (no matter how many fields are in it) -- it's a header:
        csvread.next()
        ## Save the next line... it contains the dates for each column
        ## Which column is "ours"?
        strarray = csvread.next()
        faftercol=0
        fbeforecol=len(strarray)
        for icol in range(10,len(strarray)):
        #for icol in range(10,24):
#            print strarray[icol]
            coldate = self._convertDate(icol,strarray)
#            print coldate
            if (filedate > coldate):
                faftercol = max(icol,faftercol)
            if (filedate < coldate):
                fbeforecol = min(icol,fbeforecol)

        delta1 = filedate - self._convertDate(faftercol,strarray)
        delta2 = fbeforedate = self._convertDate(fbeforecol,strarray) - self._convertDate(faftercol,strarray)
        fractionalong = float(delta1.days)/float(delta2.days)
#        print strarray[faftercol], " < ", filedate, " < ", strarray[fbeforecol], " ....", fractionalong 
#        print faftercol, fbeforecol

        ## OK, we know which columns to look for, now go get the snowheights
        newheights = {}
        for row in csvread:
            if len(row) > 1 :        ## skips empty lines
                #print row
                ## Read strings
                sheightfirst = row[faftercol]
                sheightsecond = row[fbeforecol]
                ## Convert to floats
                if (sheightfirst == '-'):
                    heightfirst = 0
                else:
                    heightfirst = float(sheightfirst)
                if (sheightsecond == '-'):
                    heightsecond = 0
                else:
                    heightsecond = float(sheightsecond)
                ## Check for negative values, and these changed to zero
                if (heightfirst < 0):
                    heightfirst = 0
                if (heightsecond < 0):
                    heightsecond = 0
                ## Now compute the interpolated snow height
                height = heightfirst+fractionalong*(heightsecond-heightfirst)
                ## Grab the station/tank numbers, and load 'em into the result array
                tank = row[0]
                station = int(row[1])
                id = row[2]
                if station not in newheights:
                    newheights[station] = [0,0]
                if id == 'A':
                    newheights[station][0] = height
                elif id == 'B':
                    newheights[station][1] = height
                else:
                    print 'unknown tankid'
        return newheights
