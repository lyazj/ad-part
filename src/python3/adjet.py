import npgz
import numpy as np
import matplotlib.pyplot as plt

Feature = np.float32
NFEAT_JET = 27
NFEAT_PAR = 22
NFEAT_EVT = 7
NFEAT_LEP = 11
NFEAT_PHO = 13
NPAR_JET = 128
NRSLT_CLS = 188
NHIDDEN = 128
NJET_EVT = 10
NLEP_EVT = 10
NPHO_EVT = 10

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
JET_N2_0               = 10
JET_N3_0               = 11
JET_N3_1               = 12
JET_N3_2               = 13
JET_N3_3               = 14
JET_N3_4               = 15
JET_N3_5               = 16
JET_N3_6               = 17
JET_N3_7               = 18
JET_N3_8               = 19
JET_N3_9               = 20
JET_N3_10              = 21
JET_TAU21              = 22
JET_TAU32              = 23
JET_TAU43              = 24
JET_FTREC              = 25
JET_LABEL              = 26

EVT_HT                 =  0
EVT_MET                =  1
EVT_METPHI             =  2
EVT_NJET               =  3
EVT_NLEP               =  4
EVT_NPHO               =  5
EVT_LABEL              =  6

LEP_PT                 =  0
LEP_ETA                =  1
LEP_PHI                =  2
LEP_E                  =  3
LEP_CHARGE             =  4
LEP_ISO_DB             =  5
LEP_ISO_RC             =  6
LEP_D0                 =  7
LEP_D0_ERR             =  8
LEP_DZ                 =  9
LEP_DZ_ERR             = 10

PHO_PT                 =  0
PHO_ETA                =  1
PHO_PHI                =  2
PHO_E                  =  3
PHO_T                  =  4
PHO_EHOE               =  5
PHO_ISO_DB             =  6
PHO_ISO_RC             =  7
PHO_SUMPT_C            =  8
PHO_SUMPT_N            =  9
PHO_SUMPT_C_PU         = 10
PHO_SUMPT              = 11
PHO_STATUS             = 12

CLS_NAME = [ r'<unknown>', ] * NRSLT_CLS

PAR_FEAT_NAME = [
    r'$\log p_\mathrm{T}$',
    r'$\log E$',
    r'$\log p_\mathrm{T, rel}$',
    r'$\log E_\mathrm{rel}$',
    r'$\Delta R$',
    r'charge',
    r'is_charged_hadron',
    r'is_neutral_hadron',
    r'is_photon',
    r'is_electron',
    r'is_muon',
    r'$d_0$',
    r'$d_{0, err}$',
    r'$d_z$',
    r'$d_{z, err}$',
    r'$\Delta\eta$',
    r'$\Delta\phi$',
    r'$p_x$',
    r'$p_y$',
    r'$p_z$',
    r'$E$',
    r'mask',
]

JET_FEAT_NAME = [
    r'$p_{\mathrm{T}}$',
    r'$\eta$',
    r'$\phi$',
    r'$E$',
    r'$N_{\mathrm{particle}}$',
    r'$m_{\mathrm{SD}}$',
    r'$\tau_{1}$',
    r'$\tau_{2}$',
    r'$\tau_{3}$',
    r'$\tau_{4}$',
    r'$n_{2,0}$',
    r'$n_{3,0}$',
    r'$n_{3,1}$',
    r'$n_{3,2}$',
    r'$n_{3,3}$',
    r'$n_{3,4}$',
    r'$n_{3,5}$',
    r'$n_{3,6}$',
    r'$n_{3,7}$',
    r'$n_{3,8}$',
    r'$n_{3,9}$',
    r'$n_{3,10}$',
    r'$\tau_{21}$',
    r'$\tau_{32}$',
    r'$\tau_{43}$',
    r'$f_{t,\mathrm{rec}}$',
    r'label',
]

EVT_FEAT_NAME = [
    r'HT',
    r'MET',
    r'$\phi_{\mathrm{MET}}$',
    r'$N_{\mathrm{jet}}$',
    r'$N_{\mathrm{lepton}}$',
    r'$N_{\mathrm{photon}}$',
    r'label',
]

LEP_FEAT_NAME = [
    r'$p_\mathrm{T}$',
    r'$\eta$',
    r'$\phi$',
    r'$E$',
    r'charge',
    r'iso_db',
    r'iso_rc',
    r'$d_0$',
    r'$d_{0, err}$',
    r'$d_z$',
    r'$d_{z, err}$',
]

PHO_FEAT_NAME = [
    r'$p_\mathrm{T}$',
    r'$\eta$',
    r'$\phi$',
    r'$E$',
    r't',
    r'EHoE',
    r'iso_db',
    r'iso_rc',
    r'$p_\mathrm{T,sum,c}$',
    r'$p_\mathrm{T,sum,n}$',
    r'$p_\mathrm{T,sum,c,pu}$',
    r'$p_\mathrm{T,sum}$',
    r'status',
]

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

    def __init__(self, data, reduced=True):
        self.reduced = reduced
        if self.reduced: assert tuple(data.shape) == (NFEAT_JET,)
        else: assert tuple(data.shape) == (NFEAT_JET + NFEAT_PAR * JET_NPAR,)
        self.data = data

    @property
    def jet(self):
        return self.data[:NFEAT_JET]

    @property
    def par(self):
        return self.data[NFEAT_JET:].reshape(NPAR_JET, NFEAT_PAR)

    def particles(self):
        return [ADParticle(p) for p in self.par]

    @property
    def pt(self):     return self.data[ 0]
    @property
    def eta(self):    return self.data[ 1]
    @property
    def phi(self):    return self.data[ 2]
    @property
    def e(self):      return self.data[ 3]
    @property
    def npar(self):   return self.data[ 4]
    @property
    def sdmass(self): return self.data[ 5]
    @property
    def tau1(self):   return self.data[ 6]
    @property
    def tau2(self):   return self.data[ 7]
    @property
    def tau3(self):   return self.data[ 8]
    @property
    def tau4(self):   return self.data[ 9]
    @property
    def n2_0(self):   return self.data[10]
    @property
    def n3_0(self):   return self.data[11]
    @property
    def n3_1(self):   return self.data[12]
    @property
    def n3_2(self):   return self.data[13]
    @property
    def n3_3(self):   return self.data[14]
    @property
    def n3_4(self):   return self.data[15]
    @property
    def n3_5(self):   return self.data[16]
    @property
    def n3_6(self):   return self.data[17]
    @property
    def n3_7(self):   return self.data[18]
    @property
    def n3_8(self):   return self.data[19]
    @property
    def n3_9(self):   return self.data[20]
    @property
    def n3_10(self):  return self.data[21]
    @property
    def tau21(self):  return self.data[22]
    @property
    def tau32(self):  return self.data[23]
    @property
    def tau43(self):  return self.data[24]
    @property
    def ftrec(self):  return self.data[25]
    @property
    def label(self):  return self.data[26]

    def hist_par(self, index, *args, strip_padding=True, **kwargs):
        n = int(self.npar) if strip_padding else NPART_JET
        kwargs = kwargs.copy()
        if n == 0: kwargs['density'] = False
        plt.hist(self.par[:n,index], *args, **kwargs)
        plt.xlabel(PAR_FEAT_NAME[index])
        plt.ylabel('a.u.')

class ADEvent:

    def __init__(self, data):
        assert tuple(data.shape) == (NFEAT_EVT,)
        self.data = data

    @property
    def ht(self):      return self.data[0]
    @property
    def met(self):     return self.data[1]
    @property
    def metphi(self):  return self.data[2]
    @property
    def njet(self):    return self.data[3]
    @property
    def nlep(self):    return self.data[4]
    @property
    def npho(self):    return self.data[5]
    @property
    def label(self):   return self.data[6]

class ADLepton:

    def __init__(self, data):
        assert tuple(data.shape) == (NFEAT_LEP,)
        self.data = data

    @property
    def pt(self):                return self.data[ 0]
    @property
    def eta(self):               return self.data[ 1]
    @property
    def phi(self):               return self.data[ 2]
    @property
    def e(self):                 return self.data[ 3]
    @property
    def charge(self):            return self.data[ 4]
    @property
    def iso_db(self):            return self.data[ 5]
    @property
    def iso_rc(self):            return self.data[ 6]
    @property
    def d0(self):                return self.data[ 7]
    @property
    def d0_err(self):            return self.data[ 8]
    @property
    def dz(self):                return self.data[ 9]
    @property
    def dz_err(self):            return self.data[10]

class ADPhoton:

    def __init__(self, data):
        assert tuple(data.shape) == (NFEAT_PHO,)
        self.data = data

    @property
    def pt(self):                return self.data[ 0]
    @property
    def eta(self):               return self.data[ 1]
    @property
    def phi(self):               return self.data[ 2]
    @property
    def e(self):                 return self.data[ 3]
    @property
    def t(self):                 return self.data[ 4]
    @property
    def ehoe(self):              return self.data[ 5]
    @property
    def iso_db(self):            return self.data[ 6]
    @property
    def iso_rc(self):            return self.data[ 7]
    @property
    def sumpt_c(self):           return self.data[ 8]
    @property
    def sumpt_n(self):           return self.data[ 9]
    @property
    def sumpt_c_pu(self):        return self.data[10]
    @property
    def sumpt(self):             return self.data[11]
    @property
    def status(self):            return self.data[12]

class ADPFData:

    def __init__(self, data, reduced=True):
        self.reduced = reduced
        if self.reduced: self.data = data.reshape(-1, NFEAT_JET)
        else: self.data = data.reshape(-1, NFEAT_JET + NFEAT_PAR * JET_NPAR)

    def jets(self):
        return [ADJet(j, reduced=self.reduced) for j in self.data]

    def pars(self, strip_padding=True, index=None):
        pars = []
        for jet in self.jets():
            n = int(jet.npar) if strip_padding else NPART_JET
            if index is not None:
                pars.extend(jet.par[:n, index])
            else:
                pars.extend(jet.par[:n])
        pars = np.array(pars)
        return pars

    def hist(self, index, *args, **kwargs):
        kwargs = kwargs.copy()
        if self.data.shape[0] == 0: kwargs['density'] = False
        plt.hist(self.data[:,index], *args, **kwargs)
        plt.xlabel(JET_FEAT_NAME[index])
        plt.ylabel('a.u.')

    def hist_par(self, index, *args, strip_padding=True, **kwargs):
        pars = self.pars(strip_padding, index)
        kwargs = kwargs.copy()
        if len(pars) == 0: kwargs['density'] = False
        plt.hist(pars, *args, **kwargs)
        plt.xlabel(PAR_FEAT_NAME[index])
        plt.ylabel('a.u.')

class ADCFData:

    def __init__(self, data):
        self.data = data.reshape(-1, NRSLT_CLS)

    def top1(self):
        return np.argmax(self.data, axis=-1)

class ADHIDData:

    def __init__(self, data):
        self.data = data.reshape(-1, NHIDDEN)

class ADPFLabel:

    def __init__(self, pf):
        self.data = np.array([j.jet[JET_LABEL] for j in pf.jets()]).astype('int')

    def top1(self):
        return self.data

class ADEVTData:

    def __init__(self, data):
        self.data = data.reshape(-1, NFEAT_EVT)

    def hist(self, index, *args, **kwargs):
        kwargs = kwargs.copy()
        if self.data.shape[0] == 0: kwargs['density'] = False
        plt.hist(self.data[:,index], *args, **kwargs)
        plt.xlabel(EVT_FEAT_NAME[index])
        plt.ylabel('a.u.')

class ADLEPData:

    def __init__(self, data):
        self.data = data.reshape(-1, NFEAT_LEP)

    def hist(self, index, *args, **kwargs):
        kwargs = kwargs.copy()
        if self.data.shape[0] == 0: kwargs['density'] = False
        plt.hist(self.data[:,index], *args, **kwargs)
        plt.xlabel(LEP_FEAT_NAME[index])
        plt.ylabel('a.u.')

class ADPHOData:

    def __init__(self, data):
        self.data = data.reshape(-1, NFEAT_PHO)

    def hist(self, index, *args, **kwargs):
        kwargs = kwargs.copy()
        if self.data.shape[0] == 0: kwargs['density'] = False
        plt.hist(self.data[:,index], *args, **kwargs)
        plt.xlabel(PHO_FEAT_NAME[index])
        plt.ylabel('a.u.')

class ADCollection:

    def __init__(self, pf, cf):
        data = [[] for _ in range(NRSLT_CLS)]
        for jet, cls in zip(pf.data, cf.top1()):
            data[cls].append(jet)
        self.data = [ADPFData(np.array(d)) for d in data]

    def hist(self, index, *args, **kwargs):
        kwargs.pop('label', None)
        kwargs.pop('color', None)
        if 'range' not in kwargs:
            data = np.concatenate([d.data[:,index] for d in self.data])
            mean = np.mean(data)
            std = np.std(data)
            kwargs['range'] = (mean - 3 * std, mean + 3 * std)
        for i, item in enumerate(self.data):
            item.hist(index, *args, **kwargs, label=CLS_NAME[i])

    def hist_par(self, index, *args, strip_padding=True, **kwargs):
        kwargs.pop('label', None)
        kwargs.pop('color', None)
        if 'range' not in kwargs:
            data = np.concatenate([d.pars(strip_padding, index) for d in self.data])
            mean = np.mean(data)
            std = np.std(data)
            kwargs['range'] = (mean - 3 * std, mean + 3 * std)
        for i, item in enumerate(self.data):
            item.hist_par(index, *args, strip_padding=strip_padding, **kwargs, label=CLS_NAME[i])

def load_pf(dumpfile: str):
    return ADPFData(np.fromgz(dumpfile, dtype=Feature))

def load_cf(partfile: str):
    return ADCFData(np.fromgz(partfile, dtype=Feature))

def load_hid(hidfile: str):
    return ADHIDData(np.fromgz(hidfile, dtype=Feature))

def load_evt(evtfile: str):
    return ADEVTData(np.fromgz(evtfile, dtype=Feature))

def load_lep(lepfile: str):
    return ADLEPData(np.fromgz(lepfile, dtype=Feature))

def load_pho(phofile: str):
    return ADPHOData(np.fromgz(phofile, dtype=Feature))

def collect(pf: ADPFData, cf: ADCFData = None):
    if cf is None: cf = ADPFLabel(pf)
    return ADCollection(pf, cf)

class ADRawDataSet:

    def __init__(self, jetfile: str, evtfile: str, lepfile: str, phofile: str):
        jet = load_pf(jetfile).data
        evt = load_evt(evtfile).data
        lep = load_lep(lepfile).data
        pho = load_pho(phofile).data

        # Make input tensors.
        t_evt = evt[:,:EVT_LABEL].reshape(-1, 1, EVT_LABEL)
        t_jet = np.zeros((evt.shape[0], NJET_EVT, JET_LABEL))
        t_lep = np.zeros((evt.shape[0], NLEP_EVT, NFEAT_LEP))
        t_pho = np.zeros((evt.shape[0], NPHO_EVT, NFEAT_PHO))
        jet_i, lep_i, pho_i = 0, 0, 0
        for evt_i in range(evt.shape[0]):
            njet, nlep, npho = map(lambda x: int(evt[evt_i,x]), (EVT_NJET, EVT_NLEP, EVT_NPHO))
            njet_to_save, nlep_to_save, npho_to_save = map(min, (njet, nlep, npho), (NJET_EVT, NLEP_EVT, NPHO_EVT))
            t_jet[evt_i,:njet_to_save] = jet[jet_i:jet_i + njet_to_save,:JET_LABEL]
            t_lep[evt_i,:nlep_to_save] = lep[lep_i:lep_i + nlep_to_save]
            t_pho[evt_i,:npho_to_save] = pho[pho_i:pho_i + npho_to_save]
            jet_i, lep_i, pho_i = jet_i + njet, lep_i + nlep, pho_i + npho
        if jet_i != jet.shape[0]: raise RuntimeError(f'expect {jet_i} jets, got {jet.shape[0]}')
        if lep_i != lep.shape[0]: raise RuntimeError(f'expect {lep_i} leptons, got {lep.shape[0]}')
        if pho_i != pho.shape[0]: raise RuntimeError(f'expect {pho_i} photons, got {pho.shape[0]}')

        self.evt = t_evt
        self.jet = t_jet
        self.lep = t_lep
        self.pho = t_pho
        self.label = evt[:,EVT_LABEL].reshape(-1, 1)

class ADParTDataSet:

    def __init__(self, hidfile: str, evtfile: str, lepfile: str, phofile: str):
        jet = load_hid(hidfile).data
        evt = load_evt(evtfile).data
        lep = load_lep(lepfile).data
        pho = load_pho(phofile).data

        # Make input tensors.
        t_evt = evt[:,:EVT_LABEL].reshape(-1, 1, EVT_LABEL)
        t_jet = np.zeros((evt.shape[0], NJET_EVT, NHIDDEN))
        t_lep = np.zeros((evt.shape[0], NLEP_EVT, NFEAT_LEP))
        t_pho = np.zeros((evt.shape[0], NPHO_EVT, NFEAT_PHO))
        jet_i, lep_i, pho_i = 0, 0, 0
        for evt_i in range(evt.shape[0]):
            njet, nlep, npho = map(lambda x: int(evt[evt_i,x]), (EVT_NJET, EVT_NLEP, EVT_NPHO))
            njet_to_save, nlep_to_save, npho_to_save = map(min, (njet, nlep, npho), (NJET_EVT, NLEP_EVT, NPHO_EVT))
            print(evt_i, njet_to_save, jet_i, jet.shape)
            t_jet[evt_i,:njet_to_save] = jet[jet_i:jet_i + njet_to_save]
            t_lep[evt_i,:nlep_to_save] = lep[lep_i:lep_i + nlep_to_save]
            t_pho[evt_i,:npho_to_save] = pho[pho_i:pho_i + npho_to_save]
            jet_i, lep_i, pho_i = jet_i + njet, lep_i + nlep, pho_i + npho
        if jet_i != jet.shape[0]: raise RuntimeError(f'expect {jet_i} jets, got {jet.shape[0]}')
        if lep_i != lep.shape[0]: raise RuntimeError(f'expect {lep_i} leptons, got {lep.shape[0]}')
        if pho_i != pho.shape[0]: raise RuntimeError(f'expect {pho_i} photons, got {pho.shape[0]}')

        self.evt = t_evt
        self.jet = t_jet
        self.lep = t_lep
        self.pho = t_pho
        self.label = evt[:,EVT_LABEL].reshape(-1, 1)
