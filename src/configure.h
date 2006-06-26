/* configure.h */



#ifndef CONFIGURE_CONFIGURE_H
# define CONFIGURE_CONFIGURE_H

# include <System.h>


/* types */
typedef enum _HostArch
{
	HA_I386 = 0, HA_I486, HA_I586, HA_I686,
	HA_SPARC, HA_SPARC64,
	HA_SUN4U,
	HA_UNKNOWN
} HostArch;
# define HA_LAST HA_UNKNOWN
extern const String * sHostArch[HA_LAST+1];

typedef enum _HostOS
{
	HO_GNU_LINUX = 0,
	HO_FREEBSD, HO_NETBSD, HO_OPENBSD,
	HO_SUNOS,
	HO_UNKNOWN
} HostOS;
# define HO_LAST HO_UNKNOWN
extern const String * sHostOS[HO_LAST+1];

typedef enum _HostKernel
{
	HK_LINUX20 = 0, HK_LINUX22, HK_LINUX24, HK_LINUX26,
	HK_NETBSD20, HK_NETBSD30,
	HK_SUNOS57, HK_SUNOS58, HK_SUNOS59, HK_SUNOS510,
	HK_UNKNOWN
} HostKernel;
# define HK_LAST HK_UNKNOWN
struct HostKernel
{
	HostOS os;
	const char * version;
};
extern const struct HostKernel sHostKernel[HK_LAST+1];

typedef enum _TargetType
{
	TT_BINARY = 0, TT_LIBRARY, TT_OBJECT, TT_UNKNOWN
} TargetType;
# define TT_LAST TT_UNKNOWN
extern const String * sTargetType[TT_LAST];

typedef enum _ObjectType
{
	OT_C_SOURCE = 0, OT_ASM_SOURCE, OT_UNKNOWN
} ObjectType;
# define OT_LAST OT_UNKNOWN
extern const String * sObjectType[OT_LAST];
String * _source_extension(String * source);


/* constants */
# define PROJECT_CONF "project.conf"
# define MAKEFILE "Makefile"


/* configure */
/* types */
typedef struct _Prefs
{
	int flags;
	char * bindir;
	char * destdir;
	char * includedir;
	char * libdir;
	char * prefix;
} Prefs;
# define PREFS_n	0x1
# define PREFS_v	0x2
typedef struct _Configure
{
	Prefs * prefs;
	HostArch arch;
	HostOS os;
	HostKernel kernel;
} Configure;


/* functions */
int configure_error(char const * message, int ret);

int enum_string(int last, const String * strings[], String * str);
int enum_string_short(int last, const String * strings[], String * str);

#endif /* !CONFIGURE_CONFIGURE_H */