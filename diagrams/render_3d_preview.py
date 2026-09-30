import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np
import os

def draw_cube(ax, origin, size, color='blue', alpha=0.3, label=None):
    x, y, z = origin
    dx, dy, dz = size
    
    # 8 vertices
    vertices = np.array([
        [x, y, z],
        [x + dx, y, z],
        [x + dx, y + dy, z],
        [x, y + dy, z],
        [x, y, z + dz],
        [x + dx, y, z + dz],
        [x + dx, y + dy, z + dz],
        [x, y + dy, z + dz]
    ])
    
    # 6 faces
    faces = [
        [vertices[0], vertices[1], vertices[5], vertices[4]],
        [vertices[7], vertices[6], vertices[2], vertices[3]],
        [vertices[0], vertices[4], vertices[7], vertices[3]],
        [vertices[1], vertices[2], vertices[6], vertices[5]],
        [vertices[0], vertices[3], vertices[2], vertices[1]],
        [vertices[4], vertices[5], vertices[6], vertices[7]]
    ]
    
    poly = Poly3DCollection(faces, facecolors=color, linewidths=1, edgecolors='black', alpha=alpha)
    ax.add_collection3d(poly)

fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Enclosure outer shell
draw_cube(ax, (0, 0, 0), (54, 32, 18), color='#34495e', alpha=0.4, label='Main Enclosure Shell')

# Camera Lens Hole (Front)
draw_cube(ax, (-1, 13, 6), (3, 7, 7), color='#e74c3c', alpha=0.9, label='📷 OV2640 Camera Lens Slot')

# Microphone Hole (Front)
draw_cube(ax, (-1, 4, 3), (3, 3, 3), color='#f1c40f', alpha=0.9, label='🎤 INMP441 Mic Port')

# OLED Display Window (Top)
draw_cube(ax, (12, 8, 17), (26, 14, 3), color='#3498db', alpha=0.8, label='📺 SSD1306 OLED Screen Window')

# Glasses C-Clip Channel (Inner Back)
draw_cube(ax, (4, 32, 2), (46, 7, 14), color='#9b59b6', alpha=0.6, label='🕶️ Glasses Arm Snap C-Clip')

# Internal ESP32-CAM Board representation
draw_cube(ax, (3, 3, 3), (40, 26, 6), color='#2ecc71', alpha=0.7, label='🧠 ESP32-CAM PCB (Internal)')

# USB Charging Port (Rear)
draw_cube(ax, (53, 11, 3), (3, 10, 5), color='#e67e22', alpha=0.9, label='🔌 TP4056 USB-C Charger Port')

ax.set_xlabel('Length X (54 mm)', fontsize=10, labelpad=10)
ax.set_ylabel('Width Y (32 mm)', fontsize=10, labelpad=10)
ax.set_zlabel('Height Z (18 mm)', fontsize=10, labelpad=10)
ax.set_title('3D CAD Architecture Model — AI Smart Glasses Holding Clip', fontsize=14, fontweight='bold', pad=15)

ax.set_xlim(-5, 60)
ax.set_ylim(-5, 45)
ax.set_zlim(-2, 25)

# Camera orientation
ax.view_init(elev=25, azim=-45)

plt.tight_layout()
output_png = r"c:\Users\harin\Desktop\smart Glasses\diagrams\smart_glasses_clip_3d.png"
plt.savefig(output_png, dpi=300)
print(f"Rendered 3D model PNG to: {output_png}")
