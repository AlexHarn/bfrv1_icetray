import numpy as np

class Kalman:
    
    
    def __init__(self, stateDim, obsDim, initial_state=[], Q_Value=1e-2, R_Value=1, P_Value=None ):
        self.stateDim = stateDim
        self.obsDim = obsDim
        
        self.Q = np.eye(stateDim)*Q_Value         # Process noise
        self.R = np.eye(obsDim)*R_Value           # Observation noise
        self.A = np.eye(stateDim)                 # Transition matrix
        self.H = np.zeros((obsDim, stateDim))     # Measurement matrix
        self.K = np.zeros_like(self.H.T)          # Gain matrix
        
        if not P_Value is None:
            self.P = P_Value                                   # State covariance
        else:
            self.P = np.zeros_like(self.A)


        self.x = np.zeros((stateDim, 1))          # Current state of the system
        self.z = np.zeros((obsDim,1))             # Actual Measurement
        
        
        idx = np.ix_(np.r_[0:len(initial_state)],np.r_[0])
        
        self.x[idx] = np.array([initial_state]).T               # Initialization
        
        
        idx = np.r_[0:obsDim]
        positionIdx = np.ix_(idx, idx)
        self.H[positionIdx] = np.eye(obsDim)

        if obsDim == stateDim:        
            # The model is: x( t + 1 ) = x( t )
            pass

        if obsDim == stateDim / 2.:
            # The model is : x( t + 1 ) = x( t ) + v( t ) * dt
            
            self.velocityIdx = np.ix_(idx,idx+obsDim)
            self.A = np.eye(stateDim)
            self.A[self.velocityIdx] = np.eye(obsDim)
    
        elif obsDim == stateDim / 3.:
            # The model is : x( t + 1 ) = x( t ) + v( t ) + a( t ) * dt^2 / 2
            
            self.velocityIdx = np.ix_(idx, idx + obsDim)
            self.accelIdx = np.ix_(idx, idx + obsDim * 2)
            self.accelAndVelIdx = np.ix_(idx + obsDim, idx + obsDim * 2)
            
            self.A = np.eye(stateDim)
            self.A[self.velocityIdx] += np.eye(obsDim)
            self.A[self.accelIdx] += 0.5 * np.eye(obsDim)
            self.A[self.accelAndVelIdx] += np.eye(obsDim)
        
        else:
            raise NotImplementedError("Dimension mismatch between obsDim and stateDim")
    
    def Update(self, z, dt=1):
        
        # Input
        self.z = z[:, np.newaxis]
        
        if self.obsDim == self.stateDim / 2.:
            self.A[self.velocityIdx] = np.eye(self.obsDim) * dt
        
        if self.obsDim == self.stateDim / 3.:
            self.A[self.velocityIdx] = np.eye(self.obsDim) * dt
            self.A[self.accelIdx] = 0.5 * np.eye(self.obsDim) * np.power(dt,2)
            self.A[self.accelAndVelIdx] = np.eye(self.obsDim) * dt
        
        # Make prediction
        self.x = np.dot(self.A, self.x)
        self.P = np.dot(np.dot(self.A, self.P), self.A.T) + self.Q
        
        # Compute the optimal Kalman gain factor
        self.K = np.linalg.solve((np.dot(np.dot(self.H, self.P), self.H.T) + self.R).T, np.dot(self.H, self.P.T)).T
        
        # Correction based on observation
        self.x += np.dot(self.K, ( self.z - np.dot(self.H, self.x) ))
        self.P -= np.dot(np.dot(self.K, self.H), self.P)


    def Predict(self):
        return self.x.flatten()

    def GetVariance(self):
        return np.diag(self.P)
        
    def GetVarianceVector(self):
        return self.P
    
    
    def Debug(self):
        np.set_printoptions(linewidth=120)
        print("####################### DEBUG #######################")
        
        print("Q")
        print(self.Q)  # Process noise
        print("R")
        print(self.R)  # Observation noise
        print("A")
        print(self.A)  # Transition matrix
        print("H")
        print(self.H)  # Measurement matrix
        print("K")
        print(self.K)  # Gain matrix
        print("P")
        print(self.P)  # State covariance
        print("x")
        print(self.x)  # The actual state of the system
        print("z")
        print(self.z)  # The observation
        np.set_printoptions()
