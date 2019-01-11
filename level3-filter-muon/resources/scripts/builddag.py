#!/bin/env python
import os
import sys
import re
from glob import glob

data_sim = "/data/sim/IceCube"
data_exp = "/data/exp/IceCube"
#data_ana = "/data/ana/Muon/level3"
data_ana = "/data/ana/IceCube/Muon/MultiYearDiffuseAnalysis/IC86-2012/datasets"

gcds = {}
gcds[2012] = "/data/sim/sim-new/downloads/GCD/GeoCalibDetectorStatus_2012.56063_V1.i3.gz"
gcds[2013] = "/data/sim/sim-new/downloads/GCD/GeoCalibDetectorStatus_2013.56429_V1.i3.gz"

def build_nugen_dag(dag_file, season, dataset, type, num, reprocess=False, is_test=False):
    indir = "{data_sim}/{season}/filtered/level2/neutrino-generator/{dataset}/{num:02d}000-{num:02d}999".format(data_sim=data_sim, **locals())
    gcd = gcds[season]
    outdir = "{data_ana}/sim/{season}/neutrino-generator/{dataset}/{num:02d}000-{num:02d}999".format(data_ana=data_ana, **locals())
    os.system("mkdir -p {outdir}".format(**locals()))
    logdir = "{outdir}/logs".format(**locals())
    os.system("mkdir -p {logdir}".format(**locals()))
    files = sorted(glob("{indir}/*.i3.bz2".format(**locals())))
    infiles = []
    for f in files:
        matches = re.findall(r"Level2_[a-zA-Z0-9_]*\.[0-9]*\.[0-9]*\.[0-9]*.i3.bz2", f)
        if matches:
            infiles.append(f)
    infiles = files
    outfiles = ["{outdir}/{file}".format(file=os.path.basename(f).replace("Level2", "Level3"), **locals()) for f in infiles]
    if reprocess:
        files = [(infiles[i], outfile) for i, outfile in enumerate(outfiles)]
    else:
        files = [(infiles[i], outfile) for i, outfile in enumerate(outfiles) if not os.path.isfile(outfile)]
    if is_test and len(files)>100:
        files = files[:100]
    for infile, outfile in files:
        base = os.path.basename(infile)
        jobid = "{base}".format(**locals())
        if outfile[-4:]==".bz2":
            hd5outfile = outfile.replace(".i3.bz2", ".hd5")
            rootoutfile = outfile.replace(".i3.bz2", ".root")
        else:
            hd5outfile = outfile.replace(".i3", ".hd5")
            rootoutfile = outfile.replace(".i3", ".root")
        dag_file.write("JOB {jobid} OneJob.submit\n".format(**locals()))
        dag_file.write('VARS {jobid} INFILE="{infile}" OUTFILE="{outfile}" HD5OUTFILE="{hd5outfile}" ROOTOUTFILE="{rootoutfile}"  GCD="{gcd}" LOGDIR="{logdir}"\n'.format(**locals()))

def build_corsika_dag(dag_file, season, dataset, num, reprocess=False, is_test=False):
    indir = "{data_sim}/{season}/filtered/level2/CORSIKA-in-ice/{dataset}/{num:02d}000-{num:02d}999".format(data_sim=data_sim, **locals())
    gcd = gcds[season]
    outdir = "{data_ana}/sim/{season}/CORSIKA-in-ice/{dataset}/{num:02d}000-{num:02d}999".format(data_ana=data_ana, **locals())
    os.system("mkdir -p {outdir}".format(**locals()))
    logdir = "{outdir}/logs".format(**locals())
    os.system("mkdir -p {logdir}".format(**locals()))
    print indir
    files = sorted(glob("{indir}/*.i3.bz2".format(**locals())))
    infiles = []
    for f in files:
        matches = re.findall(r"Level2_[a-zA-Z0-9_]*\.[0-9]*\.[0-9]*\.[0-9]*.i3.bz2", f)
        if matches:
            infiles.append(f)
    infiles = files
    outfiles = ["{outdir}/{file}".format(file=os.path.basename(f).replace("Level2", "Level3"), **locals()) for f in infiles]
    if reprocess:
        files = [(infiles[i], outfile) for i, outfile in enumerate(outfiles)]
    else:
        files = [(infiles[i], outfile) for i, outfile in enumerate(outfiles) if not os.path.isfile(outfile)]
    if is_test and len(files)>100:
        files = files[:100]
    for infile, outfile in files:
        base = os.path.basename(infile)
        jobid = "{base}".format(**locals())
        if outfile[-4:]==".bz2":
            hd5outfile = outfile.replace(".i3.bz2", ".hd5")
            rootoutfile = outfile.replace(".i3.bz2", ".root")
        else:
            hd5outfile = outfile.replace(".i3", ".hd5")
            rootoutfile = outfile.replace(".i3", ".root")
        dag_file.write("JOB {jobid} OneJob.submit\n".format(**locals()))
        dag_file.write('VARS {jobid} INFILE="{infile}" OUTFILE="{outfile}" HD5OUTFILE="{hd5outfile}" ROOTOUTFILE="{rootoutfile}"  GCD="{gcd}" LOGDIR="{logdir}"\n'.format(**locals()))

def build_data_dag(dag_file, season, year, date, run_id, reprocess=False):
    # merge 10 files
    if season==2012:
        indir = "{data_exp}/{year}/filtered/level2/{date}".format(data_exp=data_exp, **locals())
        gcd = "{data_exp}/{year}/filtered/level2/{date}/Level2_IC86.{season}_data_Run00{run_id}_{date}_GCD.i3.gz".format(data_exp=data_exp, **locals())
    if season==2013:
        indir = "{data_exp}/{year}/filtered/level2/{date}/Run00{run_id}".format(data_exp=data_exp, **locals())
        gcd = glob("{data_exp}/{year}/filtered/level2/{date}/Run00{run_id}/Level2_IC86.{season}_data_Run00{run_id}_{date}_*_GCD.i3.gz".format(data_exp=data_exp, **locals()))[0]
    outdir = "{data_ana}/exp/{year}/{date}".format(data_ana=data_ana, **locals())
    os.system("mkdir -p {outdir}".format(**locals()))
    logdir = "{outdir}/logs".format(**locals())
    os.system("mkdir -p {logdir}".format(**locals()))
    files = "Level2_IC86.{season}_data_Run00{run_id}_Subrun00000".format(**locals())
    # find max file number
    j = 0
    maxnum = 0
    for j in range(0, 41):
        for k in range(0, 10):
            if os.path.isfile("{indir}/{files}{j:02d}{k}.i3.bz2".format(**locals())):
                maxnum = j
    # loop over file names, starting from 00* up to {maxnum}*
    for num in range(0, maxnum):
        globstr = "{indir}/Level2_IC86.{season}_data_Run00{run_id}_Subrun00000{num:02d}?.i3.bz2".format(**locals())
        base = os.path.basename(globstr.replace("?", ""))
        infiles = sorted(glob(globstr))
        infile_csv = ""
        for infile in infiles:
            infile_csv += infile + ","
        infile_csv = infile_csv[:-1]
        outfile = "{0}/{1}".format(outdir, base.replace("Level2","Level3"))
        if outfile[-4:]==".bz2":
            hd5outfile = outfile.replace(".i3.bz2", ".hd5")
            rootoutfile = outfile.replace(".i3.bz2", ".root")
        else:
            hd5outfile = outfile.replace(".i3", ".hd5")
            rootoutfile = outfile.replace(".i3", ".root")
        if not os.path.isfile(outfile) or reprocess:
            jobid = "{base}".format(**locals())
            dag_file.write("JOB {jobid} OneJob.submit\n".format(**locals()))
            dag_file.write('VARS {jobid} INFILE="{infile_csv}" OUTFILE="{outfile}" HD5OUTFILE="{hd5outfile}" ROOTOUTFILE="{rootoutfile}"  GCD="{gcd}" LOGDIR="{logdir}"\n'.format(**locals()))
