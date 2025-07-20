# DS25 Project Phase 2 Report Team 9


### 2.1 SORT: Implementation of External Sorting

### Introduction
In external sorting, We are using the algorithm described in (Chapter 18, Elmasri and Navathe) with the slight modifications. 

Let's assume:
- $b$: Total number of file blocks in the table
- $nB$: Available buffer space (number of buffer blocks)
- $nR$: Number of initial runs where, $nR = \left\lceil\frac{b}{nB}\right\rceil$

The book mentions to create $nR$ runs of $nB$ blocks size each. For sorting $nB$ blocks internally we require same sized buffer which might not be suitable. Hence, we sort each block individually first even though we have $nB$ blocks available. It would only result in one more extra merge pass and we would not require additional buffers to work!

#### Merging Phase:

In Merging phase, we start by loading first $dM$ blocks where $dM$ is degree of merging.

$dM$ = $min(nB - 1, blocksLeftToSort)$

We create a priority queue of cursors of $dM$ size and we define our own comparator to compare the rows of Cursors. We also create a `result` block which will hold the results of the merge. Once the block is full, we write it to the disk. When a specific cursor is empty we remove that from the prioriry queue. 

### Psuedo working (Illustration):

![Diagram 1](https://i.imgur.com/1bDvOav.png)

---

![Diagram 2](https://i.imgur.com/S4Fxmmx.png)

---

![Diagram 3](https://i.imgur.com/A3mjqi6.png)

---

![Diagram 4](https://i.imgur.com/BXNFMYz.png)

### Complexity Analysis:

The algorithm would require the following block accesses:

1. Initial sort phase : $2 * b$
2. Merge Phase : $2 * b * log_{dM}(b)$

Note: Ideally it should be $2 * b * log_{dM}(nR)$ but we modified the initial phase so $nR$ = $b$ in our case.


### 2.2 ORDER BY: Application of External Sorting

### Introduction

The ORDER BY clause is used in SQL to sort the result of a query based on one or more columns in either ascending or descending order. In this implementation, we support sorting based on multiple columns and allow independent sorting strategies (ASC/DESC) for each column. The result is materialized into a new relation.

### Implementation

A new result table is created with the same schema as the source table. All rows from the original table are copied into it. The sort() function is then applied using external sorting with two phases:

- Phase 1: Each disk block is loaded and sorted individually in memory.

- Phase 2: A k-way merge is performed using a priority queue to produce the final sorted output. The sorted result is stored on disk and loaded into the catalogue.

### Complexity Analysis

The overall time complexity is O(N log B), where N is the number of rows and B is the number of blocks, and space complexity is O(M) where M is the number of buffer pages available.


### 2.3 GROUP BY: Application of External Sorting

### Introduction

The GROUP BY clause enables grouping of rows based on a specified column and supports filtering via HAVING using aggregate functions like MIN, MAX, SUM, AVG, and COUNT. The resultant table stores the result of GROUP BY operation. Attribute1, is the column in the table based on which table has to be sorted.
The Aggregate Function1 aggregates over Attribute2, and performs the
comparision with attribute value. Finally, the records that pass the comparision, are taken towards Return part where the value of the Aggerate Function-2 over attribute3 is returned.


### Implementation

In the implementation, the original table is first sorted externally on the grouping column using an in-place multi-way merge sort to ensure rows with the same group key are contiguous. A temporary table is then created by copying the blocks of the sorted table into a new table without modifying the original data. 

During the scan of the sorted temporary table, we use three hash maps: map1 for the HAVING aggregate, map2 for the RETURN aggregate, and map_cnt to track group sizes. Based on the aggregate function type, values are initialized (e.g., COUNT starts at 1, MIN/MAX start with the first value) and updated per group as rows are iterated using a Cursor. For AVG, the sum is stored in the map and later divided by the count after the scan. 

Once all groups are processed, we apply the HAVING condition to filter out groups and write the valid (groupKey, returnAggregateValue) pairs to a CSV file. Finally, the result is loaded as a new table into the catalogue.

### Complexity Analysis

The time complexity is dominated by the sorting step: O(N log N) for sorting N rows, followed by a single scan of the table (O(N)), and a linear pass over the number of groups G (O(G)) to finalize and write results. 

Thus, total complexity is O(N log N + G)

### Assumptions
- There should be no table named with TEMP anywhere. This is a reserved prefix. 

### 2.4 Partition Hash Join: Implementation of Equijoin

### Introduction
For this implementation of EquiJoin, we are creating the algorithm based on the General Case for Partition Join algorithm described in (Chapter 18, Elmasri and Navathe). This involves using the same hash function for rows of different relations; rows for different relations that hash to the same bucket value are compared. If they have the same value, they are joint. 

Given we have 10 blocks available, (BLOCK_COUNT_P2=10), we consider PARTITION_SIZE = 9  - as we always need 1 block for writing. 

The implementation is relatively straightforward. 

The hash function is `ColumnValue mod PARTITION_SIZE` aka `ColumnValue mod 9`. This is a simple hash function and therefore is easy to implement for creating temporary hash buckets which will be disposed later. 

1. We hash the individual files to temporary hash buckets, saved as pages on the disk. 
2. Then we consider each bucket/partition.
3. For one bucket, for Relation R1 we bring upto PARTITION_SIZE blocks worth of data to main memory
4. This itself is stored internally as a Hash Map, to increase efficiency. 
5. Then we probe with all rows from the other Relation R2. 
6. If there is a match, the rows are put in the new JOIN Table
7. We may have to repeat steps 3 to 6 until relation R1 is done. 

#### Assumptions: 

- We are using this Hashing (and its associated functions) exclusively as a means of implementating EquiJoin. This is different from a table being stored using Hashing, which may require more complex and dynamic Hashing.
- The hash file blocks are deleted before the next operation.
- There is infinite disk space to store the temporary hash file blocks.
- The join should be done such that the sum of row size of R1 and R2 doesnt exceed the max no of Integers for one Column. 
- We assume we can follow pattern of writing to a temporary CSV before using blockify. 

#### Complexity Analysis:
It involves the following components.
##### Cost Expression:
Block accesses for the cost of paritioning i.e reading blocks and writing files R1, R2 :  
$$
 2 \times (b_{R1} + b_{R2})
$$

##### Block accesses for probing:
$$
b_{R1} + b_{R2} \times \left(b_{R1} \div Partition\_size\right)
$$

##### Block access for result:
$$
b_{Res}
$$

