# Summary

Lxfanalert is designed to watch a dodgy thinkpad fan and alert when it isn't operational. May or may not require `options thinkpad_acpi fan_control=1` in modprobe configuration.

It polls `/proc/acpi/ibm/fan` every 15 seconds and detects failure based on the presence of a line like `speed:.*65535`.

# License

Public domain - do whatever you want with the code.
