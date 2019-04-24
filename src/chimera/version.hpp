#ifndef CHIMERA_VERSION_HPP
#define CHIMERA_VERSION_HPP

/** Convert the result of an expression to a string. */
#define STR(s) STR2(s)
#define STR2(s) #s

/** This is the major version of Chimera. */
#define CHIMERA_VERSION_MAJOR 1

/** This is the minor version of Chimera. */
#define CHIMERA_VERSION_MINOR 0

/** This is the build number of Chimera */
#define CHIMERA_VERSION_BUILD 50

/** This is the version the netcode uses. */
#define CHIMERA_NETCODE_VERSION 0

/** This is the version as a string. */
#define CHIMERA_VERSION_STRING STR(CHIMERA_VERSION_MAJOR) "." STR(CHIMERA_VERSION_MINOR) " build " STR(CHIMERA_VERSION_BUILD)

#endif
