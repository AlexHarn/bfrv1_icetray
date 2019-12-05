#Useful functions to submit jobs to condor---This is the general version----

def CondorMakeDirs():
    '''Creates condor output directories'''
    
    import os
    
    main_dir = '/scratch/grenzi/'
    
    #if not (os.path.exists(main_dir+"condor-execs") and os.path.exists(main_dir+"condor-logs") and\
    #        os.path.exists(main_dir+"condor-out") and os.path.exists(main_dir+"condor-error") and\
    #        os.path.exists(main_dir+"condor-jobs")):
    try:
        os.makedirs(main_dir+"condor-execs")
    except:
        pass
    try:    
        os.makedirs(main_dir+"condor-logs")
    except:
        pass
    try:    
        os.makedirs(main_dir+"condor-out")
    except:
        pass
    try:
        os.makedirs(main_dir+"condor-error")
    except:
        pass
    try:
        os.makedirs(main_dir+"condor-jobs")
    except:
        pass
        
def DAGMakeFamilyDirs(FAMNUM):
    '''Creates DAG output "family" directories
    Takes a number id as argument.
    Reuturns back a number id, in case the given id "family" already exists"
    '''
    
    import os, shutil
    
    main_dir = '/scratch/grenzi/'
    
    while os.path.exists(main_dir+"condor-jobs/dagfamily%d" % FAMNUM):
        FAMNUM+=1
    
    execspath = (main_dir+"condor-execs/dagfamily%d" % FAMNUM)
    logspath = (main_dir+"condor-logs/dagfamily%d" % FAMNUM)
    outpath = (main_dir+"condor-out/dagfamily%d" % FAMNUM)
    errpath = (main_dir+"condor-error/dagfamily%d" % FAMNUM)
    jobspath = (main_dir+"condor-jobs/dagfamily%d" % FAMNUM)
    
    os.makedirs(execspath)
    os.makedirs(logspath)
    os.makedirs(outpath)
    os.makedirs(errpath)
    os.makedirs(jobspath)
    
    return FAMNUM                    

def CondorCreate(COMMAND, FAMNUM=0, PID=101, submit=False, memory=1000, quicktest=False):
    '''Creates a Condor file to be submitted
    FAMNUM: "Family" number id for the directories in which to work
    Put FAMNUM=0 in order not to work on dagfamily directories
    COMMAND: command line for the job to execute
    PID: ID of the job
    option: submit = 'submit' to submit
    '''

    import os, subprocess
    
    main_dir = '/scratch/grenzi/'

    if FAMNUM == 0:
        dagfamily = ''
    else:
        dagfamily = ('dagfamily%d' % FAMNUM)
        
    expath = (main_dir+"condor-execs/"+dagfamily+"/condor-%d.sh" % PID)
    while os.path.exists(expath):
        PID += 1 
        expath = (main_dir+"condor-execs/"+dagfamily+"/condor-%d.sh" % PID)
    fexec = open(expath, 'w+')
    fexec.write("#!/bin/bash\n")
    fexec.write("date\n")
    fexec.write("hostname\n")
    fexec.write("cd `pwd`\n")
    fexec.write(COMMAND+'\n')
    fexec.write("date\n")
    fexec.close()

    args = ['chmod', 'a+x', expath]
    
    try:
        subprocess.call(args)
    except:
        print 'Path exists: %s'%os.path.exists(expath)
        print str(args).replace('[','').replace("'","").replace(', ',' ').replace(']','')    
            
    
    jobpath=(main_dir+"condor-jobs/"+dagfamily+'/sub%d.condor' % PID)
    fcondor = open(jobpath, 'w+')
    fcondor.write("Universe  = vanilla\n")
    fcondor.write("Executable = "+expath+'\n')
    fcondor.write("Log = %scondor-logs/%s/condor-%d.log\n" % (main_dir,dagfamily,PID))
    fcondor.write("Output = %scondor-out/%s/condor-%d.out\n" % (main_dir,dagfamily,PID))
    fcondor.write("Error = %scondor-error/%s/condor-%d.err\n" % (main_dir,dagfamily,PID))
    fcondor.write("Notification = NEVER\n")
    #if "clsim_launcher" or 'L1' or 'L2' in COMMAND:
    #    fcondor.write("request_gpus = 1\n")
    #    fcondor.write("+AccountingGroup = \"gpu.$ENV(USER)\"\n")
    if quicktest:
        fcondor.write("+AccountingGroup = \"quicktest.$ENV(USER)\"\n")    
    fcondor.write("request_memory = %d\n"%memory)
    fcondor.write("+NATIVE_OS = true\n")
    fcondor.write("Requirements = (OpSysMajorVer =?= 7)\n")
    fcondor.write("Hold = False\n")
    fcondor.write('should_transfer_files = YES\n')
    fcondor.write('transfer_input_files=""\n')
    fcondor.write('transfer_output_files=""\n')
    fcondor.write("Queue\n")
    fcondor.close()

    if submit:
        subprocess.call(['condor_submit ',jobpath])
    #else:
        #print "The job is not going to be submitted"    
    
    return jobpath #('sub%d.condor' % PID)

    
def DAGSubmit(famnum, PARENT, CHILD, LEVEL3='', LEVEL4=''):
    '''Creates a DAG structure with parent and children and submits it
    famnum: the "family" id number
    PARENT: LIST of parent jobs
    CHILD: LIST of children jobs
    optional LEVEL3 and LEVEL4: LIST of jobs at the corresponding level'''
    
    import os, subprocess
             
    main_dir = '/scratch/grenzi/'
    
    dagpath = (main_dir+"condor-jobs/dagfamily%d/submit.dag" % famnum)
    dagfile = open(dagpath, 'w+')
    
    i=1
    for job in range(len(PARENT)):
        dagfile.write(('JOB %d '+PARENT[job]+'\n') % i)
        i+=1
    ipar=i
    for job in range(len(CHILD)):
        dagfile.write(('JOB %d '+CHILD[job]+'\n') % i)
        i+=1
    imax=i
    if LEVEL3 != '':
        for job in range(len(LEVEL3)):
            dagfile.write(('JOB %d '+LEVEL3[job]+'\n') % i)
            i+=1
        imaxmax=i
    if LEVEL4 != '':
        for job in range(len(LEVEL4)):
            dagfile.write(('JOB %d '+LEVEL4[job]+'\n') % i)
            i+=1
        imaxmaxmax=i
    
    dagfile.write('PARENT ')                 
    for i in range(1, ipar):
        dagfile.write('%d ' % i)
    dagfile.write('CHILD ')                 
    for i in range(ipar, imax):
        dagfile.write('%d ' % i)          
    if LEVEL3 != '':
        dagfile.write('\nPARENT ')                 
        for i in range(ipar, imax):
            dagfile.write('%d ' % i)
        dagfile.write('CHILD ')                 
        for i in range(imax, imaxmax):
            dagfile.write('%d ' % i)
    if LEVEL4 != '':
        dagfile.write('\nPARENT ')                 
        for i in range(imax, imaxmax):
            dagfile.write('%d ' % i)
        dagfile.write('CHILD ')                 
        for i in range(imaxmax, imaxmaxmax):
            dagfile.write('%d ' % i)
        
    dagfile.close()
        
    os.system("condor_submit_dag -no_submit "+dagpath)
    
    subpath = (main_dir+"condor-jobs/dagfamily%d/submit.dag.condor.sub" % famnum)
    subfile = open(subpath, 'r')
    lines = subfile.readlines()
    subfile.close() 
    
    subfile = open(subpath, 'w')
    for line in lines:
        if "submit.dag.dagman.log" in line:
            lines.insert(lines.index(line)+1, "request_memory = 8000\n")
            #lines.insert(lines.index(line)+2, "+AccountingGroup = \"1_week.$ENV(USER)\"\n")
            #lines.insert(lines.index(line)+2, "+AccountingGroup = \"quicktest.$ENV(USER)\"\n")            
    subfile.writelines(lines)
    subfile.close()    
    
    args = ["condor_submit",subpath]
    subprocess.call(args)
    
    