# IPython log file

import numpy as np
try:
   import cPickle as pickle
except:
   import pickle
import os, sys
from argparse import ArgumentParser
from icecube.icetop_Level3_scripts.histograms import Livetime
from icecube import icetray

parser = ArgumentParser(usage='%s -i <newhistos.pickle> -r <oldhistos.pickle> --inlivetime <newlivetime.pickle> -- reflivetime <oldlivetime.pickle> [arguments] -o <outplots.pdf>'%os.path.basename(sys.argv[0]))
parser.add_argument("--with-cuts", action="store_true", default=False, dest="with_cuts", help="Plot the distributions after cuts.")
parser.add_argument("-o","--outfile",action="store",type=str, dest="outfile",help="Outfile pdf name which will store the plots.")
parser.add_argument("-i","--infile",action="store",type=str, dest="infile",help="Input pickle file which will be compared to the ref pickle file.")
parser.add_argument("-r","--reffile",action="store",type=str, dest="reffile",help="Reference pickle file.")
parser.add_argument("--inlegend",action="store",type=str, dest="inleg",help="Input legend",default="input")
parser.add_argument("--reflegend",action="store",type=str, dest="refleg",help="Reference legend",default="reference")
parser.add_argument("--inlivetime",action="store",type=str, dest="inlivetime",help="Livetime pickle file for the input.",default=None)
parser.add_argument("--reflivetime",action="store",type=str, dest="reflivetime",help="Livetime pickle file for the reference.",default=None)

(args) = parser.parse_args()

if not (args.infile and args.reffile and args.outfile):
    parser.print_help()
    exit(0)

def findbincenters(binedges):
    bincenters=np.zeros(len(binedges)-1)
    for i in range(len(binedges)-1):
        bincenters[i]=binedges[i]+(binedges[i+1]-binedges[i])/2
        bincenters=np.array(bincenters,dtype=np.float)
    return bincenters
    
def fitLivetime(livetime_pickleFile):
    from ROOT import TH1F, TF1, TFile, TObject
    hist_dict=pickle.load(open(livetime_pickleFile, 'rb'), encoding='latin-1')
    ## In new files, there is a 'filelist' at the end here, so do not do this check.
    ## Instead, we'll just look for the element called "Livetime"
    #if len(hist_dict.keys())>1:
    #    icetray.logging.log_fatal("There should only be the livetime in here! File: %s"%livetime_pickleFile)

    ltobj = hist_dict['Livetime']
    if type(ltobj) is dict:  ## The new py3 style?
        lifetime_counts= np.array(ltobj['bin_values'])
        temp, binedges = np.histogram([],range = [ltobj['xmin'], ltobj['xmax']], bins = len(ltobj['bin_values']))
    else:
        lifetime_counts= np.array(ltobj.bin_values)
        temp, binedges = np.histogram([],range = [ltobj.xmin, ltobj.xmax], bins = len(ltobj.bin_values))

    bincenters=findbincenters(binedges)

    # I'll do this fitting in ROOT. If someone wants to convert this to python/scipy/gnuplot... go ahead.                                                                                                   
    hist=TH1F("hist","hist",len(bincenters),binedges[0],binedges[-1])
    for i in range(0,len(bincenters)):
        hist.Fill(bincenters[i],lifetime_counts[i])

    fit=TF1("fit","[0]*TMath::Exp(-x/[1])",binedges[0],binedges[-1])
    fit.SetParameters(lifetime_counts[0],1.)

    hist.Fit("fit","Q")
    rootoutfile=TFile("livetimes.root","UPDATE")
    hist.Write(livetime_pickleFile[:-6],TObject.kOverwrite)
    rootoutfile.Close()
    tau = fit.GetParameter(1)
    tot_lifetime=tau*sum(lifetime_counts)
    print(tot_lifetime)
    return tot_lifetime

hist = {}
binedges = {}
xlabel={}
yerr={}
dataset_dict={}

pickleFile=args.infile
hist_dict=pickle.load(open(pickleFile, 'rb'), encoding='latin-1')
vars_list_1d = hist_dict.keys() 
dataset=args.inleg

hist[dataset] = {}
binedges[dataset] = {}
xlabel[dataset]={}
yerr[dataset]={}

for key in hist_dict.keys():
    if not (key == 'filelist'):   ## New pickle files have this in there for some reason, old ones don't
        obj = hist_dict[key]
        if type(obj) is dict:  ## The new py3 style?
            #print("Found new style: ", dataset, key, type(obj), obj)
            hist[dataset][key] = np.array(obj['bin_values'])
            temp, binedges[dataset][key] = np.histogram([],range = [obj['xmin'], obj['xmax']], bins = len(obj['bin_values']))
            xlabel[dataset][key]=key # No expression histogram
            yerr[dataset][key] = np.ones_like(obj['bin_values'])
        else: # isinstance(key, icecube.icetop_Level3_scripts.histograms.getLivetime.Livetime):  ## The old py2 style?
            #print("Found old style: ", dataset, key, type(obj), obj)
            hist[dataset][key] = np.array(obj.bin_values)
            temp, binedges[dataset][key] = np.histogram([],range = [obj.xmin, obj.xmax], bins = len(obj.bin_values))
            xlabel[dataset][key]=key # No expression histogram
            yerr[dataset][key] = np.ones_like(obj.bin_values)
    else:
        print(dataset, ": Skipping the filelist at the end which is not a histogram")

dataset_dict[dataset]={}
dataset_dict[dataset]['Dataset_label']=dataset
dataset_dict[dataset]['Color']='r'

pickleFile_ref=args.reffile

hist_dict_ref=pickle.load(open(pickleFile_ref, 'rb'), encoding='latin-1')
vars_list_1d_ref = hist_dict_ref.keys()
dataset_ref=args.refleg

hist[dataset_ref] = {}
binedges[dataset_ref] = {}
xlabel[dataset_ref]={}
yerr[dataset_ref]={}

for key in hist_dict_ref.keys():
    hist[dataset_ref][key] = np.array(hist_dict_ref[key].bin_values)
    temp, binedges[dataset_ref][key] = np.histogram([],range = [hist_dict_ref[key].xmin, hist_dict_ref[key].xmax], bins = len(hist_dict_ref[key].bin_values))
    xlabel[dataset_ref][key]=key
    yerr[dataset_ref][key] = np.ones_like(hist_dict_ref[key].bin_values)

dataset_dict[dataset_ref]={}
dataset_dict[dataset_ref]['Dataset_label']=dataset_ref+" (ref)"
dataset_dict[dataset_ref]['Color']='b'

options = {}
options['1DErrorBoxes']='False'
options['Data_MC_Ratio']='True'
options['Data_MC_Diff']='False'
options['Reference']=dataset_ref

livetime_dict={}
livetime_dict[dataset]=1.
livetime_dict[dataset_ref]=1.
if args.inlivetime:
    livetime_dict[dataset]=fitLivetime(args.inlivetime)
if args.reflivetime:
    livetime_dict[dataset_ref]=fitLivetime(args.reflivetime)

# Get good order
vars_list_1d=[]
i3src = os.getenv("I3_SRC")
fileWithKeys=open(i3src+"/icetop_Level3_scripts/resources/examples/histos/listWithPlots_L3.txt")
for line in fileWithKeys:
    if len(line.split())>0:
        if args.with_cuts:
            vars_list_1d +=["IT73_"+line.split()[0]]
        else:
            vars_list_1d +=line.split()

from plotting_function import gen_plots_pdf
gen_plots_pdf(dataset_dict, options, vars_list_1d, hist, yerr, binedges, xlabel, livetime_dict,args.outfile)
