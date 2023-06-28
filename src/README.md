# An Efficient, General, and Expandable Gen-matching Algorithm

The two-step procedure:

* p-matching: Match gen-particles to predefined patterns (decay graphs) $\to$ gen-groups
* j-matching: Match jets to gen-groups

## p-matching

### Step 1: gen-graph pruning

Two types:

* remove intermediate states: e.g. $q\bar q \to g_1g_2, g_1 \to g^\prime_1 \to g^{\prime\prime}_1\ \Longrightarrow\ q\bar q \to g^{\prime\prime}_1g_2$
* remove uninterested nodes: e.g. mesons produced by hadronization

Implementation: depth-first search (DFS)

### Step 2: recursive-descendant matching

Two steps:

* serialization: decay graph $\to$ topological sequence (or decay tree $\to$ pre-order sequence)
* recursion: $\mathrm{SEQ} ::= [~~]\ |\ [\,\mathrm{VTX},\, \mathrm{SEQ}\,]$, where $\mathrm{VTX}$ denotes an accepting domain of gen-particles

Output: gen-particle sequences, all of the same length to $\mathrm{SEQ}$, permutation sensitive but not harmful

## j-matching

Strategy: adopting the first match among groups

Criteria: $\Delta R \leq 0.8$ for each vertex, and the entry with the lowest $\Delta R$ in the group is adopted

Motivation: inclusion relationships, e.g. $\mathrm{G\underline{}T\underline{}BCQ} \supset \mathrm{G\underline{}W\underline{}CQ}$

```c++
const ADPIDSelector P_T   { 6, -6 };
const ADPIDSelector P_W   { 24, -24 };
const ADPIDSelector P_C   { 4, -4 };
const ADPIDSelector P_Q   { 1, 2, 3, 4, -1, -2, -3, -4 };
const ADPIDSelector P_B   { 5, -5 };
const ADPIDSelector P_E   { 11, -11 };
const ADPIDSelector P_NE  { 12 };
const ADPIDSelector P_M   { 13, -13 };
const ADPIDSelector P_NM  { 14 };
const ADPIDSelector P_H   { 25 };
const ADPIDSelector P_G   { 21 };
const ADPIDSelector P_Z   { 23 };
const ADPIDSelector P_L   { 11, -11, 13, -13 };
const ADPIDSelector P_V   { 23, 24, -24 };
const ADPIDSelector P_QCD { 1, 2, 3, 4, 5, 6, -1, -2, -3, -4, -5, -6, 21 };

const ADDecayGraph G_T_BCQ { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_C), new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_T_BQQ { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_T_BC { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_C) } },
} };

const ADDecayGraph G_T_BQ { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_T_BEN { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_E), new ADDecayGraph(P_NE) } },
} };

const ADDecayGraph G_T_BMN { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_M), new ADDecayGraph(P_NM) } },
} };

const ADDecayGraph G_W_CQ { P_W, { new ADDecayGraph(P_C), new ADDecayGraph(P_Q) } };
const ADDecayGraph G_W_QQ { P_W, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } };

const ADDecayGraph G_Z_BB { P_Z, { new ADDecayGraph(P_B), new ADDecayGraph(P_B) } };
const ADDecayGraph G_Z_CC { P_Z, { new ADDecayGraph(P_C), new ADDecayGraph(P_C) } };
const ADDecayGraph G_Z_QQ { P_Z, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } };

const ADDecayGraph G_H_BB { P_H, { new ADDecayGraph(P_B), new ADDecayGraph(P_B) } };
const ADDecayGraph G_H_CC { P_H, { new ADDecayGraph(P_C), new ADDecayGraph(P_C) } };
const ADDecayGraph G_H_QQ { P_H, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } };
const ADDecayGraph G_H_GG { P_H, { new ADDecayGraph(P_G), new ADDecayGraph(P_G) } };

const ADDecayGraph G_H_QQQQ { P_H, {
  new ADDecayGraph { P_V, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
  new ADDecayGraph { P_V, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_H_QQL { P_H, {
  new ADDecayGraph { P_V, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
  new ADDecayGraph { P_V, { new ADDecayGraph(P_L) } },
} };

const ADDecayGraph G_QCD { P_QCD, { new ADDecayGraph(P_QCD) } };  // XXX
```

## Test

* delphes_stop2b1000_neutralino300: 

  QCD=8564 W_QQ=4186 T_BQQ=28394 T_BL=2020  ACC: 0.74607

* delphes_njets:

  QCD=890656  ACC: 0.67394
