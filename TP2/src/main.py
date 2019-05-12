from netCDF4 import Dataset
import numpy as np
from scipy.signal import convolve2d
from matplotlib import pyplot as plt

filename = "../includes/OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc"
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
convoluted_img = convolve2d(matrix, border_kernel, mode='valid')
import pdb; pdb.set_trace()

#plt.imshow(matrix, cmap='gray')
#plt.show()
#plt.figure()
plt.imshow(convoluted_img, cmap='gray', vmin=0, vmax=1)
plt.show()

data_in.close()
exit()
