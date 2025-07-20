import csv
import random

def get_key_values(num_rows):
    key_values = []
    total_count = 0
    print("Enter value-frequency pairs (value count). The total count should equal", num_rows)
    while total_count < num_rows:
        value, count = map(int, input("Value and count (separated by space): ").split())
        total_count += count
        if total_count > num_rows:
            print("Total count exceeded the number of rows! Please try again.")
            total_count -= count
            continue
        key_values.extend([value] * count)
    key_values.sort()
    return key_values

def generate_column(col_type, num_rows):
    if col_type == "random":
        return [random.randint(0, 100) for _ in range(num_rows)]
    elif col_type == "ascending":
        return list(range(num_rows))
    elif col_type == "descending":
        return list(range(num_rows - 1, -1, -1))

def generate_csv(num_rows, columns):
    data = [[] for _ in range(num_rows)]

    for i, col_name in enumerate(columns):
        col_type = input(f"Enter type for column '{col_name}' (random, ascending, descending, key): ").strip().lower()

        while col_type not in ["random", "ascending", "descending", "key"]:
            print("Invalid type! Please enter one of the following: random, ascending, descending, key")
            col_type = input(f"Enter type for column '{col_name}' (random, ascending, descending, key): ").strip().lower()

        if col_type == "key":
            key_values = get_key_values(num_rows)
            for j in range(num_rows):
                data[j].append(key_values[j])
        else:
            col_data = generate_column(col_type, num_rows)
            for j in range(num_rows):
                data[j].append(col_data[j])

    print("\nGenerated Ordered Data:")
    for row in data:
        print(" ".join(map(str, row)))

    # Save ordered data to CSV
    filename = "ordered_data.csv"
    with open(filename, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(columns)
        writer.writerows(data)
    print(f"\nOrdered data saved as '{filename}'")

    # Option to shuffle data
    shuffle_choice = input("\nDo you want to shuffle the rows? (yes/no): ").strip().lower()
    if shuffle_choice == "yes":
        random.shuffle(data)
        print("\nGenerated Shuffled Data:")
        for row in data:
            print(" ".join(map(str, row)))
        # Save shuffled data to CSV
        filename = "shuffled_data.csv"
        with open(filename, mode="w", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(columns)
            writer.writerows(data)
        print(f"\nShuffled data saved as '{filename}'")

def main():
    num_rows = int(input("Enter the number of rows: "))
    num_cols = int(input("Enter the number of columns: "))
    column_names = input("Enter column names (comma-separated): ").strip()
    columns = [name.strip() for name in column_names.split(",")]

    if len(columns) != num_cols:
        print("Error: Number of column names does not match the number of columns specified!")
        return

    generate_csv(num_rows, columns)

if __name__ == "__main__":
    main()
