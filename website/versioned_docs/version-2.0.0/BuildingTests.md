---
id: version-2.0.0-BuildingTests
title: Building QAppImageUpdate Unit Tests
sidebar_label: Building  Tests
original_id: BuildingTests
---

Running the entire unit tests requires **lot of bandwidth**,**hard drive space** and **cpu time.**
You can however run a quick tests to make sure everything works somewhat okay.

To build and run the tests from CMake, do the following.

```
 $ cd QAppImageUpdate
 $ mkdir build
 $ cd build
 $ cmake -DBUILD_TESTS=ON ..
 $ make -j$(nproc)
 $ ./tests/QAppImageUpdateTests
```


**For a Quick Test**.

```
 $ cmake -DBUILD_TESTS=ON -DQUICK_TEST=ON .. 
```


