#
# compare total cross sections with two different models.
#

import sys
import numpy as np
import math as m

xsecdir = sys.argv[1]
model1 = sys.argv[2]
model2 = sys.argv[3]

#read file

def load_data(model, xsectag) :
    dict = {}
    fname = xsecdir + "/" + model + ".list"
    #print "open ", fname
    for line in open(fname) :
        if line[0] != "#" and len(line.strip())>0 :
            splits = line.split()
            #print splits
            dict[splits[0]] = splits[1]

    #load cc nu
    nbins = 0
    xmin = 0
    xmax = 0
    loge = []
    xseclist = []
    for line in open(xsecdir + "/" + dict[xsectag]) :
        if nbins == 0 :
            splits = line.split()
            nbins = int(splits[0])
            xmin = float(splits[2])
            xmax = float(splits[3])
            loge = np.linspace(xmin, xmax, nbins)
        else :
            xseclist.append(float(line))
    xsecs = np.array(xseclist)

    return loge, xsecs

m1_cc_nu_loge, m1_cc_nu_xsec = load_data(model1, "CC_Nu_iso_xsec")
m1_nc_nu_loge, m1_nc_nu_xsec = load_data(model1, "NC_Nu_iso_xsec")
m1_cc_nubar_loge, m1_cc_nubar_xsec = load_data(model1, "CC_NuBar_iso_xsec")
m1_nc_nubar_loge, m1_nc_nubar_xsec = load_data(model1, "NC_NuBar_iso_xsec")
m2_cc_nu_loge, m2_cc_nu_xsec = load_data(model2, "CC_Nu_iso_xsec")
m2_nc_nu_loge, m2_nc_nu_xsec = load_data(model2, "NC_Nu_iso_xsec")
m2_cc_nubar_loge, m2_cc_nubar_xsec = load_data(model2, "CC_NuBar_iso_xsec")
m2_nc_nubar_loge, m2_nc_nubar_xsec = load_data(model2, "NC_NuBar_iso_xsec")

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt

fig1 = plt.figure(1)
ax1 = fig1.add_subplot(2,2,1)
ax1.plot(m1_cc_nu_loge, m1_cc_nu_xsec, label=model1 + " CCNu", color="red")
ax1.plot(m2_cc_nu_loge, m2_cc_nu_xsec, label=model2 + " CCNu", color="blue")
ax1.legend(loc="best")
ax1.set_yscale("log")

ax2 = fig1.add_subplot(2,2,2)
ax2.plot(m1_cc_nubar_loge, m1_cc_nubar_xsec, label=model1 + " CCNuBar", color="red")
ax2.plot(m2_cc_nubar_loge, m2_cc_nubar_xsec, label=model2 + " CCNuBar", color="blue")
ax2.legend(loc="best")
ax2.set_yscale("log")

ax3 = fig1.add_subplot(2,2,3)
ax3.plot(m1_nc_nu_loge, m1_nc_nu_xsec, label=model1 + " NCNu", color="red")
ax3.plot(m2_nc_nu_loge, m2_nc_nu_xsec, label=model2 + " NCNu", color="blue")
ax3.legend(loc="best")
ax3.set_yscale("log")

ax4 = fig1.add_subplot(2,2,4)
ax4.plot(m1_nc_nubar_loge, m1_nc_nubar_xsec, label=model1 + " NCNuBar", color="red")
ax4.plot(m2_nc_nubar_loge, m2_nc_nubar_xsec, label=model2 + " NCNuBar", color="blue")
ax4.legend(loc="best")
ax4.set_yscale("log")



from scipy import interpolate

loges = np.linspace(1,7, 61)

def interpolation(x, y, loges) :
    fcn = interpolate.interp1d(x,y,kind='linear')  # choose linear, quadratic, cubic
    vals = [fcn(e) for e in loges]
    return np.array(vals)

fig2 = plt.figure(2)
ax11 = fig2.add_subplot(2,4,1)
m1_cc_nu_x = interpolation(m1_cc_nu_loge,m1_cc_nu_xsec,loges)
m2_cc_nu_x = interpolation(m2_cc_nu_loge,m2_cc_nu_xsec,loges)
ax11.plot(loges,m1_cc_nu_x,label=model1+" CCNu", color="red")
ax11.plot(loges,m2_cc_nu_x,label=model2+" CCNu", color="blue")
ax11.set_yscale("log")
ax11.legend(loc="upper left")
ax15 = fig2.add_subplot(2,4,5)
ax15.plot(loges,m1_cc_nu_x/m2_cc_nu_x,label="ratio CCNu", color="red")
ax15.set_xlabel("log10(E)")
ax15.legend(loc="best")

ax12 = fig2.add_subplot(2,4,2)
m1_cc_nubar_x = interpolation(m1_cc_nubar_loge,m1_cc_nubar_xsec,loges)
m2_cc_nubar_x = interpolation(m2_cc_nubar_loge,m2_cc_nubar_xsec,loges)
ax12.plot(loges,m1_cc_nu_x,label=model1+" CCNuBar", color="red")
ax12.plot(loges,m2_cc_nu_x,label=model2+" CCNuBar", color="blue")
ax12.set_yscale("log")
ax12.legend(loc="upper left")
ax16 = fig2.add_subplot(2,4,6)
ax16.plot(loges,m1_cc_nubar_x/m2_cc_nubar_x,label="ratio CCNuBar", color="red")
ax16.set_xlabel("log10(E)")
ax16.legend(loc="best")

ax13 = fig2.add_subplot(2,4,3)
m1_nc_nu_x = interpolation(m1_nc_nu_loge,m1_nc_nu_xsec,loges)
m2_nc_nu_x = interpolation(m2_nc_nu_loge,m2_nc_nu_xsec,loges)
ax13.plot(loges,m1_nc_nu_x,label=model1+" NCNu", color="red")
ax13.plot(loges,m2_nc_nu_x,label=model2+" NCNu", color="blue")
ax13.set_yscale("log")
ax13.legend(loc="upper left")
ax17 = fig2.add_subplot(2,4,7)
ax17.plot(loges,m1_nc_nu_x/m2_nc_nu_x,label="ratio NCNu", color="red")
ax17.set_xlabel("log10(E)")
ax17.legend(loc="best")

ax14 = fig2.add_subplot(2,4,4)
m1_nc_nubar_x = interpolation(m1_nc_nubar_loge,m1_nc_nubar_xsec,loges)
m2_nc_nubar_x = interpolation(m2_nc_nubar_loge,m2_nc_nubar_xsec,loges)
ax14.plot(loges,m1_nc_nu_x,label=model1+" NCNuBar", color="red")
ax14.plot(loges,m2_nc_nu_x,label=model2+" NCNuBar", color="blue")
ax14.set_yscale("log")
ax14.legend(loc="upper left")
ax18 = fig2.add_subplot(2,4,8)
ax18.plot(loges,m1_nc_nubar_x/m2_nc_nubar_x,label="ratio NCNuBar", color="red")
ax18.set_xlabel("log10(E)")
ax18.legend(loc="best")




plt.show()










