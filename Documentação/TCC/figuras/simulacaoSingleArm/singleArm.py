import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "sans-serif",
    "font.sans-serif": ["Helvetica"]})

desloc = np.linspace(0.1,0.5,15*60)
y = np.arctan(desloc/(0.5))*3,18
speed = np.diff(y)
fig = plt.figure(figsize=(16, 9),)

print(speed)

plt.plot(desloc, y, color='blue', label='$$ r = 0,5m $$')
plt.legend(loc="upper left")
# fig.set(xlabel='$$ a $$', ylabel='$$ \phi $$')

# fig.suptitle("figname")
plt.savefig('./singleArmSimulation.pdf', bbox_inches='tight')
plt.close()
