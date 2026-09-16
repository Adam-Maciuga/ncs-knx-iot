.. _knx_iot_ets_commissioning:

Commissioning the samples with ETS
##################################

.. contents::
   :local:
   :depth: 2

The light switch samples use ETS commissioning and the OpenThread Joiner by default.
This page describes how to join the devices to a Thread network and configure their KNX group communication in ETS.
For an overview of the protocol flow, see :ref:`knx_iot_commissioning`.

Requirements
************

You need:

* `ETS tool`_ version 6.4 or later, with compatible product entries for the light switch actuator and sensor.
  Those entries can be found in the Catalog under `KNX Association`:`KNX Virtual`:`IoT Demo Sensor/Actuator`, or downloaded from the `KNX IoT virtual LSxB product entry`_.
* A Thread Border Router that makes the Thread network reachable from the ETS host.
  This guide uses an OpenThread Border Router (OTBR), but another Border Router can be used.
* One development kit for each sample.

The product entry uses manufacturer code ``0x00FA``, which is assigned to the KNX Association.
The samples use this code for demonstration purposes.
The following serial numbers identify the devices:

========  ================
Sample    Serial number
========  ================
Actuator  ``00fa10020900``
Sensor    ``00fa10020700``
========  ================

Build and flash
***************

From the |addon| directory, build and flash the default configuration for each sample:

.. code-block:: console

   west build -b nrf54l15dk/nrf54l15/cpuapp -d build/actuator samples/light_switch_actuator
   west flash -d build/actuator

   west build -b nrf54l15dk/nrf54l15/cpuapp -d build/sensor samples/light_switch_sensor
   west flash -d build/sensor

Join the Thread network
***********************

The samples start the Joiner automatically with PSKd ``N0RD1C``.
The following steps use OTBR and its ``ot-ctl`` interface.
If you use another Border Router, follow its equivalent commissioning procedure and then verify the attachment.

#. Optionally retrieve the device EUI-64 by running the following command in the device shell.
   If the shell is disabled (for example, in the release configuration), skip this step and authorize the Joiner using the wildcard method described later.

   .. code-block:: console

      ot eui64

#. On the OTBR host, start the Commissioner and authorize the Joiner:

   .. code-block:: console

      ot-ctl commissioner start
      ot-ctl commissioner joiner add <EUI64> N0RD1C

   It is also possible to authorize the Joiner without specifying its EUI-64.
   Just make sure only one device is trying to join the network in this scenario.

   .. code-block:: console

      ot-ctl commissioner joiner add * N0RD1C

#. Wait until **LED 0** remains on, indicating that the device has attached to the Thread network.
   If the device does not attach, press **Button 0** to retry the Joiner.

If the Commissioner authorization expires before the device joins, repeat the authorization and retry the Joiner.

.. note::

   ETS requires the samples to be attached to a Thread network that is reachable from the ETS host.
   Thread commissioning is not required if you provision the Thread Operational Dataset manually.
   See the Commissioning section of the appropriate sample documentation for this alternative.

Configure the devices in ETS
****************************

#. Create an ETS project and add an IoT line.
#. Add the actuator and sensor product entries to the line.
#. Read each device's onboarding QR string from its shell:

   .. code-block:: console

      knx_iot qr_code

#. Enter the corresponding QR string in ETS when prompted for the device certificate.
#. Link the switch and status group objects of the sensor and actuator.
#. In the ETS topology view, right-click each device and select :guilabel:`Download` > :guilabel:`Download All`.
#. Press **Button 1** to enable programming mode or select an option to program the device via the serial number.
   **LED 1** blinks while programming mode is active.
   You can also enable programming mode from the shell with ``knx_iot pm 1``.

After commissioning, **LED 1** remains on.
Press **Button 2** or **Button 3** on the sensor and verify that the corresponding LED (**LED 2** or **LED 3**) changes state on the actuator.
