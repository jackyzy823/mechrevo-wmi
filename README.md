* Requirements

	1. kernel-devel (RHEL based) / linux-headers (Debian based)
	2. gcc & make

* Build steps

	1. make
	2. insmod

* Usage

	1. Read: `cat /sys/devices/platform/PNP0C14:00/wmi_bus/wmi_bus-PNP0C14:00/ABBC0F6C-8EA1-11D1-00A0-C90629100000/performance_mode`
	2. Edit: `echo 1 | sudo tee /sys/devices/platform/PNP0C14:00/wmi_bus/wmi_bus-PNP0C14:00/ABBC0F6C-8EA1-11D1-00A0-C90629100000/performance_mode`


* Note

	Only tested with Code 01 V2.0

* TODO

	dkms or akmods

* How

	The seq i guess:


```mermaid
sequenceDiagram
    FnX ->>+ KeyboardController: Press Key
    activate EC
    KeyboardController ->>+ EC: Keyboard Event
    EC->>EC: Update Mode
    deactivate EC
    activate OS
    KeyboardController ->>+ OS: Keyboard Event
    Note over EC,OS: Windows: SELECT from acpitest_qulong <br/> where instance = "acpi\pnp0c14\3"
    Note over EC,OS: Linux: cat sysfs
    OS ->> EC: Query Mode via WMI
    deactivate OS
    OS ->> OS: Display Mode

    Opt 
    Note over EC,OS: Linux: echo sysfs
    Note over EC,OS: Windows: wmic set
    OS ->> EC:  Update Mode via WMI
    end
```
