#!/usr/bin/env python

'''
This script is a utility which converts an old pickle file, where
the histograms are stored as objects (requiring icetray), to a new
file where the state is stored as a dictionary.  Therefore, you won't
need icetray to view histograms and downstream code can be easily
modified to read histograms from a DB or a pickle file.
'''

from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("input", help="Pickle file to convert.")                                    
args = parser.parse_args()

import os
try:
   import cPickle as pickle
except:
   import pickle

from icecube.production_histograms.histograms.histogram import Histogram

with open(args.input, 'rb') as infile:
    histograms = pickle.load(infile, encoding='latin-1')
    for key, value in histograms.items():
        if isinstance(value, Histogram):
            histograms[key] = value.__getstate__()

with open(args.input,'wb') as outfile:
    pickle.dump(histograms, outfile)
