#ifndef MODULE_VEHICLE_MAVLINK_GLOBAL_H
#define MODULE_VEHICLE_MAVLINK_GLOBAL_H


#ifdef _MSC_VER
#  if defined(MODULE_VEHICLE_MAVLINK_LIBRARY)
#    define MODULE_VEHICLE_MAVLINKSHARED_EXPORT  __declspec(dllexport)
#  else
#    define MODULE_VEHICLE_MAVLINKSHARED_EXPORT  __declspec(dllimport)
#  endif
#else
#  define MODULE_VEHICLE_MAVLINKSHARED_EXPORT
#endif

#endif // MODULE_VEHICLE_MAVLINK_GLOBAL_H
