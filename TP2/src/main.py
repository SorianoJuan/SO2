from netCDF4 import Dataset
import numpy as np
from scipy.signal import convolve2d
from matplotlib import pyplot as plt

NX=21696
NY=21696
NKERNEL=3
filename = "../includes/OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc"
convolve_filename = "../includes/convolved_out.bin"
data_in = Dataset(filename, mode="r")

matrix = data_in.variables['CMI'][:]

np.ma.set_fill_value(matrix, 0)
matrix = matrix.filled()
border_kernel = np.array ([
    [-1,-1,-1],
    [-1,8,-1],
    [-1,-1,-1],
    ])
matrix = matrix[::3,::3]

#convoluted_img = convolve2d(matrix, border_kernel, mode='valid')
convolved_in_c = np.fromfile(convolve_filename, count=(NX-NKERNEL+1)*(NY-NKERNEL+1), dtype=np.float32,sep="").reshape(NX-NKERNEL+1,NY-NKERNEL+1)
plt.imshow(convolved_in_c, cmap='gray', vmin=0, vmax=512)
import pdb; pdb.set_trace()
plt.show()

data_in.close()
exit()
