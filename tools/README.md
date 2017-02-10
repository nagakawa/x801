## Tools for x801 assets

These are Python scripts that build assets.

### Common File Syntax

`#` starts a comment. It must be on its own line.

A field is an alphanumeric string, followed by a colon and zero or more
space-separated values.

A value is either a numeric literal or a string in quotes. Escaping `\\`,
`\n`, and `\t` are supported. It may also be `to END` where `END` is any
alphanumeric string. In that case, the lines after this one, until `END`
is reached, comprise a string in that place. Having multiple `to END` strings
is supported.

A section declaration is an alphanumeric string prefixed with `@` (this is a
1st-level declaration) or any number *n* of `=` (this is an *n + 1*st-level
declaration). The declaration is followed by an optional comment (no `#`
needed). A section is implicitly ended at the next section declaration of a
level at least as shallow as it.

Any fields before the first section declaration belongs to the global header.
Any fields after that belongs the appropriate section.

#### Using the `fparse` module

The module exports a function `parse`:

  header, sections, dsCount = parse(source)

* `header` is a section containing the global header
* `sections` is a section of all sections
* `dsCount` is number of 1st-level sections

Since a field and a section can have the same name, and there can be multiple
instances of each, the `fparse` module represents a section as a dictionary
from strings (keys) to lists of entries, each of which can be a field value
(which is also a list) or a section.

### compile-map.py

Usage:

    python3 tools/compile-map.py asset-src/maps/map.0.0.msf \
        assets/map/map.0.0.map

Syntax: the Common File Syntax.

    # Header:
    # World ID
    WorldID: 0
    # Area ID
    AreaID: 0
    # Corresponds to the TILE section of the x801 map format
    @TILE
    =Layer There can be multiple layers, of course.
    #      Layers are ordered by when they occur in the source;
    #      e. g. Layer 0 will correspond to the first =Layer
    #      declaration.
    # Dimensions
    Width: 80
    Height: 80
    StartX: -40
    StartY: -40
    ==Tiles
    # Python code to modify the tile array.
    # _ is the array.
    # Builtin functions (except __import__), as well as functions
    # from math and random are provided.
    # In addition, the following functions are provided:
    # get(_, x, y) ­– get the tile at (x, y)
    # put(_, x, y, t) – set the tile at (x, y) to t
    # fill(_, x1, y1, x2, y2, t) – fill the rectangle with a block
    # In this case, we create a blank rectangle with solid blocks
    # along the borders.
    Code: to END
    fill(_, 1, 1, 78, 78, 0)
    fill(_, 0, 0, 0, 79, 0x80000001)
    fill(_, 0, 79, 79, 79, 0x80000001)
    fill(_, 79, 79, 79, 0, 0x80000001)
    fill(_, 79, 0, 0, 0, 0x80000001)
    END