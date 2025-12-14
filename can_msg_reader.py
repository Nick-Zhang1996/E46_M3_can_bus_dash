import pickle
from collections import Counter
import matplotlib.pyplot as plt

from can_msg_logger import CANFrame
from can_msg_parsers import *

def main():
    filename = "to_publix.pkl"
    
    # --- CONFIGURATION: WHAT TO PLOT ---
    # Change these values to plot different data.
    # TARGET_ID: The CAN ID (hex) you want to track.
    # TARGET_KEY: The dictionary key returned by the parser you want to plot.
    
    # Example 1: Plot Engine RPM
    # TARGET_ID = 0x316
    # TARGET_KEY = 'RPM'

    # Example 2: Plot Steering Angle (requires the 0x1F5 parser added previously)
    # TARGET_ID = 0x1F5 
    # TARGET_KEY = 'Angle_Deg'

    # Example 3: Plot Wheel Speed (Front Left)
    # TARGET_ID = 0x1F0
    # TARGET_KEY = 'LR_Speed_kph'
    # -----------------------------------
    TARGET_ID = 0x1F3
    TARGET_KEY = "byte5"
    # 1. Load the data
    try:
        with open(filename, "rb") as f:
            frames = pickle.load(f)
    except Exception as e:
        print(f"Error loading pickle file: {e}")
        return

    print(f"Total Frames Loaded: {len(frames)}")

    # 2. Extract Data for Plotting
    plot_data = []
    found_target = False

    for i, frame in enumerate(frames):
        # Only process the ID we care about for plotting
        if frame.id == TARGET_ID:
            parsed = parse_can_msg(frame)
            
            # Check if parsing was successful and the specific key exists
            if parsed and TARGET_KEY in parsed:
                plot_data.append(parsed[TARGET_KEY])
                found_target = True

    # 3. Plotting
    if not found_target:
        print(f"Warning: No data found for ID 0x{TARGET_ID:X} with key '{TARGET_KEY}'.")
        print("Check your target ID and parser keys.")
    else:
        print(f"Plotting {len(plot_data)} points for {TARGET_KEY}...")
        
        plt.figure(figsize=(10, 6))
        plt.plot(plot_data, label=f"ID 0x{TARGET_ID:X}: {TARGET_KEY}")
        
        plt.title(f"CAN Data Plot: {TARGET_KEY}")
        plt.xlabel("Frame Index")
        plt.ylabel(TARGET_KEY)
        plt.legend()
        plt.grid(True, which='both', linestyle='--', linewidth=0.5)
        
        # Optional: Limit y-axis if data is noisy or has huge spikes
        # plt.ylim(0, 8000) 
        
        #plt.xlim(0,len(plot_data))
        plt.show()

if __name__ == "__main__":
    main()