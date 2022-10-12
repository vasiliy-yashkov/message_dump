# message_dump
Program for generating property files containing error codes and error messages from firebird 5 for jaybird driver.

# Using

* Set path to Firebird directory in `FB_HOME` environment variable.
* Use cmake to generate the build files.
* Build project.
* Use `./message_dump` to generate `isc_error_msg.properties` and `isc_error_sqlstates.properties`. Launch without parameters will generate files in directory with `message_dump` file. Or use parameters to specify output files: \
`./message_dump /tmp/isc_error_msg.properties /tmp/isc_error_sqlstates.properties`

## Example

export FB_HOME=/opt/firebird \
cmake -S . -B build \
cd build \
make \
./message_dump
