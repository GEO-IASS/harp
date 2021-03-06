harpmerge
==========

Combine multiple products from files or directories by appending them accross
the time dimension and storing the result into a single output file.

::

  Usage:
      harpmerge [options] <file|dir> [<file|dir> ...] <output product file>
          Concatenate all products as specified by the file and directory paths
          into a single product.

          Options:
              -a, --operations <operation list>
                  List of operations to apply to each product.
                  An operation list needs to be provided as a single expression.
                  See the 'operations' section of the HARP documentation for
                  more details.
                  Operations will be performed before a product is appended.

              -o, --options <option list>
                  List of options to pass to the ingestion module.
                  Only applicable of an input product is not in HARP format.
                  Options are separated by semi-colons. Each option consists
                  of an <option name>=<value> pair. An option list needs to be
                  provided as a single expression.

              -l, --list
                  Print to stdout each filename that is currently being merged.

              -f, --format <format>
                  Output format:
                      netcdf (default)
                      hdf4
                      hdf5

          If the merged product is empty, a warning will be printed and the
          tool will return with exit code 2 (without writing a file).

      harpmerge -h, --help
          Show help (this text).

      harpmerge -v, --version
          Print the version number of HARP and exit.
