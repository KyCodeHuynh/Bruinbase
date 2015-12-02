# Bruinbase

## Introduction 

Bruinbase is a lightweight relational database with B+ tree index.


## Optimizations

NOTE: We have a few extra page reads, especially for LOADs, 
due to how we store our BTreeIndex metadata. Specifically, 
page 0 of the B+ tree holds the root PageId, largest key, etc. 
Differences in page read counts from the example output and ours
is due to this, not inefficient indexing. 

## Team

* Crystal Hsieh: crystalhsieh7@gmail.com

* Ky-Cuong Huynh: kycuonghuynh@ucla.edu


## Thanks

* Professor Cho
