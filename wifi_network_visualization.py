#!/usr/bin/env python3
# Copyright (c) 2025 Texas State University
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Ahmed Maksud <ahmed.maksud@email.ucr.edu>
# PI: Marcelo Menezes De Carvalho <mmcarvalho@txstate.edu>
# Texas State University

"""
WiFi Network Animation and Visualization Script

This script creates dynamic animations of WiFi network simulation data:
- Real-time network topology visualization
- Station movement patterns and trajectories
- Performance metrics overlay (throughput, distance)
- Adaptive parameter visualization (transmission power)
- Interactive plots for network analysis

Key Features:
- Animated scatter plots showing station movement
- Color-coded performance indicators
- Distance-based layout visualization
- Transmission power adaptation tracking
- Export capabilities for presentation/analysis

Data Sources:
- toy_data.csv: Simulation results from toy1.py
- Real-time data during simulation (optional)
- Network topology and performance metrics

Visualization Components:
- Station positions over time
- Access Point location (fixed)
- Throughput heat mapping
- Distance circles for range analysis
- Adaptive control parameter tracking
"""

# Data visualization and animation libraries
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pandas as pd
import numpy as np
from tqdm import tqdm

# Standard libraries for file operations and system utilities
import os
import sys
from datetime import datetime

print("WiFi Network Animation - Starting visualization setup...")

# === FILE SYSTEM AND DATA LOADING ===
"""
Setup data input paths and load simulation results:
- Locate CSV data file from WiFi simulation
- Load and validate simulation data
- Prepare data structures for animation
"""
script_dir = os.path.dirname(os.path.abspath(__file__))
csv_path = os.path.join(script_dir, "toy_data.csv")
df = pd.read_csv(csv_path)
mlim = max(df["pos_x"].abs().max(), df["pos_y"].abs().max()) + 1

# Create figure and axis
fig, ax = plt.subplots(figsize=(10, 8))
ax.set_xlim(-mlim, mlim)
ax.set_ylim(-mlim, mlim)
ax.grid(True)
ax.set_title("WiFi STA Animation")

# AP position (center)
ap = ax.scatter([0], [0], c="red", s=100, label="AP")

# STA containers (hollow markers with STA numbers)
scat = ax.scatter(
    [], [], facecolors="none", edgecolors="blue", s=250, label="STA", linewidths=2
)
time_text = ax.text(0.05, 0.9, "", transform=ax.transAxes)
annotations = []
sta_texts = []


def init():
    scat.set_offsets(np.empty((0, 2)))
    time_text.set_text("")
    for txt in sta_texts:
        txt.remove()
    sta_texts.clear()
    return scat, time_text


def update(frame):
    current_time = df["now_sec"].unique()[frame]
    current_data = df[df["now_sec"] == current_time]

    # Update STA positions
    scat.set_offsets(current_data[["pos_x", "pos_y"]].values)

    # Clear previous elements
    for ann in annotations:
        ann.remove()
    annotations.clear()

    for txt in sta_texts:
        txt.remove()
    sta_texts.clear()

    # Add STA numbers inside markers
    for _, row in current_data.iterrows():
        txt = ax.text(
            row["pos_x"],
            row["pos_y"],
            str(int(row["sta_id"])),
            color="blue",
            ha="center",
            va="center",
            fontsize=10,
            fontweight="bold",
        )
        sta_texts.append(txt)

    # Create adjacent annotation boxes
    for _, row in current_data.iterrows():
        text = f"DL: {row['dl_tp']:.2f} Mbps\nDist: {row['distance']:.2f}m"
        ann = ax.text(
            row["pos_x"] + 0.01,  # Right offset
            row["pos_y"] + 0.01,  # Upper offset
            text,
            bbox=dict(facecolor="white", alpha=0.8, boxstyle="round,pad=0.2"),
            fontsize=9,
            ha="left",
            va="bottom",
        )
        annotations.append(ann)

    total_ul = current_data["ul_tp"].mean()
    total_dl = current_data["dl_tp"].sum()
    ApTx = current_data["set_ApTx"].mean()
    time_text.set_text(
        f"Ap Tx power: {ApTx:.2f} dB\n"
        f"Total DL: {total_dl:.2f} Mbps\n"
        f"Total UL: {total_ul:.2f} Mbps\n"
        f"Time: {current_time:.2f}s"
    )

    return scat, time_text, *annotations, *sta_texts


# Animation setup
frames = len(df["now_sec"].unique())
pbar = tqdm(total=frames, desc="Rendering frames")


def progress_callback(current_frame, total_frames):
    pbar.n = current_frame
    pbar.refresh()


ani = animation.FuncAnimation(
    fig,
    update,
    frames=frames,
    init_func=init,
    interval=200,
    blit=False,
)

plt.legend()
ani.save(
    "sta_animation.gif",
    writer="imagemagick",
    fps=5,
    progress_callback=progress_callback,
)
pbar.close()
print("Animation saved to sta_animation.gif")
