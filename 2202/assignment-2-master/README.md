## _INF-2202 (Fall 2015)_
# Assignment #2: (CONCURRENT) DEDUPLICATION

_Date given: 17.09.2015_

_Date due: 12.10.2015 (EOD)_


## Introduction

In this mandatory assignment you will implement a deduplication sender and receiver using Go.

Deduplication is a global compression technique that is often used by backup systems. It achieves very high compression ratio by identifying redundancy over the entire dataset instead of just a local window. Both sides maintain a big cache of previously sent data, and for redundant data a short fingerprint is sent instead of the data content. Deduplication systems need to support high throughput.

Please read [Assignment \#2.pdf](https://github.com/uit-inf-2202/assignment-2/raw/master/Assignment%20%232.pdf) for the complete assignment instructions.

## Practicalities

The assignment will be done in groups of two. One student will build and evaluate the _sender_ part, and another student will build and evaluate the _receiver_ part.

All students must submit an _individual report_.

Reports and code are handed in using GitHub.
