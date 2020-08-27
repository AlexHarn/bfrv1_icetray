
import numpy as np
import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt
#import tables
import sys
from matplotlib import gridspec
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.colors import LogNorm

#sys.path.append('/home/sam/IceCube/software/L3_L4/histograms/v1/PythonTools/pyik/')
import mplext as mpl

#from icecube.weighting.weighting import PowerLaw
from icecube.icetray import I3Units

def findbincenters(binedges):
    bincenters=np.zeros(len(binedges)-1)
    for i in range(len(binedges)-1):
        bincenters[i]=binedges[i]+(binedges[i+1]-binedges[i])/2
        bincenters=np.array(bincenters,dtype=np.float)
    return bincenters

def gen_plots_pdf(dataset_dict,options,var_list_1d, hist,y_err,binedges,xlabel,livetime_dict,pdf_file_name='default_pdf.pdf'):
    """ Plots histograms and saves them in a pdf"""

    pdf=PdfPages(pdf_file_name)

    gs = gridspec.GridSpec(2, 1, height_ratios=[3, 1])
    gs.update(hspace=0.05)

    for var in var_list_1d:
        #--------------------
        # PLOT THE HISTOGRAMS
        #--------------------
        y_rel_err={}
        ax= plt.subplot(gs[0])
        for prim in hist.keys():
            
            if np.sum(hist[prim][var])==0:
                print('empty hist', prim, var)
                continue

            mpl.plot_hist(binedges[prim][var], hist[prim][var], livetime_dict[prim],
                          color=dataset_dict[prim]['Color'],
                          label=dataset_dict[prim]['Dataset_label'])

            if options['1DErrorBoxes']=='True':
                mpl.plot_boxerrors(binedges[prim][var], hist[prim][var], 
                y_err[prim][var],
                color=dataset_dict[prim]['Color'],
                alpha=0.4)
                
            y_rel_err[prim] = np.array([nu/de if de!=0 else np.nan for nu,de in zip(y_err[prim][var],hist[prim][var])])


        ax.legend(bbox_to_anchor=(0.5, 1.10),loc='upper center', ncol=2)#, fontsize='x-small')
        ax.set_yscale('log',nonposy='clip')
        #ax.set_yscale('log')
        #ax.set_ylabel('Normalized Histogram')
        ax.grid(b=True,which='both')
        #ax.grid(b=True,which='major')

        plt.setp(ax.get_xticklabels(), visible=False)

        #--------------------------
        # PLOT THE HISTOGRAM RATIOS
        #--------------------------

        ax2=plt.subplot(gs[1],sharex=ax)
        #ax2.grid(b=True,which='both')
        ax2.grid(b=True,which='major')
        ax2.set_xlabel(r'%s'%xlabel[prim][var])


        if options["Reference"] in hist.keys():
            for prim in hist.keys():
                if prim=='Reference':
                    continue
                
                if np.sum(hist[prim][var])==0:
                    print('empty hist', prim, var)
                    continue
            
                # Load numer, denom and bincenters values
                numer=np.array(hist[prim][var].copy()/livetime_dict[prim]) # these would have been normalized above
                denom=np.array(hist[options["Reference"]][var].copy()/livetime_dict[options["Reference"]])
                bincenters = findbincenters(binedges[prim][var])

                # cut out zero bins
                bool = (denom!=0)&(numer!=0)
                numer= np.array(numer[bool],dtype=np.float)
                denom= np.array(denom[bool],dtype=np.float)
                bincenters=bincenters[bool]
                barwidth=binedges[prim][var][1] - binedges[prim][var][0]
                binstarts=binedges[prim][var][:-1][bool]
            
                # calc rel errors for numer and denom
                denom_err= y_rel_err[options["Reference"]][bool]
                numer_err= y_rel_err[prim][bool]

                    
                # calculate ratio and abs error on ratio
                if options['Data_MC_Ratio']=='True':
                    ratio= numer/denom
                    error=np.sqrt(numer_err**2 + denom_err**2) * ratio
                    plt.errorbar(bincenters,ratio,yerr=error,color=dataset_dict[prim]['Color'])
                    plt.axhline(y=1,color='k')
                    plt.ylim([0,2])
                    #plt.yscale('log', nonposy='clip')
                    ax2.set_ylabel('Data/ref')

                if options['Data_MC_Diff']=='True':
                    diff = np.array(hist['Data'][var].copy()) - np.array(hist[prim][var].copy()) 
                    error = y_err[prim][var]
                    boolean = np.isfinite(diff)&np.isfinite(error)&(error>0)
                    diff = diff[boolean]
                    error = error[boolean]
                    temp = findbincenters(binedges[prim][var])[boolean]
                    plt.scatter(temp,diff/error, color=dataset_dict[prim]['Color'],marker='^')
                    #mpl.plot_hist(binedges[prim][var],diff/error, color=dataset_dict[prim]['Color'])
                    #plt.bar(binstarts,diff/error,width=barwidth,edgecolor=dataset_dict[prim]['Color'],alpha=0.6,fill=False)
                    plt.axhline(y=0,color='k')
                    plt.ylim([-10,10])
                    #plt.yscale('symlog',threshy=5.0)
                    ax2.set_ylabel(r'Data-MC/$\sigma_{MC}$')


        pdf.savefig()

        plt.close()

    pdf.close()
    return    
