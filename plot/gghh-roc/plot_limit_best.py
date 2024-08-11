#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

kt = 1.0
kl_array = np.linspace(-10, 20, 31)
#methods = ['none', 'raw', 'lite', 'full', 'hid']
methods = ['raw', 'lite', 'full', 'hid']
limits = np.load('limit_best/limits.npy')
labels = [
    #'HbbVSQCD only',
    '(1) high-level jet variables',
    '(2) probHbb and probQCD',
    '(3) 188 output scores',
    '(4) 128 hidden scores',
]

for imethod, method in enumerate(methods):
    plt.plot(kl_array, limits[imethod, :, 2], label=labels[imethod])
plt.xlabel(r'$\kappa_\lambda$')
plt.ylabel(r'$\mu$')
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig('plot_limit_best.pdf')
