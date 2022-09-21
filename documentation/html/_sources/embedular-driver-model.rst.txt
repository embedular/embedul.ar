Device model
============

This section describes the mechanism used through the embedul.ar framework to implement an object-oriented device abstraction layer modeled in "C."

A *device* is a platform-independent base object with public methods and a predefined, private interface that implementations must follow. Therefore, a driver is an implementation of a device's private interface. There are a handful of devices in embedul.ar; each represents a given functionality.

.. note::

   This document will deal with the design and implementation details of a simple driver so you can create your own based on the same premises. For a reference of embedul.ar devices, please look here.

Design principles
-----------------

The design requirements stated below satisfy the premises of an object-oriented interface in "C" running on resource-constrained systems like embedded ones. By design, there is also an effort to be compatible with critical systems standards like MISRA-C and CERT C.

1. No use of dynamic (heap) memory allocation.
2. Platform-independent devices and their public interfaces (abstraction).
3. Devices enforce the implementation of a precise, predefined private interface similar to private pure virtual functions from an abstract base class in C++ (inheritance).
4. Platform-dependent implementations must remain encapsulated within their definitions.
5. An implementation accessed through a cast to its base device pointer must be able to provide the implemented services through the public base device interface (polymorphism).
6. Both the device and implementation must be treated as opaque structures. The user must perform all interactions through the public device interface (encapsulation).
7. The system must allow for multiple instances of the same base device or a unique instance, also called a "singleton."
8. A device may have several implementations to cover different platforms, architectures, and libraries to cover the same functionality on the same platform but by diverse methods.

The following section will dive into how each requirement was met by looking at :c:struct:`RANDOM`'s abstract device definition and one implementation called :c:struct:`RANDOM_SFMT`. The driver uses a specific pseudorandom number generator library (every other device and implementation should follow the same naming convention.)


Implementation details
----------------------

1. No dynamic-memory policy
^^^^^^^^^^^^^^^^^^^^^^^^^^^

embedul.ar follows a strict no dynamic-memory policy.

There is no general-purpose operating system on bare metal programming. On memory-constrained devices like battery-powered embedded systems, perhaps there is an RTOS available at best. Consequently, no operating system dynamic memory management with hardware memory management unit (MMU) exists to safeguard the system integrity from dereferencing dangling pointers, a widespread bug. There is the problem of memory fragmentation arising from memory chunks allocation and deallocation that may lead to unpredictable out-of-memory scenarios. Often, there is the problem of not deallocating a given chunk that creates another widespread bug: memory leaks.

An out-of-memory event may render the device unusable. Yet, correctly dealing with dynamic out-of-memory situations is a matter that programmers often neglect. Handling such an event safely and consistently is complex; that is why there is hardly any dynamic memory usage in critical systems: it is avoided on purpose when reliability is vital. Apart from eliminating all problems listed above, static memory allows for better planning of memory resource partitioning and even planning a static memory map.


2. Platform-independent device definition
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Every device implements a struct that contains, if needed, the required struct members to define what it represents at an abstract level:

.. code-block:: c

   struct RANDOM
   {
       // Some member(s).
   };
   
Device names are written in uppercase. In this particular case, there are no generic attributes required to portray a generic random number generator. Devices have specific methods to get/set attributes or perform actions. In the case of :c:struct:`RANDOM`, these are:

.. code-block:: c

   uint32_t RANDOM_GetUint32 (void);
   uint64_t RANDOM_GetUint64 (void);

Device methods begin with the base definition name, an underscore, and the function name, in camelcase, as shown.


3. Device private interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The private interface enforces a precise internal interface for an implementation to follow. The :c:struct:`RANDOM_IFACE` defined by the device materializes that requirement:

.. code-block:: c

   typedef uint32_t (* RANDOM_GetUint32Func)(struct RANDOM *const R);
   typedef uint64_t (* RANDOM_GetUint64Func)(struct RANDOM *const R);

   struct RANDOM_IFACE
   {
       const char                   * const Description;
       const RANDOM_GetUint32Func   GetUint32;
       const RANDOM_GetUint64Func   GetUint64;
   };
   
Now that :c:struct:`RANDOM_IFACE` made its acquaintance, below is the complete :c:struct:`RANDOM` base definition:

.. code-block:: c

   struct RANDOM
   {
       const struct RANDOM_IFACE * iface;
   };
   
The private interface, always called *{DEVICE}*\_IFACE, is the first member of any device structure. The implementation is in charge of setting the required IFACE members:

- A statically allocated string with the implementation description; which platform, device or algorithm it provides. This member is mandatory on all IFACE implementations.
- Function definitions that follow the purpose, parameters and return values of the corresponding IFACE function pointers.

:c:struct:`RANDOM_IFACE` allows to introduce an essential base driver public interface function:

.. code-block:: c

   void RANDOM_Init (struct RANDOM *const R, const struct RANDOM_IFACE *const Iface);

Along with the required definition of their IFACE, all devices implement a specialized driver function that adheres to the following naming convention:

*{DEVICE}*\_Init (struct *{DEVICE}* \*const X, const struct *{DEVICE}*\_IFACE \*const Iface).

There are two aspects of the driver model that will be clarified by looking at the :c:func:`RANDOM_Init()` source code:

1. What a *{DEVICE}*\_Init() function do.
2. How to check that the implementation provided necessary parts of the *{DEVICE}*\_IFACE.

.. code-block:: c

   // The only RANDOM instance allowed to exist.
   static struct RANDOM *s_r = NULL;

   void RANDOM_Init (struct RANDOM *const R, const struct RANDOM_IFACE *const Iface)
   {
       BOARD_AssertState  (!s_r);  // 1.
       BOARD_AssertParams (R && Iface);  // 2.

       // Required interface elements
       BOARD_AssertInterface (Iface->Description  // 3.
                               && Iface->GetUint32
                               && Iface->GetUint64);
       ZERO_MEMORY (R);  // 4.

       R->iface = Iface;  // 5.

       s_r = R;  // 6.
   }

As seen in the above listing, a *{DEVICE}*\_Init() function is in charge of:

1. Checking for valid system status. In the case of a singleton, the Init() function must be called only once per base driver to assure the existence of a single initialized instance. For example, :c:struct:`RANDOM`, :c:struct:`BOARD`, :c:struct:`VIDEO`, and :c:struct:`SOUND` are singleton devices by design.
2. Checking for valid parameters (``R`` and ``Iface`` are not NULL pointers).
3. Checking that the implementation filled required interface members, depending on the base driver implementation needs.
4. Clearing base driver instance memory.
5. Assigning the implementation-supplied IFACE struct to the device instance.
6. In the case of a singleton, assigning the initialized instance as the only one in existence.

For the public device interface, the following listing clarifies the relationship between that and the implementation:

.. code-block:: c

   uint32_t RANDOM_GetUint32 (void)
   {
       BOARD_AssertState (s_r);
       return s_r->iface->GetUint32(s_r);
   }


   uint64_t RANDOM_GetUint64 (void)
   {
       BOARD_AssertState (s_r);
       return s_r->iface->GetUint64(s_r);
   }

Both public device methods perform an internal call to the implementation through the IFACE function pointers. Note how both checks for valid system status (initialized singleton) and then call the implementation functions passing the device instance as the first parameter, and returning the results to the caller. There are no assertions on iface or its function pointers; since s_r is private, it's okay to assume that the initialization function already checked it.

It is common to store a device using its lowercase name (por example, random.c/.h for the :c:struct:`RANDOM` device). Devices are stored in the "embedul.ar/source/core/device" directory.


4. Encapsulated platform-dependant implementation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The private device interface is an abstract implementation that does not work on its own. It is necessary to implement functionality by giving support to a concrete random number generator. There are hardware devices and software pseudorandom number generators. The implementation presented below, :c:struct:`RANDOM_SFMT`, uses the Fast Mersenne Twister (SFMT) generator by Mutsuo Saito and Makoto Matsumoto.

.. code-block:: c

   #include "embedul.ar/source/core/device/random.h"
   #include "SFMT.h"

   struct RANDOM_SFMT
   {
       struct RANDOM    device;
       uint64_t         seed;
       sfmt_t           sfmt;
   };

A device (struct RANDOM) named "device" must be placed as the first implementation member, followed by other implementation-defined members (in that case, sfmt_t).
   
Implementation names comply with the following convention: *{DEVICE}*\_*{IMPLEMENTATION}*, all in uppercase. In the case of a hardware device, it is common practice to include the integrated circuit, for example, IO_LP5036. Since it is not a platform-dependent driver but a specific implementation of a random number generator, c:struct:`RANDOM_SFMT` driver implementation resides on embedul.ar/source/drivers. Every other driver follows a similar naming convention as discussed.

In the listing below, it may be clear how the implementation defines and fills the corresponding base driver *{DEVICE}*\_IFACE:

.. code-block:: c

   static void         hardwareInit    (struct RANDOM *const R);
   static uint32_t     getUint32       (struct RANDOM *const R);
   static uint64_t     getUint64       (struct RANDOM *const R);


   static const struct RANDOM_IFACE RANDOM_SFMT_IFACE =
   {
       .Description    = "Fast Mersenne Twister",
       .HardwareInit   = hardwareInit,
       .GetUint32      = getUint32,
       .GetUint64      = getUint64
   };
   
Both the IFACE and functions declaration are static; the compiler will not export those symbols outside of the source they are defined. Also, the IFACE struct is const, which states the read-only intend.

The implementation's Init function, :c:func:`RANDOM_SFMT_Init`, also follows another device model pattern:

.. code-block:: c

   void RANDOM_SFMT_Init (struct RANDOM_SFMT *const S, const uint64_t Seed)
   {
       BOARD_AssertParams (S);  // 1.

       DEVICE_IMPLEMENTATION_Clear (S);  // 2.
       
       S->seed = Seed; // 3.

       RANDOM_Init ((struct RANDOM *)s, &RANDOM_SFMT_IFACE);  // 4.
   }
   
Any implementation Init() function must receive its self instance and, as required, any other suitable parameter needed to initialize the implementation. In this case, it requires a 64-bit seed. Once inside the function, it must perform the following steps in order:

1. Checking for valid parameters (``S`` not NULL).
2. Clearing implementation instance memory.
3. Initialize implementation data members.
4. Calling the device Init() function passing an instance cast from :c:struct:`RANDOM_SFMT` to :c:struct:`RANDOM` and a pointer to the IFACE definition.

In turn, :c:func:`RANDOM_Init` will perform generic device initialization and it will call the implementation HardwareInit(), if available. That function will perform the actual random number generator initialization, as shown below in the IFACE implementation functions listing:

.. code-block:: c

   static void hardwareInit (struct RANDOM *const R)
   {
       struct RANDOM_SFMT *const S = (struct RANDOM_SFMT *) R;

       sfmt_init_by_array (&S->sfmt, (uint32_t *)&S->seed, 2);

       LOG_Items (1, LANG_PERIOD, (uint32_t)SFMT_MEXP);
   }

   static uint32_t getUint32 (struct RANDOM *const R)
   {
       struct RANDOM_SFMT *const S = (struct RANDOM_SFMT *) R;

       return sfmt_genrand_uint32 (&S->sfmt);
   }

   static uint64_t getUint64 (struct RANDOM *const R)
   {
       struct RANDOM_SFMT *const S = (struct RANDOM_SFMT *) R;

       return sfmt_genrand_uint64 (&S->sfmt);
   }

As a general rule, all private interface implementation members take a generic device instance pointer. Inside those members, it is okay to cast from a generic device instance to an implementation instance and back. The mechanism allows using an abstract, platform and device-independent interface to pass a generic instance from a specific implementation. It is possible to use any driver implementation through a cast to the corresponding device instance, completely encapsulating the implementation.


5. Implementation access through its base definition pointer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As discussed, a device definition is abstract and does not work on its own. The instance that gets allocated is the one from the implementation, as in the following example:

.. code-block:: c

   #include "embedul.ar/source/core/drivers/random_sfmt.h"
   
   struct RANDOM_SFMT r_sfmt;
   
   RANDOM_SFMT_Init (&r_sfmt, 1234);

Since :c:struct:`RANDOM` is a singleton, the base drive interface accesses the instance that was already set in RANDOM_Init() as shown earlier, so there is no need to pass any instance to the interface:

.. code-block:: c

   uint32_t number = RANDOM_GetUint32 ();

Suppose that :c:struct:`RANDOM` is not a singleton but a multi-instance driver. Then, the interface might take a :c:struct:`RANDOM` instance as the first argument, as already discussed:

.. code-block:: c

   struct RANDOM * r = (struct RANDOM *) r_sfmt;

   uint32_t number = RANDOM_GetUint32 (r);
   
In any case, the user only manipulates generic device instances and calls to public device members. This abstraction layer allows the embedul.ar framework and its applications to remain detached entirely from platform and driver implementations while still using a strict interface to define and access those implementations.


6. Instances treated as opaque structures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Public device interface functions should cover all meaningful instance accesses. A good interface disallows forbidden instance member accesses.

There are ways to enforce a real opaque-ness on a "C" structure. But, as a design decision, keeping code simple is more important than hiding and obfuscating.


7. Multiple instances and singletons
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:c:struct:`RANDOM` is a singleton, but simple changes allow to support unlimited instances of it. Start by removing the singleton definition, state check and assignment at initialization:

.. code-block:: c

   void RANDOM_Init (struct RANDOM *const R, const struct RANDOM_IFACE *const Iface)
   {
       BOARD_AssertParams (R && Iface);

       // Required interface elements
       BOARD_AssertInterface (Iface->Description
                               && Iface->GetUint32
                               && Iface->GetUint64);
       OBJECT_Clear (R);

       R->iface = Iface;
   }

At the base driver interface, use the instance passed as the first parameter.

.. code-block:: c

   uint32_t RANDOM_GetUint32 (struct RANDOM *const R)
   {
       BOARD_AssertParams (R);
       return R->iface->GetUint32(R);
   }


   uint64_t RANDOM_GetUint64 (struct RANDOM *const R)
   {
       BOARD_AssertParams (R);
       return R->iface->GetUint64(R);
   }
   
   
As seen, it is more involved to implement singletons than multiple instances.


8. One generic device, several implementations 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Each generic device may have several underlying implementations. :c:struct:`RANDOM` may have any number of software algorithms and hardware random number generators implemented. However, each implementation exposes the same :c:struct:`RANDOM` instance type and public interface to the user.


The following diagram summarizes the generic device and driver implementation.

.. image:: images/driver_model.drawio.svg
