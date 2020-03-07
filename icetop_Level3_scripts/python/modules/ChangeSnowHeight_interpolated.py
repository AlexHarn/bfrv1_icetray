from icecube import icetray,dataio,dataclasses
from icecube.icetray.i3logging import log_fatal

import csv
import datetime
from scipy import interpolate

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
                if e not in self.newheights:
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
            log_fatal( 'No geometry found')
        self.PushFrame(frame,"OutBox")

    def _convertDate(self, i, s):
        ## Need this function because the strings come in a couple different formats.
        #print("Converting ", s[i])
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
        csvread.__next__()
        ## Save the next line... it contains the dates for each column
        strarray = csvread.__next__()
        
        ### --- Kath replaces the clunky routine with a more graceful one using scipy.interpolate ---
        ## Convert them all to "days since <reference date>"
        refdate = datetime.date(2008,1,1)   # An arbitrary choice
        istart = 10  # Where the "interesting" columns start
        coldate = strarray[:]
        for icol in range(istart,len(strarray)):
            delta = self._convertDate(icol,strarray) - refdate
            coldate[icol] = delta.days
        origlength = len(strarray)
        
        newheights = {}

        for row in csvread:
            if len(row) > 1 :        ## skips empty lines
                #print(row)
                ## First, make a copy of all the original headers and this row
                x = coldate[:]
                y = row[:]
                
                ## Replace "leading dashes" with zeros (but NOT holes in the middle), and
                ## Convert everything else to floats
                replacezeros = True
                for i in range(istart,len(y)):
                  if (y[i] == '-'):
                    if replacezeros:
                      y[i] = 0.0
                  else:
                    y[i] = float(y[i])
                    replacezeros = False   ## Leave the holes there, from here on in.

                ## Now, remove the holes, if there are any.  One at a time.
                nholes = 0
                while '-' in y:
                    removeme = y.index('-')
                    #print("I found a hole in this row.  Removing...")
                    x.pop(removeme)
                    y.pop(removeme)
                    nholes += 1

                ## Check the lengths of things
                if (len(x) != len(y)) or (len(x) != origlength-nholes):
                  log_fatal("SOMETHING WRONG WITH THE LENGTHS!! X=%d, Y=%d"%(len(x), len(y)))
                  
                ## Perform the interpolation
                f = interpolate.interp1d(x[istart:], y[istart:])
                xnew = (filedate - refdate).days
                height = float(f(xnew))    ## So that it's just a number and not an array(<number>)

                ## Grab the station/tank numbers, and load 'em into the result array
                tank = row[0]
                station = int(row[1])
                id = row[2]
                #print("St ", station, id, ": For ", xnewdt, ": Snow = ", height)
                if station not in newheights:
                    newheights[station] = [0,0]
                if id == 'A':
                    newheights[station][0] = height
                elif id == 'B':
                    newheights[station][1] = height
                else:
                    log_fatal('unknown tankid')

        return newheights

