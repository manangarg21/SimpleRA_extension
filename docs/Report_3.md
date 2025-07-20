# DS25 Project Phase 3 Report Team 9

# Indexing Structure

We chose a **non-clustered sorted secondary index** (an on-disk table sorted by the indexed column) because:

- It supports **efficient range queries** (`<, <=, >, >=`) by binary-searching the index pages and scanning only the required range.
- It requires only **O(1)** extra memory per index page and uses our existing external-merge sort for build.
- It cleanly integrates with our `Table`/`Cursor`/`Page`/`BufferManager` framework, with minimal new code.

- Efficient exact and range lookups: O(log M + K)

- External sort build under memory cap: O(N·log_B N)

- Minimal integration effort with existing buffer/external-sort


Alternative (hashed) indexes excel at equality but cannot accelerate range predicates without scanning all buckets.

# Assumptions

- **Statistics of unique values** in a column are *not* maintained during INSERT/DELETE. If needed, a full scan or separate routine could recompute these.
- **Single-attribute conditions**: All `WHERE` clauses involve exactly one column and one literal.
- **Block size**: We assume the same `BLOCK_SIZE` for data and index pages; pages hold up to `maxRowsPerBlock = (BLOCK_SIZE*1000)/(sizeof(int)*columnCount)` rows.
- **Index dirty bit**: After any data modification (`INSERT`, `DELETE`, `UPDATE` on the indexed column), the index is marked dirty and will be rebuilt on next `SEARCH`.
- **CSV newline**: The source CSV file must end with a newline before blockification, so that `load()` reads the final data line correctly.


# Implementation Details

This report covers the design and implementation of the following commands in our relational algebra engine: **INSERT**, **DELETE**, **UPDATE**, **SEARCH**, and **INDEX**. We describe how each command is executed, how it leverages the buffer manager, and how secondary indexing is integrated.

## INSERT
- **Syntax**: `INSERT INTO table_name (col1 = val1, col2 = val2, ...)`
- **Execution Steps**:
  1. **Parse & Validate**: Extract column–value pairs; validate table and column existence.
  2. **Build New Row**: Initialize a full-length `vector<int>` of zeros, then fill in specified values.
  3. **Append to Data Pages**: Only the *last* page is read and possibly modified:
     - If the last page has free capacity, read it, append the new row, and rewrite that one page.
     - Otherwise allocate a new page file and write the row there.
  4. **Update Table Metadata**: Increment `rowCount`, update `rowsPerBlockCount`, and leave existing pages intact.
  5. **Invalidate Indexes**: Mark all secondary indexes on this table as *dirty* so that next SEARCH will rebuild them.

## DELETE
- **Syntax**: `DELETE FROM table_name WHERE col op value`
- **Execution Steps**:
  1. **Parse & Validate**: Ensure table exists, column exists, and table is indexed on that column.
  2. **Locate via Index**: Binary-search the on-disk index table `idx_<table>_<col>` to find pages covering the predicate.
  3. **Delete Matching Rows**:
     - For each index entry matching the predicate, read the corresponding data page once.
     - Filter out deleted rows into `vector<vector<int>>`, then rewrite that single data page file.
     - Record pages touched to avoid redundant rewrites.
  4. **Invalidate Index**: After deletion, mark the index dirty so SEARCH will rebuild it.
  5. **Update Statistics**: Decrement `rowCount` by number of deleted tuples.

## UPDATE
- **Syntax**: `UPDATE table_name WHERE col1 op val1 SET col2 = val2`
- **Execution Steps**:
  1. **Parse & Validate**: Table and columns must exist; table must be indexed on the WHERE column.
  2. **Locate via Index**: Use the same page-binary-search logic as DELETE/SEARCH.
  3. **Modify Rows In-Place**:
     - For each matching index entry, fetch data page, modify only the targeted column in-memory, rewrite that single page.
  4. **Invalidate Affected Indexes**: If the updated column is indexed, mark that index dirty.

## SEARCH
- **Syntax**: `result <- SEARCH FROM table_name WHERE col op value`
- **Default Behaviour**: If no index or index is dirty, *auto-build* (or rebuild) the on-disk secondary index before probing.
- **Execution Steps**:
  1. **Check/Rebuild Index**: If `hasIndexOn(col)==false || isIndexDirty(col)==true`, call `buildSecondaryIndex(col)`.
  2. **Binary-Search Pages**: Run `findFirstPageGE`/`findLastPageLE` on the index table to narrow to at most O(log N) pages.
  3. **Scan & Filter**: For each entry in those pages, apply the predicate to its `key`; for matches, fetch the corresponding data page and output the row to the result table.
  4. **Materialize Result**: Blockify the result table and register it in the `TableCatalogue`.

## INDEX
- **Syntax**: `INDEX ON col FROM table_name USING {HASH}`
- **Secondary Index Structure**: We use an **on-disk sorted table** of `(key, page, offset)` entries:
  1. Build a temp CSV with header `(col,page,offset)`.
  2. `load()` → blockify into pages under `../data/temp` using the same external split logic as base tables.
  3. `sort()` → external merge sort on the key column with up to `BLOCK_COUNT_P2` buffers.
  4. Register the index by calling `Table::registerIndexOn(col)` and storing its name `idx_<table>_<col>`.


# How Indexing Improves Performance

Without an index, `SEARCH` or `DELETE` on a predicate requires scanning *all* data pages (O(#pages) I/O). With the on-disk sorted index:

1. **Build cost**: One external sort using up to 10 buffers (optimal under the constraints).
2. **Query cost**: O(log N) index page reads + O(K) index entries scanned + O(M) data page reads, where `K` is number of matching keys and `M` the number of distinct pages containing them.

In typical large-table scenarios, `K << totalRows`, so this yields **orders-of-magnitude fewer** I/Os compared to a full table scan.

## Contributions
All members collaborated closely, discussing and exploring various design ideas. While contributions were fluid, the general responsibilities were as follows:
- Irfan Ukani (2023201039): Worked on the update and delete functionalities.
- Manan Garg (2022101109): Worked on setting up the indexing and the search functionality.
- Namrata Baliga (2022101021): Worked on setting up the indexing and insert functionality.

All were involved in the testing phase.
