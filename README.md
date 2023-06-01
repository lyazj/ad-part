# ad-part: Anomaly detection aided by Particle Transformer

repo: https://github.com/lyazj/ad-part

architecture:

```mermaid
graph TD

library("ad-part library (libadhep)")

library --> definitions
library --> utilities
library --> Ntuple
library --> ParT

definitions --> addef

utilities --> adenv
utilities --> adfs
utilities --> admem

Ntuple --> adtype
Ntuple --> adbranch
Ntuple --> adnbranch
Ntuple --> adpdg
Ntuple --> adjet

ParT --> adtensor
ParT --> adrunner
```

```mermaid
graph TD

executable("ad-part executables")

executable --> test
executable --> core

test --> root-test
test --> delphes-test
test --> onnx-test
test --> test-more(...)

core --> pdg-gen
core --> flatten
core --> part
core --> part-gpu
core --> part-gen
core --> core-more(...)
```

`adtensor` architecture:

```mermaid
classDiagram
ADTensor
ADTensor <|-- ADPFTensor
ADTensor <|-- ADCFTensor
ADPFTensor <|-- ADPFPoints
ADPFTensor <|-- ADPFFeatures
ADPFTensor <|-- ADPFVectors
ADPFTensor <|-- ADPFMask
ADCFTensor <|-- ADSoftmax
class ADTensor {
    Feature *data
    int64_t size
    int64_t *shape
    int64_t shape_size
    ADTensor(const int64_t *shape_in, int64_t shape_size_in)
    tensor(const Ort::MemoryInfo &)
    tensor(const Ort::MemoryInfo &, int64_t shape0)
    get_name()*
}
class ADPFTensor {
    ADPFTensor(int64_t nfeat)
    get_name()*
    set_value(int64_t n, const ADJet &)*
}
class ADCFTensor {
    ADCFTensor()
    get_name()*
    get_value(int64_t n, ADParT &)*
}
class ADPFPoints {
    ADPFPoints()
    deta(int64_t n, int64_t p)
    dphi(int64_t n, int64_t p)
    get_name()*
    set_value(int64_t n, const ADJet &)*
}
class ADPFFeatures {
    ADPFFeatures()
    log_pt(int64_t n, int64_t p)
    log_e(int64_t n, int64_t p)
    log_pt_rel(int64_t n, int64_t p)
    log_e_rel(int64_t n, int64_t p)
    delta_r(int64_t n, int64_t p)
    charge(int64_t n, int64_t p)
    is_charged_hadron(int64_t n, int64_t p)
    is_neutral_hadron(int64_t n, int64_t p)
    is_photon(int64_t n, int64_t p)
    is_electron(int64_t n, int64_t p)
    is_muon(int64_t n, int64_t p)
    d0(int64_t n, int64_t p)
    d0_err(int64_t n, int64_t p)
    dz(int64_t n, int64_t p)
    dz_err(int64_t n, int64_t p)
    deta(int64_t n, int64_t p)
    dphi(int64_t n, int64_t p)
    get_name()*
    set_value(int64_t n, const ADJet &)*
}
class ADPFVectors {
    ADPFVectors()
    px(int64_t n, int64_t p)
    py(int64_t n, int64_t p)
    pz(int64_t n, int64_t p)
    e(int64_t n, int64_t p)
    get_name()*
    set_value(int64_t n, const ADJet &)*
}
class ADPFMask {
    ADPFMask()
    mask(int64_t n, int64_t p)
    get_name()*
    set_value(int64_t n, const ADJet &)*
}
class ADSoftmax {
    ADSoftmax()
    softmax(int64_t n, int64_t c)
    get_name()*
    get_value(int64_t n, ADParT &)*
}
```

design: split data preprocessing (flatten, normalize, etc.) and model running

* partially due to conveniences gained upon current third-party library interfaces
* enabling intermediate result checking, improving robustness and maintainability
* enabling a quick startup of the model, saving our time to tune or debug on it

`flatten`: from Delphes AT to flat Ntuple

main idea:

* In `ADJet` constructor, do RTTI for its constituents, routing to corresponding `ADParticle ` constructor
* Robustness: unrecognized objects are detected and warned

reference: https://github.com/delphes/delphes/blob/master/examples/Example3.C

```c++
ADJet(const ADPDGQuerier &, const Jet &);

ADParticle(const GenParticle &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
ADParticle(const Track &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
ADParticle(const Tower &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
ADParticle(const Muon &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
```

`part`/`part-gpu`: from flat Ntuple to ParT scores

main idea:

* ONNX model exporting: `ParT_full.pt` $\to$ `part.onnx`
* run the model on GPU powered by [ONNX Runtime](https://onnxruntime.ai/)
