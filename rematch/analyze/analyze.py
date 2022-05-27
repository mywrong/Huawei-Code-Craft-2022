import numpy as np
import matplotlib.pyplot as mp
from mpl_toolkits.mplot3d import axes3d

C,W=np.arange(1000),np.arange(1000)
# V=[9,99,999,9999,99999,9999999]
V=[10]
mp.figure('3D Wireframe')
ax = mp.gca(projection='3d')  # 获取三维坐标
mp.title('3D Wireframe', fontsize=20)
# 三维标签
ax.set_xlabel('c', fontsize=14)
ax.set_ylabel('w', fontsize=14)
ax.set_zlabel('cost', fontsize=14)
mp.tick_params(labelsize=10)
# rstride/cstride 行列间距
for v in V:
    COST=[]
    for c in C:
        CO=[]
        for w in W:
            if w<=c:
                cost=(w-v)**2/c+w
            else:
                cost=0
            CO.append(cost)
        COST.append(CO)
    COST=np.array(COST)

    ax.plot_wireframe(W, C, COST, rstride=30,
                      cstride=30, linewidth=0.5,
                      color='orangered')
    # ax.set_zlim3d(zmin=0, zmax=10e6)
mp.show()
