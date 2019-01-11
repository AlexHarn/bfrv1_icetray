if __name__ == '__main__':
    sys.exit(0)

class gridpoint:
    """
    Helper class for making grids around the direction of a reconstructed track
    (or any other hypothesis with likelihood-fitted kinematics).
    For each grid point we want to store:
    + the full specification of an event hypothesis, to be used by likelihood service
    + the grid coordinates, e.g. some cartesian projection of 2D direction angles around the track
    + the negative log-likelihood value
    The 'grid' will be a simple list of grid points. The grid that was hardcoded in the
    currently used version of paraboloid (I3ParaboloidFitter) uses a 2D grid, with
    grid points arranged on rings around the input direction. That is also the default
    in this python implementation.
    """
    def __init__(self,hypo,gp,val=0.):
        self.hypothesis = hypo
        self.gridproj   = gp
        self.value      = val
