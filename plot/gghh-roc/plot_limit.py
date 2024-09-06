#!/usr/bin/env python3

import re
import numpy as np
import matplotlib.pyplot as plt

POSTFIX = re.search(r'(?:^|/)plot_limit([^/]*)\.py$', __file__).group(1) or '_default'

weights_gghh = {
    'ggHH_kl_0_kt_1': lambda kl, kt: (0.2*kl**2 - 1.2*kl + 1.0) * 2.508E-04 * 43834,
    'ggHH_kl_1_kt_1': lambda kl, kt: (-0.25*kl**2 + 1.25*kl) * 1.172E-04 * 73366,
    'ggHH_kl_5_kt_1': lambda kl, kt: (0.05*kl**2 - 0.05*kl) * 3.154E-04 * 34757,
}

kt = 1.0
kl_array = np.linspace(-10, 20, 31)
weights = sum(w(kl_array, kt) for w in weights_gghh.values()) / 100  # fb
methods = ['none', 'raw', 'lite', 'full', 'hid']
limits = np.load('limit' + POSTFIX + '/limits.npy')
labels = [
    'HbbVSQCD only',
    '(1) high-level jet variables',
    '(2) probHbb and probQCD',
    '(3) 188 output scores',
    '(4) 128 hidden scores',
]

for imethod, method in enumerate(methods):
    plt.plot(kl_array, limits[imethod, :, 2], label=labels[imethod])
plt.xlabel(r'$\kappa_\lambda$')
plt.ylabel(r'$\mu$')
plt.yscale('log')
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig('plot_limit' + POSTFIX + '.pdf')

plt.clf()
for imethod, method in enumerate(methods):
    plt.plot(kl_array, limits[imethod, :, 2] * weights, label=labels[imethod])
plt.xlabel(r'$\kappa_\lambda$')
plt.ylabel(r'$\sigma$ [fb]')
plt.yscale('log')
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig('plot_limit_xs' + POSTFIX + '.pdf')
