import pandas as pd
import numpy as np

def rotate_clockwise(matrix):
    n = len(matrix)
    rotated_matrix = [[0] * n for _ in range(n)]
    
    for i in range(n):
        for j in range(n):
            rotated_matrix[j][n - i - 1] = matrix[i][j]
    
    return rotated_matrix

n = 51
matrix = np.arange(1, n*n + 1).reshape(n, n).tolist()


rotated_matrix = rotate_clockwise(matrix)

transpose_matrix = np.transpose(matrix).tolist()

transpose_matrix = [[-element for element in row] for row in transpose_matrix]

df_original = pd.DataFrame(matrix)
df_rotated = pd.DataFrame(rotated_matrix)
df_transpose = pd.DataFrame(transpose_matrix)

original_file_path = f'original_{n}x{n}_matrix.csv'
rotated_file_path = f'rotated_{n}x{n}_matrix.csv'
transpose_file_path = f'transpose_{n}x{n}_matrix.csv'

df_original.to_csv(original_file_path, index=False, header=False)
df_rotated.to_csv(rotated_file_path, index=False, header=False)
df_transpose.to_csv(transpose_file_path, index=False, header=False)

print(f"Original matrix saved to {original_file_path}")
print(f"Rotated matrix saved to {rotated_file_path}")
print(f"Transpose matrix saved to {transpose_file_path}")
