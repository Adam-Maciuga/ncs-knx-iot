.. _knx_iot_migration:

KNX IoT migration guide
#######################

.. contents::
   :local:
   :depth: 3

This page describes the changes required to migrate your application between KNX IoT add-on versions.

Updating from v0.1
******************

Stack API changes
=================

Applications that call the Point API directly must review the following changes:

* ``oc_core_set_device()`` was replaced by individual identity and version setters.
  Version setters now take a pointer to an ``oc_knx_version_info_t`` structure.
* ``oc_core_set_device_res()`` must be called explicitly after the basic device identity is set and before the remaining device setup.
  It initializes the KNX resources and connectivity.
* ``app_get_precalculated_spake_data()`` is required for the v1.1.0 SPAKE2+ verifier.
  A generated record must match the password, salt, and iteration count used by the application.
* ``oc_device_info_t.serialnumber`` is a ``const char *`` rather than an ``oc_string_t``.
  Remove ``oc_string()`` wrappers around it.
* Include port-layer headers such as ``oc_clock.h``, ``oc_connectivity.h``, ``oc_storage.h``, and ``dns-sd.h`` with the ``port/`` prefix.
* ``knx_publish_service()`` was renamed to ``knx_dns_sd_update_service()``.

Kconfig changes
===============

Remove legacy stack settings that are no longer exposed:

* ``KNXIOT_SPAKE2P_PSA``
* ``KNXIOT_SPAKE2P_MBEDTLS``
* ``KNXIOT_LIB_DEMO_APPS``
* ``LIB_KNX_LOG_LEVEL``
* ``OC_PRINT_ENABLED``
* ``OC_USE_KNX_PORT_LAYER_ENABLED``
* ``OC_REPLAY_PROTECTION_ENABLED``
* ``KNX_LOG_TO_ZEPHYR``

Also remove ``MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS`` from application configuration files.
The SPAKE2+ implementation now handles the required Mbed TLS declarations internally.

For logging, use ``KNXIOT_DEBUG`` to compile the stack's debug code and ``KNXIOT_LOG_LEVEL`` to select the stack log level.
``KNX_THREAD_STACK_SIZE`` remains available and now controls the add-on's stack event-loop thread.

Add-on application changes
****************************

Datapoint method flags
======================

The ``methods`` member of ``knx_datapoint_t`` now uses add-on flags:

.. code-block:: c

   .methods = KNX_DP_GET | KNX_DP_PUT,

Replace old uses of ``OC_GET`` and ``OC_PUT`` in datapoint definitions with ``KNX_DP_GET`` and ``KNX_DP_PUT``.

Programming mode and persisted state
====================================

Programming mode is runtime-only in the v1.1.0 integration and does not survive a restart.
Applications must not rely on the stack restoring it from storage.
