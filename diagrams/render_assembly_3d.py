import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np

fig = plt.figure(figsize=(14, 9))
ax = fig.add_subplot(111, projection='3d')

def draw_box(ax, origin, size, color='black', alpha=0.3):
    x, y, z = origin
    dx, dy, dz = size
    vertices = np.array([
        [x, y, z], [x + dx, y, z], [x + dx, y + dy, z], [x, y + dy, z],
        [x, y, z + dz], [x + dx, y, z + dz], [x + dx, y + dy, z + dz], [x, y + dy, z + dz]
    ])
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

# --- 1. EYEGLASSES FRAME ---
# Left Rim
draw_box(ax, (-60, -5, 0), (45, 4, 35), color='#2c3e50', alpha=0.6)
# Right Rim
draw_box(ax, (15, -5, 0), (45, 4, 35), color='#2c3e50', alpha=0.6)
# Nose Bridge
draw_box(ax, (-15, -3, 20), (30, 2, 5), color='#2c3e50', alpha=0.8)

# Left Temple Arm
draw_box(ax, (-60, 0, 15), (4, 100, 8), color='#34495e', alpha=0.7)
# Right Temple Arm
draw_box(ax, (56, 0, 15), (4, 100, 8), color='#34495e', alpha=0.7)

# --- 2. 3D ENCLOSURE CLIP (Mounted on Right Temple Arm) ---
# Main Clip Enclosure Box
draw_box(ax, (60, 20, 5), (32, 54, 18), color='#9b59b6', alpha=0.85)

# Camera Lens Aperture (Pointing Forward along -Y)
draw_box(ax, (72, 18, 10), (8, 3, 8), color='#e74c3c', alpha=0.95)

# OLED Screen Viewport (Top Face)
draw_box(ax, (65, 30, 23), (22, 28, 3), color='#3498db', alpha=0.9)

# Microphone Port (Front Corner)
draw_box(ax, (62, 18, 7), (4, 3, 4), color='#f1c40f', alpha=0.95)

# USB Charging Slot (Rear Face)
draw_box(ax, (70, 74, 8), (12, 3, 6), color='#e67e22', alpha=0.95)

# Axis Styling & Labels
ax.set_xlabel('Width X (mm)', labelpad=10)
ax.set_ylabel('Depth Y (mm)', labelpad=10)
ax.set_zlabel('Height Z (mm)', labelpad=10)
ax.set_title('3D CAD Assembly Schematic — Smart Glasses Frame + Side Clip Housing', fontsize=14, fontweight='bold')

ax.set_xlim(-70, 100)
ax.set_ylim(-20, 110)
ax.set_zlim(-10, 40)

ax.view_init(elev=20, azim=-60)

plt.tight_layout()
output_png = r"c:\Users\harin\Desktop\smart Glasses\diagrams\smart_glasses_assembly_3d.png"
plt.savefig(output_png, dpi=300)
print(f"Assembly 3D schematic saved to: {output_png}")
