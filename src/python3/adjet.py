import npgz
import numpy as np

Feature = np.float32
NFEAT_JET = 10
NFEAT_PAR = 22
NPAR_JET = 128
NFEAT_TOT = NFEAT_JET + NFEAT_PAR * NPAR_JET

PAR_LOG_PT             =  0
PAR_LOG_E              =  1
PAR_LOG_PT_REL         =  2
PAR_LOG_E_REL          =  3
PAR_DELTA_R            =  4
PAR_CHARGE             =  5
PAR_IS_CHARGED_HADRON  =  6
PAR_IS_NEUTRAL_HADRON  =  7
PAR_IS_PHOTON          =  8
PAR_IS_ELECTRON        =  9
PAR_IS_MUON            = 10
PAR_D0                 = 11
PAR_D0_ERR             = 12
PAR_DZ                 = 13
PAR_DZ_ERR             = 14
PAR_DETA               = 15
PAR_DPHI               = 16
PAR_PX                 = 17
PAR_PY                 = 18
PAR_PZ                 = 19
PAR_E                  = 20
PAR_MASK               = 21

JET_PT                 =  0
JET_ETA                =  1
JET_PHI                =  2
JET_E                  =  3
JET_NPAR               =  4
JET_SDMASS             =  5
JET_TAU1               =  6
JET_TAU2               =  7
JET_TAU3               =  8
JET_TAU4               =  9

class ADParticle:

    def __init__(self, data):
        assert tuple(data.shape) == (NFEAT_PAR,)
        self.data = data

    @property
    def particle(self):
        return self.data

    @property
    def log_pt(self):            return self.data[ 0]
    @property
    def log_e(self):             return self.data[ 1]
    @property
    def log_pt_rel(self):        return self.data[ 2]
    @property
    def log_e_rel(self):         return self.data[ 3]
    @property
    def delta_r(self):           return self.data[ 4]
    @property
    def charge(self):            return self.data[ 5]
    @property
    def is_charged_hadron(self): return self.data[ 6]
    @property
    def is_neutral_hadron(self): return self.data[ 7]
    @property
    def is_photon(self):         return self.data[ 8]
    @property
    def is_electron(self):       return self.data[ 9]
    @property
    def is_muon(self):           return self.data[10]
    @property
    def d0(self):                return self.data[11]
    @property
    def d0_err(self):            return self.data[12]
    @property
    def dz(self):                return self.data[13]
    @property
    def dz_err(self):            return self.data[14]
    @property
    def deta(self):              return self.data[15]
    @property
    def dphi(self):              return self.data[16]
    @property
    def px(self):                return self.data[17]
    @property
    def py(self):                return self.data[18]
    @property
    def pz(self):                return self.data[19]
    @property
    def e(self):                 return self.data[20]
    @property
    def mask(self):              return self.data[21]

class ADJet:

    def __init__(self, data):
        assert tuple(data.shape) == (NFEAT_TOT,)
        self.data = data

    @property
    def jet(self):
        return self.data[:NFEAT_JET]

    @property
    def par(self):
        return self.data[NFEAT_JET:].reshape(NPAR_JET, NFEAT_PAR)

    @property
    def particles(self):
        return [ADParticle(p) for p in self.par]

    @property
    def pt(self):     return self.data[0]
    @property
    def eta(self):    return self.data[1]
    @property
    def phi(self):    return self.data[2]
    @property
    def e(self):      return self.data[3]
    @property
    def npar(self):   return self.data[4]
    @property
    def sdmass(self): return self.data[5]
    @property
    def tau1(self):   return self.data[6]
    @property
    def tau2(self):   return self.data[7]
    @property
    def tau3(self):   return self.data[8]
    @property
    def tau4(self):   return self.data[9]

class ADData:

    def __init__(self, data):
        self.data = data.reshape(-1, NFEAT_TOT)

    @property
    def jets(self):
        return [ADJet(j) for j in self.data]

def load(dumpfile: str) -> list:
    return ADData(np.fromgz(dumpfile, dtype=Feature))
