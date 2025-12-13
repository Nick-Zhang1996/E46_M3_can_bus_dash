import pickle
from collections import Counter
from can_msg_logger import CANFrame

def main():
    filename = "to_publix.pkl"

    # 2. Load the data
    try:
        with open(filename, "rb") as f:
            frames = pickle.load(f)
    except FileNotFoundError:
        print(f"File {filename} not found.")
        return
    except Exception as e:
        print(f"Error loading pickle file: {e}")
        return

    print(f"Total Frames Loaded: {len(frames)}")
    print("-" * 30)

    # 3. Count occurrences of each ID using a Counter
    # We extract just the .id attribute from every frame
    id_counts = Counter(frame.id for frame in frames)

    # 4. Print results sorted by ID
    # sorted() ensures the IDs appear in numerical order (e.g., 0x001 before 0x100)
    for can_id in sorted(id_counts.keys()):
        count = id_counts[can_id]
        print(f"ID: 0x{can_id:03X}, count: {count}")

if __name__ == "__main__":
    main()