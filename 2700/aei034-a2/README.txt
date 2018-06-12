Testfile provided: dbtest.py

In order to test with this file: 
1. make front
2. python dbtest.py
3. ./front_run testdb.dbcmd

In order to test for linear search vs binary search:
1. Locate yourself to approx 1000 lines into schema.c
2. In the function table_search(), at the bottom, the code needed is commented out.
	Uncomment this, and linear search will be tested instead of binary.
