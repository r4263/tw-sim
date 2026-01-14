# TW-SIM 
## Trigger wheel simulator for ESP32

Developed with **ESP32-S3 N16R8 dev module**

Just upload the code and run. Type commands within the serial console.

The signal output pin number is **42** and can be configured in the code by the define **OUTPUT_PIN**.

### Available commands:
#### **set rpm *\<value*\>** - Set the desired RPM of the simulated engine.
#### **set wheel *\<index*\>** - Set the trigger wheel index(available indexes can be seen and modified in the code by the constant *ToothWheelConfig*).
#### **enable** - Enable the output signal.
#### **disable** - Disable the output signal.