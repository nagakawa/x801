## A Tour of the Experiment801 Codebase

Part 2: Version Classes

This class stores a version number. It covers a subset of Semantic Versioning;
major, minor, and patch versions are limited to 65535, prerelease numbers
(called *prenums* hereafter) are limited to 16383, and prerelease names
(hereafter *pretypes*) are limited to the four most popular varietites.
Build metadata is not supported.

Aside from the obvious `==` and `<` operators, the `Version` class also has
another comparison: `canSucceed` indicates whether one version is backward-
compatible with another. The rules for these are:

* A version is compatible with itself
* A version is incompatible with any later version
* 0.x versions are incompatible with any other version
* Non-release versions are incompatible with any other version
* A version is incompatible with another version with a different major
  release

Finally, there is a constant called `engineVersion` that stores the current
version number of the engine.
