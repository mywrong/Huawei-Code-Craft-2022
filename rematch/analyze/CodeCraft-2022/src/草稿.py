import matplotlib.pyplot as plt
import numpy as np

need=1000

W1=[i for i in range(1000)]
c1=20000
c2=10000

cost=[]
for w1 in W1:
    cost.append(w1**2/c1+w1+(need-w1)**2/c2+(need-w1))

plt.plot(W1,cost)
plt.xlabel("w1")
plt.ylabel("cost")
plt.show()