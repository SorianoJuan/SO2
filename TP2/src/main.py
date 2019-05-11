from netCDF4 import Dataset
import numpy as np

filename = "../includes/OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc"

fd = Dataset(filename, mode="r")

fd.close()
