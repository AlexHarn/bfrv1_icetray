import glob, numpy as np

import icecube
from icecube import icetray, dataclasses, simclasses
from icecube.icetray import I3Units

class PregeneratedSampler(icetray.I3Module):
    r""" A sampler used to include pre-defined noise behavior in simulation files.
    
    We believe a large fraction of IceCube noise is generated within the glass of the
    DOMs, giving noise hits that are correlated in time within a single module. While this
    is parametrizable for single-PMT DOMs, the extension to multi-PMT DOMs isn't trivial.
    In order to account for the correlations between PMTs, this module produces MCPEs
    for the multi-PMT DOMs using predefined input files of the form 

    > time_since_previous  pmt_number

    with the time_since_previous relative to the previous hit anywhere on the module.
    Because of the structure of the test file I'm using, the time here is given in seconds!

    By using this form, we can generate noise for simulation using an arbitrarily complex
    GEANT4 simulation or using actual detector data (assuming the data is solely noise
    and has negligible contributions from atmospheric muons and neutrinos). This has
    the added benefit that we don't have to perform computationally costly noise fits to
    the hitspool data and, if we take calibration noise data regularly, we can include
    effects from the detector cooling down over time.

    Because this is all pre-generated, we make the assumption that NO OTHER INFORMATION
    IS NEEDED. This means that we don't take into account any calibration constants at all
    at this stage. It should be fine, since there aren't many calibration constants used 
    in the normal noise simulation either.

    """

    input_path = "/cvmfs/icecube.opensciencegrid.org/users/mlarson/mdom_noise/mDOM_darkrate_-25C.*.npy"
    input_data = None
    start_time = -15*I3Units.microsecond
    end_time = 15*I3Units.microsecond
    module_type = dataclasses.I3ModuleGeo.mDOM
    modules_to_run = None
    physics_map_name = None
    output_map_name = None
    random = None

    n_warn_physics = 0
    max_warn_physics = 20

    ##################################
    # 
    ##################################
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter("InputPath", "The location of the noise files to be used. If this includes wildcards, then "\
                          "the noise file to be used will be chosen randomly from all matching files for "\
                          "each instance of this module.", self.input_path)

        self.AddParameter("StartWindow", "The first time relative to existing physics hits to add the noise. Negative values "\
                          "indicate a time before the first hit, while positive values indicate after the "\
                          "final physics hit. Should be given in I3Units.ns!", self.start_time)

        self.AddParameter("EndWindow", "The first time relative to existing physics hits to add the noise. Negative values "\
                          "indicate a time before the first hit, while positive values indicate after the "\
                          "final physics hit. Should be given in I3Units.ns!", self.end_time)
        
        self.AddParameter("ModuleType", "Which type of module should we be adding noise to? Typically, this will be mDOMs, "\
                          "but it could be DEggs or WOMs as well if files were generated for those.", self.module_type)
        
        self.AddParameter("PhysicsHitMap", "The name of the I3MCPESeriesMap in the frame containing physics hits for this "\
                          "module type. If empty, a new map will be produced with only noise PEs.", None)

        self.AddParameter("OutputHitMap", "The name of the I3MCPESeriesMap to write to the frame containing both any discovered "\
                          "physics hits as well as newly added noise hits. If empty or matching PhysicsHitMap, PhysicsHitMap will "\
                          "be overwritten in the frame.", None)

        self.AddParameter("RandomService", "An instance of an I3RandomService to use for selecting which PEs to add to the "\
                          "frame.", None)

        self.AddOutBox('OutBox')
        return

    ##################################
    # 
    ##################################
    def Configure(self):
        self.GetParameter("InputPath", self.input_path)
        self.GetParameter("StarWindow", self.start_time)
        self.GetParameter("EndWindow", self.end_time)
        self.GetParameter("ModuleType", self.module_type)
        self.GetParameter("PhysicsHitMap", self.physics_map_name)
        self.GetParameter("OutputHitMap", self.output_map_name)
        self.GetParameter("RandomService", self.random)

        if self.random == None:
            icetray.logging.log_warn("No RandomService given. Falling back to numpy.random", "PregeneratedSampler")
            self.random = np.random
            self.random.integer = np.random.randint

        if self.output_map_name == None and self.physics_map_name == None:
            icetray.logging.log_fatal("Cannot be missing both PhysicsHitMap and OutputHitMap. Define at least one!",
                                      "PregeneratedSampler")
        if self.physics_map_name == None:
            icetray.logging.log_warn("No PhysicsHitMap given. Producing a noise-only PE map.", "PregeneratedSampler")
        
        if self.output_map_name == None:
            icetray.logging.log_warn("No OutputHitMap given. Overwriting the physics hit map", "PregeneratedSampler")
            self.output_map_name = self.physics_map_name
            
        # Get the possible pregenerated noise files
        files = glob.glob(self.input_path)
        if len(files) == 0:
            icetray.logging.log_fatal("No input files matching pattern {} found. Check your path.".format(self.input_path),
                                      "PregeneratedSampler")
        
        # Pick one of the files
        files.sort()
        i = self.random.integer(len(files))

        icetray.logging.log_info("Loading {}".format(files[i]))
        self.input_path = files[i]
        self.input_data = np.load(files[i])
        icetray.logging.log_info("File contains {} seconds of total livetime".format(np.sum(self.input_data[:,0])))

        # Convert the times to I3Units
        self.input_data[:,0] *= icetray.I3Units.second

        return


    ##################################
    # 
    ##################################
    def Geometry(self):
        self.modules_to_run = []

        # This exists in the upgrade GCD files
        # but may not in other files. If not, create it
        if not frame.Has("I3ModuleGeoMap"):
            icetray.logging.log_fatal("No I3ModuleGeoMap in the geometry frame! Either create it yourself or use the "
                                      "I3GeometryDecomposer to produce it for you!", "PregeneratedSampler")

        
        # Get a pre-generated list of modules we need to add to. This prevents us from
        # needing to loop over the full GCD file every time
        modulegeo = frame['I3ModuleGeoMap']
        for modkey, modgeo in modulegeo:
            if modgeo.module_type != self.module_type: continue
            self.modules_to_run.append(modkey)

        self.PushFrame(frame)
        return


    ##################################
    # 
    ##################################
    def DAQ(self, frame):

        # Check to make sure we've read in the stuff from the geometry
        if self.modules_to_run == None:
            log_fatal("Entered a DAQ frame, but I haven't seen a Geometry frame. Refusing to produce anything!",
                      "PregeneratedSampler")

        # Get any existing physics pulses
        if self.physics_map_name == None: 
            physics_map = simclasses.I3MCPESeriesMap()

        elif not frame.Has(self.physics_map_name):
            if self.n_warn_physics < self.max_warn_physics:
                icetray.logging.log_warn("No physics map with name {} in frame. Going to continue by producing a "\
                                         "noise-only map for this frame. ".format(self.physics_map_name), "PregeneratedSampler")
                self.n_warn_physics += 1
            if self.n_warn_physics == self.max_warn_physics:
                icetray.logging.log_warn("Issued warning {} times. Shutting up.".format(self.max_warn_physics), 
                                         "PregeneratedSampler")
            physics_map = simclasses.I3MCPESeriesMap()
        
        else:
            physics_map = frame[self.physics_map_name]


        # Find the first and last hit in the physics PEs
        times = np.array([ [p.time for p in pulses] for pulses in physics_map.values]).flatten()
        t0 = times.min() + self.start_time
        tf = times.max() + self.end_time

        # We now have the hits. Time to start producing MCPEs
        for module in self.modules_to_run:
            omk = icetray.OMKey(module.string, module.om)
            
            # Pick a starting index
            i = self.random.integer(self.input_data.shape[0]-pad)

            # Begin pushing pulses
            current_time = t0
            while current_time < tf:
                current_time += self.input_data[i,0]
                omk.pmt = int(self.input_data[i,1])
                mcpe = simclasses.I3MCPE(1, current_time)
                
                if not omk in physics_map.keys():
                    physics_map[omk] = simclasses.I3MCPESeries()
                physics_map[omk].append(mcpe)
                
                # Increment my counter, wrapping if necessary
                i += 1
                if i > self.input_data.shape[0]: i = 0
                
            # We're done. I probably should sort these.
            physics_map[omk] = simclasses.I3MCPESeries( sorted(physics_map, 
                                                               key = lambda a: a.time) )
            
        # Now all that's left is to write it out
        if frame.Has(self.output_map_name):
            del frame[self.output_map_name]
        frame[self.output_map_name] = physics_map
        
        self.PushFrame(frame)
        return
