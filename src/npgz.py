import gzip
import numpy as np

# wrapper method: .gz file -> np.array
def fromgz(path: str, *args, **kwargs) -> np.array:
    with gzip.open(path) as file:
        buf = file.read()
    return np.frombuffer(buf, *args, **kwargs)

# Add it to numpy library.
np.fromgz = fromgz
